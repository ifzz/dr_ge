// Public domain. See "unlicense" statement at the end of dr_ge.h.

// Creates a text editor tool.
drge_subeditor* drge_editor_create_text_editor(drge_editor* pEditor, const char* fileAbsolutePath);

// Deletes the given text editor tool.
void drge_editor_delete_text_editor(drge_subeditor* pTextEditor);