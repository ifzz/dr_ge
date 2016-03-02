// Public domain. See "unlicense" statement at the end of dr_ge.h.

// Creates a text editor tool.
drge_subeditor* drge_editor_create_text_editor(drge_editor* pEditor, const char* fileAbsolutePath);

// Deletes the given text editor tool.
void drge_editor_delete_text_editor(drge_subeditor* pTextEditor);


// Gives the given text editor the keyboard focus.
void drge_editor_text_subeditor__capture_keyboard(drge_subeditor* pTextEditor);

// Release the keyboard focus from the given text editor.
void drge_editor_text_subeditor__release_keyboard(drge_subeditor* pTextEditor);


// Shows the line numbers.
void drge_editor_text_subeditor__show_line_numbers(drge_subeditor* pTextEditor);

// Hides the line numbers.
void drge_editor_text_subeditor__hide_line_numbers(drge_subeditor* pTextEditor);