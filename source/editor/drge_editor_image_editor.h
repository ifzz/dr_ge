// Public domain. See "unlicense" statement at the end of dr_ge.h.

// Creates an image editor tool.
drge_subeditor* drge_editor_create_image_editor(drge_editor* pEditor, const char* fileAbsolutePath);

// Deletes the given image editor tool.
void drge_editor_delete_image_editor(drge_subeditor* pTextEditor);