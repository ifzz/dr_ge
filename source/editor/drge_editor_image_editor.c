// Public domain. See "unlicense" statement at the end of dr_ge.h.

typedef struct
{
    int unused;
} drge_image_subeditor_data;

drge_subeditor* drge_editor_create_image_editor(drge_editor* pEditor, const char* fileAbsolutePath)
{
    // An image editor is just a sub-editor.
    drge_subeditor* pTextEditor = drge_editor_create_sub_editor(pEditor, DRGE_EDITOR_TOOL_TYPE_IMAGE_EDITOR, fileAbsolutePath);
    if (pTextEditor == NULL) {
        return NULL;
    }

    return pTextEditor;
}

void drge_editor_delete_image_editor(drge_subeditor* pTextEditor)
{
    drge_editor_delete_sub_editor(pTextEditor);
}