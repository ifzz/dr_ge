// Public domain. See "unlicense" statement at the end of dr_ge.h.

typedef struct
{
    // A pointer to the editor that owns the sub-editor.
    drge_editor* pEditor;

    // The path of the file the sub-editor is linked to. Can be an empty string in which case it is assumed to be free-floating.
    char fileAbsolutePath[DRFS_MAX_PATH];


    // The function to call when the sub-editor needs to be saved.
    drge_subeditor_save_proc saveProc;


    // The size of the extra data.
    size_t extraDataSize;

    // A pointer to the extra data.
    char pExtraData[1];

} drge_subeditor_data;

drge_subeditor* drge_editor_create_sub_editor(drge_editor* pEditor, const char* type, const char* fileAbsolutePath, size_t extraDataSize)
{
    if (pEditor == NULL || type == NULL) {
        return NULL;
    }

    // Validate the type.
    if (!ak_is_of_tool_type(type, DRGE_EDITOR_TOOL_TYPE_SUB_EDITOR)) {
        drge_errorf(pEditor->pContext, "Attempting to create a sub-editor of an incompatible type (\"%s\")", type);
        return NULL;
    }

    // Validate the path. Must be absolute.
    assert(drpath_is_absolute(fileAbsolutePath));


    // A sub-editor is just a dr_appkit tool.
    drgui_element* pAKTool = ak_create_tool(pEditor->pAKApp, NULL, type, sizeof(drge_subeditor_data) - sizeof(char) + extraDataSize, NULL);
    if (pAKTool == NULL) {
        return NULL;
    }

    drge_subeditor_data* pSEData = ak_get_tool_extra_data(pAKTool);
    if (strcpy_s(pSEData->fileAbsolutePath, sizeof(pSEData->fileAbsolutePath), (fileAbsolutePath != NULL) ? fileAbsolutePath : "") != 0) {
        drge_errorf(pEditor->pContext, "Attempting to create a sub-editor with a file path that exceeds the maximum length (\"%s\")", fileAbsolutePath);
        ak_delete_tool(pAKTool);
        return NULL;
    }

    pSEData->pEditor  = pEditor;
    pSEData->saveProc = NULL;

    // The text of the sub-editor's tab is based on the title of the tool so we'll need to update that. The last argument is
    // whether or not to mark the title as modified by placing an asterix at the end.
    drge_subeditor_update_title((drge_subeditor*)pAKTool, false);

    return (drge_subeditor*)pAKTool;
}

void drge_editor_delete_sub_editor(drge_subeditor* pSubEditor)
{
    if (pSubEditor == NULL) {
        return;
    }

    ak_delete_tool((drgui_element*)pSubEditor);
}


drge_editor* drge_subeditor_get_editor(drge_subeditor* pSubEditor)
{
    drge_subeditor_data* pSEData = ak_get_tool_extra_data((drgui_element*)pSubEditor);
    if (pSEData == NULL) {
        return NULL;
    }

    return pSEData->pEditor;
}

const char* drge_subeditor_get_absolute_path(drge_subeditor* pSubEditor)
{
    assert(ak_is_tool_of_type((drgui_element*)pSubEditor, DRGE_EDITOR_TOOL_TYPE_SUB_EDITOR));

    drge_subeditor_data* pSEData = ak_get_tool_extra_data((drgui_element*)pSubEditor);
    if (pSEData == NULL) {
        return NULL;
    }

    return pSEData->fileAbsolutePath;
}


size_t drge_subeditor_get_extra_data_size(drge_subeditor* pSubEditor)
{
    assert(ak_is_tool_of_type((drgui_element*)pSubEditor, DRGE_EDITOR_TOOL_TYPE_SUB_EDITOR));

    drge_subeditor_data* pSEData = ak_get_tool_extra_data((drgui_element*)pSubEditor);
    if (pSEData == NULL) {
        return 0;
    }

    return pSEData->extraDataSize;
}

void* drge_subeditor_get_extra_data(drge_subeditor* pSubEditor)
{
    assert(ak_is_tool_of_type((drgui_element*)pSubEditor, DRGE_EDITOR_TOOL_TYPE_SUB_EDITOR));

    drge_subeditor_data* pSEData = ak_get_tool_extra_data((drgui_element*)pSubEditor);
    if (pSEData == NULL) {
        return NULL;
    }

    return pSEData->pExtraData;
}


bool drge_subeditor_save_to_file(drge_subeditor* pSubEditor, const char* absolutePathToSaveAs)
{
    assert(ak_is_tool_of_type((drgui_element*)pSubEditor, DRGE_EDITOR_TOOL_TYPE_SUB_EDITOR));
    assert(absolutePathToSaveAs != NULL);

    drge_subeditor_data* pSEData = ak_get_tool_extra_data((drgui_element*)pSubEditor);
    if (pSEData == NULL) {
        return false;
    }

    if (pSEData->saveProc) {
        if (!pSEData->saveProc(pSubEditor, absolutePathToSaveAs)) {
            return false;
        }

        strcpy_s(pSEData->fileAbsolutePath, sizeof(pSEData->fileAbsolutePath), absolutePathToSaveAs);
        drge_subeditor_update_title(pSubEditor, false);

        return true;
    }

    return false;
}


void drge_subeditor_set_save_proc(drge_subeditor* pSubEditor, drge_subeditor_save_proc proc)
{
    assert(ak_is_tool_of_type((drgui_element*)pSubEditor, DRGE_EDITOR_TOOL_TYPE_SUB_EDITOR));

    drge_subeditor_data* pSEData = ak_get_tool_extra_data((drgui_element*)pSubEditor);
    if (pSEData == NULL) {
        return;
    }

    pSEData->saveProc = proc;
}


void drge_subeditor_update_title(drge_subeditor* pSubEditor, bool isModified)
{
    if (pSubEditor == NULL) {
        return;
    }

    char title[64];
    strncpy_s(title, sizeof(title), drpath_file_name(drge_subeditor_get_absolute_path(pSubEditor)), _TRUNCATE);
    if (title[0] == '\0') {
        strcpy_s(title, sizeof(title) - 1, "[New File]");   // -1 to ensure we leave room for the asterix in case isModified is true.
    }

    if (isModified) {
        strncat_s(title, sizeof(title), "*", _TRUNCATE);
    }

    ak_set_tool_title((drgui_element*)pSubEditor, title);
}


bool drge_subeditor_is_read_only(drge_subeditor* pSubEditor)
{
    drge_subeditor_data* pSEData = ak_get_tool_extra_data((drgui_element*)pSubEditor);
    if (pSEData == NULL) {
        return false;
    }

    const char* absolutePath = drge_subeditor_get_absolute_path(pSubEditor);
    if (absolutePath == NULL || absolutePath[0] == '\0') {
        return false;
    }

    drfs_file_info fi;
    if (drfs_get_file_info(drge_subeditor_get_editor(pSubEditor)->pContext->pVFS, absolutePath, &fi) == drfs_success) {
        return (fi.attributes & DRFS_FILE_ATTRIBUTE_READONLY) != 0;
    }

    return false;
}
