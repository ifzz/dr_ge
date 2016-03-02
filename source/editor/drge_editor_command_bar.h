

// Flags returned on ak_exec().
#define DRGE_CMDBAR_RELEASE_KEYBOARD     1
#define DRGE_CMDBAR_NO_CLEAR             2


// Creates a command bar tool.
drgui_element* drge_editor_create_command_bar_tool(drge_editor* pEditor, drgui_element* pParent);

// Deletes the command bar tool.
void drge_editor_delete_command_bar_tool(drgui_element* pCmdBarTool);


// Changes the command bar based on the given tool type.
void drge_editor_command_bar_set_context_by_tool_type(drgui_element* pCmdBarTool, const char* pToolType);


// Sets the font to use in the command bar.
void drge_editor_command_bar_set_font(drgui_element* pCmdBarTool, drgui_font* pFont);

// Sets the color of the text to show in the given command bar.
void drge_editor_command_bar_set_text_color(drgui_element* pCmdBarTool, drgui_color color);

// Sets the background color of the given command bar.
void drge_editor_command_bar_set_background_color(drgui_element* pCmdBarTool, drgui_color color);


// Captures the keyboard on the command bar.
void drge_editor_command_bar_capture_keyboard(drgui_element* pCmdBarTool);

// Releases the keybaord from the command bar.
void drge_editor_command_bar_release_keyboard(drgui_element* pCmdBarTool);


// Sets the text of the command bar.
void drge_editor_command_bar_set_command_text(drgui_element* pCmdBarTool, const char* text);



//// Text Editor Context ////

// Sets the line number for text editor info.
void drge_editor_command_bar_set_text_editor_line_number(drgui_element* pCmdBarTool, unsigned int lineNumber);

// Sets the text editor column number.
void drge_editor_command_bar_set_text_editor_column_number(drgui_element* pCmdBarTool, unsigned int columnNumber);

// Sets the text editor character number.
void drge_editor_command_bar_set_text_editor_character_number(drgui_element* pCmdBarTool, unsigned int characterNumber);
