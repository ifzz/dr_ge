// Public domain. See "unlicense" statement at the end of dr_ge.h.

typedef struct
{
    // The path of the file the sub-editor is linked to. Can be an empty string in which case it is assumed to be free-floating.
    char fileAbsolutePath[DRVFS_MAX_PATH];

} drge_subeditor_data;

drge_subeditor* drge_editor_create_sub_editor(drge_editor* pEditor, const char* type, const char* fileAbsolutePath)
{
    if (pEditor == NULL || type == NULL) {
        return NULL;
    }

    // Validate the type.
    if (!ak_is_of_tool_type(type, DRGE_EDITOR_TOOL_TYPE_SUB_EDITOR)) {
        drge_errorf(pEditor->pContext, "Attempting to create a sub-editor of an incompatible type (\"%s\")", type);
        return NULL;
    }


    // A sub-editor is just a dr_appkit tool.
    drgui_element* pAKTool = ak_create_tool(pEditor->pAKApp, NULL, type, sizeof(drge_subeditor_data), NULL);
    if (pAKTool == NULL) {
        return NULL;
    }

    drge_subeditor_data* pSEData = ak_get_tool_extra_data(pAKTool);
    if (strcpy_s(pSEData->fileAbsolutePath, sizeof(pSEData->fileAbsolutePath), (fileAbsolutePath != NULL) ? fileAbsolutePath : "") != 0) {
        drge_errorf(pEditor->pContext, "Attempting to create a sub-editor with a file path that exceeds the maximum length (\"%s\")", fileAbsolutePath);
        ak_delete_tool(pAKTool);
        return NULL;
    }

    return (drge_subeditor*)pAKTool;
}

void drge_editor_delete_sub_editor(drge_subeditor* pSubEditor)
{
    if (pSubEditor == NULL) {
        return;
    }

    ak_delete_tool((drgui_element*)pSubEditor);
}