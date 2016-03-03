// Public domain. See "unlicense" statement at the end of dr_ge.h.


// Creates a main menu tool.
drgui_element* drge_editor_create_main_menu_tool(drge_editor* pEditor, drgui_element* pParent, ak_window* pWindow);

// Deletes the given main menu tool.
void drge_editor_delete_main_menu_tool(drgui_element* pMMTool);


// Changes the main menu based on the given tool type.
void drge_editor_change_main_menu_by_tool_type(drgui_element* pMMTool, const char* type, bool readOnly);


// Disables the menu items for when there are no editors open.
void drge_editor_disable_text_editor_menu_items(drgui_element* pMMTool);


// Enables the menu items specific to sub-editors.
void drge_editor_enable_sub_editor_menu_items(drgui_element* pMMTool, bool readOnly);

// Enables the menu items specific to text editors.
void drge_editor_enable_text_editor_menu_items(drgui_element* pMMTool, bool readOnly);

