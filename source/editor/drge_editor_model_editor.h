// Public domain. See "unlicense" statement at the end of dr_ge.h.

// Creates an model editor tool.
drge_subeditor* drge_editor_create_model_editor(drge_editor* pEditor, const char* fileAbsolutePath);

// Deletes the given model editor tool.
void drge_editor_delete_model_editor(drge_subeditor* pTextEditor);