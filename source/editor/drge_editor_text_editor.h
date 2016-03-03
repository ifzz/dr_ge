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


// Retrieves the index of the line the caret is currently sitting on.
size_t drge_text_editor__get_cursor_line(drge_subeditor* pTextEditor);

// Retrieves the index of the column the caret is currently sitting on.
size_t drge_text_editor__get_cursor_column(drge_subeditor* pTextEditor);


// Moves the cursor to the beginning of the given line.
void drge_text_editor__goto_line(drgui_element* pTextEditor, size_t lineNumber);

// Moves the cursor to the beginning of the line sitting at the position based on a ratio.
//
// The given ratio should be between 0 and 100.
void drge_text_editor__goto_ratio(drgui_element* pTextEditor, size_t ratio);

/// Finds and selects the next occurance of the given string, starting from the cursor and looping back to the start.
bool drge_text_editor__find_and_select_next(drgui_element* pTextEditor, const char* text);

/// Finds the next occurance of the given string and replaces it with another.
bool drge_text_editor__find_and_replace_next(drgui_element* pTextEditor, const char* text, const char* replacement);

/// Finds every occurance of the given string and replaces it with another.
bool drge_text_editor__find_and_replace_all(drgui_element* pTextEditor, const char* text, const char* replacement);