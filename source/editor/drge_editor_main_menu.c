// Public domain. See "unlicense" statement at the end of dr_ge.h.

typedef struct
{
    // A pointer to the editor that owns this control.
    drge_editor* pEditor;


    /// A pointer to the menu bar control.
    drgui_element* pMBElement;


    /// The file menu.
    ak_window* pFileMenu;

    /// File -> New File
    ak_menu_item* pFile_NewFile;

    /// File -> Open File
    ak_menu_item* pFile_OpenFile;

    /// File -> Save
    ak_menu_item* pFile_Save;

    /// File -> Save As
    ak_menu_item* pFile_SaveAs;

    /// File -> Save As
    ak_menu_item* pFile_SaveAll;

    /// File -> Close
    ak_menu_item* pFile_Close;

    /// File -> Close All
    ak_menu_item* pFile_CloseAll;

    /// File -> Exit
    ak_menu_item* pFile_Exit;


    /// The edit menu.
    ak_window* pEditMenu;

    /// Edit -> Undo
    ak_menu_item* pEdit_Undo;

    /// Edit -> Redo
    ak_menu_item* pEdit_Redo;

    /// Edit -> Cut
    ak_menu_item* pEdit_Cut;

    /// Edit -> Copy
    ak_menu_item* pEdit_Copy;

    /// Edit -> Paste
    ak_menu_item* pEdit_Paste;

    /// Edit -> Delete
    ak_menu_item* pEdit_Delete;

    /// Edit -> Select All
    ak_menu_item* pEdit_SelectAll;


    /// The view menu.
    ak_window* pViewMenu;

    /// View -> Status Bar
    ak_menu_item* pView_StatusBar;

    /// View -> Line Numbers
    ak_menu_item* pView_LineNumbers;


    /// The find menu.
    ak_window* pFindMenu;

    /// Find -> Find
    ak_menu_item* pFind_Find;

    /// Find -> Find and Replace
    ak_menu_item* pFind_Replace;

    /// Find -> Go To
    ak_menu_item* pFind_GoTo;


    /// The Settings menu.
    ak_window* pSettingsMenu;

    /// Settings -> Settings
    ak_menu_item* pSettings_AllSettings;


    /// The help menu.
    ak_window* pHelpMenu;

    /// Help -> View Help
    ak_menu_item* pHelp_ViewHelp;

    /// Help -> About
    ak_menu_item* pHelp_About;


    /// A pointer to the file menu bar item.
    ak_menu_bar_item* pFile;

    /// A pointer to the edit menu bar item.
    ak_menu_bar_item* pEdit;

    /// A pointer to the view menu bar item.
    ak_menu_bar_item* pView;

    /// A pointer to the find menu bar item.
    ak_menu_bar_item* pFind;

    /// A pointer to the settings menu bar item.
    ak_menu_bar_item* pSettings;

    /// A pointer to the help menu bar item.
    ak_menu_bar_item* pHelp;


    /// The tool type of the current state of the main menu. This is set in drge_editor_change_main_menu_by_tool_type().
    char currentToolType[AK_MAX_TOOL_TYPE_LENGTH];

    /// Whether or not the menu is set based on a read-only tool.
    bool currentToolReadOnly;

} drge_editor_main_menu_tool;

typedef struct
{
    /// The name of the action to fire when the item is picked.
    const char* pActionName;

    /// Whether or not the item should be hidden upon being clicked.
    bool hideOnPicked;

} ide_main_menu_item;


// Creates a main menu bar item.
static ak_menu_bar_item* drge_editor_create_main_menu_bar_item(drgui_element* pMMTool, const char* text, ak_window* pMenu);

// Helper for creating a new menu item.
static ak_menu_item* drge_editor_create_main_menu_item(ak_window* pMenuWindow, const char* text, const char* shortcutStr, drgui_image* pIcon, const char* pActionName);

// Helper for retrieving the application from the given menu item.
static ak_application* drge_editor_get_application_from_mmi(ak_menu_item* pMI);

// Callback for main menu items.
static void drge_editor_on_main_menu_item_picked(ak_menu_item* pMI);

drgui_element* drge_editor_create_main_menu_tool(drge_editor* pEditor, drgui_element* pParent, ak_window* pWindow)
{
    if (pEditor == NULL) {
        return NULL;
    }

    ak_theme* pTheme = ak_get_application_theme(pEditor->pAKApp);
    if (pTheme == NULL) {
        return NULL;
    }

    drgui_element* pMMTool = ak_create_tool(pEditor->pAKApp, pParent, DRGE_EDITOR_TOOL_TYPE_MAIN_MENU, sizeof(drge_editor_main_menu_tool), NULL);
    if (pMMTool == NULL) {
        return NULL;
    }

    drge_editor_main_menu_tool* pMM = ak_get_tool_extra_data(pMMTool);
    assert(pMM != NULL);
    pMM->pEditor = pEditor;

    pMM->pMBElement = ak_create_menu_bar(ak_get_application_gui(pEditor->pAKApp), pMMTool, sizeof(&pMMTool), &pMMTool);
    if (pMM->pMBElement == NULL) {
        ak_delete_tool(pMMTool);
        return NULL;
    }

    pMM->currentToolType[0] = '\0';
    pMM->currentToolReadOnly = false;


    // Create each menu.

    //// File ////
    pMM->pFileMenu = ak_create_menu(pEditor->pAKApp, pWindow, 0, NULL);
    ak_menu_set_size(pMM->pFileMenu, 256, 400);
    ak_menu_set_font(pMM->pFileMenu, pTheme->pUIFont);

    pMM->pFile_NewFile = drge_editor_create_main_menu_item(pMM->pFileMenu, "New File", "Ctrl+N", NULL, "File.NewFile");
    pMM->pFile_OpenFile = drge_editor_create_main_menu_item(pMM->pFileMenu, "Open File...", "Ctrl+O", NULL, "File.OpenFile");
    ak_create_separator_menu_item(pMM->pFileMenu, 0, NULL);
    pMM->pFile_Save = drge_editor_create_main_menu_item(pMM->pFileMenu, "Save", "Ctrl+S", NULL, "File.Save");
    pMM->pFile_SaveAs = drge_editor_create_main_menu_item(pMM->pFileMenu, "Save As...", NULL, NULL, "File.SaveAs");
    pMM->pFile_SaveAll = drge_editor_create_main_menu_item(pMM->pFileMenu, "Save All", NULL, NULL, "File.SaveAll");
    ak_create_separator_menu_item(pMM->pFileMenu, 0, NULL);
    pMM->pFile_Close = drge_editor_create_main_menu_item(pMM->pFileMenu, "Close", NULL, NULL, "File.Close");
    pMM->pFile_CloseAll = drge_editor_create_main_menu_item(pMM->pFileMenu, "Close All", NULL, NULL, "File.CloseAll");
    ak_create_separator_menu_item(pMM->pFileMenu, 0, NULL);
    pMM->pFile_Exit = drge_editor_create_main_menu_item(pMM->pFileMenu, "Exit", "Alt+F4", ak_image_library_get_cross(ak_get_image_library(pEditor->pAKApp), 1), "Global.Exit"); ak_mi_set_icon_tint(pMM->pFile_Exit, drgui_rgb(255, 128, 128));


    //// Edit ////
    pMM->pEditMenu = ak_create_menu(pEditor->pAKApp, pWindow, 0, NULL);
    ak_menu_set_font(pMM->pEditMenu, pTheme->pUIFont);
    ak_menu_set_size(pMM->pEditMenu, 400, 256);

    pMM->pEdit_Undo = drge_editor_create_main_menu_item(pMM->pEditMenu, "Undo", "Ctrl+Z", NULL, "Edit.Undo");
    pMM->pEdit_Redo = drge_editor_create_main_menu_item(pMM->pEditMenu, "Redo", "Ctrl+Y", NULL, "Edit.Redo");
    ak_create_separator_menu_item(pMM->pEditMenu, 0, NULL);
    pMM->pEdit_Cut = drge_editor_create_main_menu_item(pMM->pEditMenu, "Cut", "Ctrl+X", NULL, "Edit.Cut");
    pMM->pEdit_Copy = drge_editor_create_main_menu_item(pMM->pEditMenu, "Copy", "Ctrl+C", NULL, "Edit.Copy");
    pMM->pEdit_Paste = drge_editor_create_main_menu_item(pMM->pEditMenu, "Paste", "Ctrl+V", NULL, "Edit.Paste");
    pMM->pEdit_Delete = drge_editor_create_main_menu_item(pMM->pEditMenu, "Delete", "Del", NULL, "Edit.Delete");
    ak_create_separator_menu_item(pMM->pEditMenu, 0, NULL);
    pMM->pEdit_SelectAll = drge_editor_create_main_menu_item(pMM->pEditMenu, "Select All", "Ctrl+A", NULL, "Edit.SelectAll");



    //// View ////
    pMM->pViewMenu = ak_create_menu(pEditor->pAKApp, pWindow, 0, NULL);
    ak_menu_set_font(pMM->pViewMenu, pTheme->pUIFont);
    ak_menu_set_size(pMM->pViewMenu, 256, 400);

    pMM->pView_StatusBar = drge_editor_create_main_menu_item(pMM->pViewMenu, "Command Bar", NULL, NULL, "View.ToggleCommandBar");
    pMM->pView_LineNumbers = drge_editor_create_main_menu_item(pMM->pViewMenu, "Line Numbers", NULL, NULL, "View.ToggleLineNumbers");


    //// Find ////
    pMM->pFindMenu = ak_create_menu(pEditor->pAKApp, pWindow, 0, NULL);
    ak_menu_set_font(pMM->pFindMenu, pTheme->pUIFont);
    ak_menu_set_size(pMM->pFindMenu, 256, 400);

    pMM->pFind_Find = drge_editor_create_main_menu_item(pMM->pFindMenu, "Find...", "Ctrl+F", NULL, "Find.Find");
    pMM->pFind_Replace = drge_editor_create_main_menu_item(pMM->pFindMenu, "Find and Replace...", "Ctrl+Shift+F", NULL, "Find.Replace");
    ak_create_separator_menu_item(pMM->pFindMenu, 0, NULL);
    pMM->pFind_GoTo = drge_editor_create_main_menu_item(pMM->pFindMenu, "Go To...", "Ctrl+G", NULL, "Find.GoTo");


    //// Settings ////
    pMM->pSettingsMenu = ak_create_menu(pEditor->pAKApp, pWindow, 0, NULL);
    ak_menu_set_font(pMM->pSettingsMenu, pTheme->pUIFont);
    ak_menu_set_size(pMM->pSettingsMenu, 256, 400);

    pMM->pSettings_AllSettings = drge_editor_create_main_menu_item(pMM->pSettingsMenu, "All Settings...", NULL, NULL, NULL);


    //// Help ////
    pMM->pHelpMenu = ak_create_menu(pEditor->pAKApp, pWindow, 0, NULL);
    ak_menu_set_font(pMM->pHelpMenu, pTheme->pUIFont);
    ak_menu_set_size(pMM->pHelpMenu, 256, 400);

    pMM->pHelp_ViewHelp = drge_editor_create_main_menu_item(pMM->pHelpMenu, "View Help (Opens Web Browser)", NULL, NULL, "Help.ViewHelp");
    ak_create_separator_menu_item(pMM->pHelpMenu, 0, NULL);
    pMM->pHelp_About = drge_editor_create_main_menu_item(pMM->pHelpMenu, "About Bytecode Studio...", NULL, NULL, "Help.ShowAboutDialog");


    // Add each item.
    pMM->pFile     = drge_editor_create_main_menu_bar_item(pMMTool, "File",     pMM->pFileMenu);
    pMM->pEdit     = drge_editor_create_main_menu_bar_item(pMMTool, "Edit",     pMM->pEditMenu);
    pMM->pView     = drge_editor_create_main_menu_bar_item(pMMTool, "View",     pMM->pViewMenu);
    pMM->pFind     = drge_editor_create_main_menu_bar_item(pMMTool, "Find",     pMM->pFindMenu);
    pMM->pSettings = drge_editor_create_main_menu_bar_item(pMMTool, "Settings", pMM->pSettingsMenu);
    pMM->pHelp     = drge_editor_create_main_menu_bar_item(pMMTool, "Help",     pMM->pHelpMenu);


    // For now, have the control size it's children such that they're the same size.
    drgui_set_on_size(pMMTool, drgui_on_size_fit_children_to_parent);

    // Set the style.
    ak_mb_set_font(pMM->pMBElement, pTheme->pUIFont);


    // We want to have the main menu default to a state as if nothing is open.
    drge_editor_change_main_menu_by_tool_type(pMMTool, NULL, false);


    return pMMTool;
}

void drge_editor_delete_main_menu_tool(drgui_element* pMMTool)
{
    drge_editor_main_menu_tool* pMM = ak_get_tool_extra_data(pMMTool);
    if (pMM == NULL) {
        return;
    }

    // Menu bar first since it references sub-menus.
    ak_delete_menu_bar(pMM->pMBElement);
    
    ak_delete_menu(pMM->pFileMenu);
    ak_delete_menu(pMM->pEditMenu);
    ak_delete_menu(pMM->pViewMenu);
    ak_delete_menu(pMM->pFindMenu);
    ak_delete_menu(pMM->pSettingsMenu);
    ak_delete_menu(pMM->pHelpMenu);

    // Delete the base tool last.
    ak_delete_tool(pMMTool);
}


void drge_editor_change_main_menu_by_tool_type(drgui_element* pMMTool, const char* type, bool readOnly)
{
    drge_editor_main_menu_tool* pMM = ak_get_tool_extra_data(pMMTool);
    if (pMM == NULL) {
        return;
    }

    // If the tool type is the same we just leave everything unchanged.
    if (type != NULL && strcmp(pMM->currentToolType, type) == 0 && pMM->currentToolReadOnly == readOnly) {
        return;
    }

    // Revert the menu back to it's default state first.
    drge_editor_disable_text_editor_menu_items(pMMTool);


    // The tool type is allowed to be null, in which case we just leave the menu in it's default state.
    if (type == NULL) {
        pMM->currentToolType[0] = '\0';
        pMM->currentToolReadOnly = false;
        return;
    }


    if (ak_is_of_tool_type(type, DRGE_EDITOR_TOOL_TYPE_SUB_EDITOR)) {
        if (ak_is_of_tool_type(type, DRGE_EDITOR_TOOL_TYPE_TEXT_EDITOR)) {
            drge_editor_enable_text_editor_menu_items(pMMTool, readOnly);
        } else {
            drge_editor_enable_sub_editor_menu_items(pMMTool, readOnly);
        }
    }
    

    strcpy_s(pMM->currentToolType, sizeof(pMM->currentToolType), type);
    pMM->currentToolReadOnly = readOnly;
}


void drge_editor_disable_text_editor_menu_items(drgui_element* pMMTool)
{
    drge_editor_main_menu_tool* pMM = ak_get_tool_extra_data(pMMTool);
    if (pMM == NULL) {
        return;
    }

    ak_mi_disable(pMM->pFile_Save);
    ak_mi_disable(pMM->pFile_SaveAs);
    ak_mi_disable(pMM->pFile_SaveAll);
    ak_mi_disable(pMM->pFile_Close);
    ak_mi_disable(pMM->pFile_CloseAll);

    ak_mi_disable(pMM->pEdit_Undo);
    ak_mi_disable(pMM->pEdit_Redo);
    ak_mi_disable(pMM->pEdit_Cut);
    ak_mi_disable(pMM->pEdit_Copy);
    ak_mi_disable(pMM->pEdit_Paste);
    ak_mi_disable(pMM->pEdit_Delete);
    ak_mi_disable(pMM->pEdit_SelectAll);

    ak_mi_disable(pMM->pView_StatusBar);
    ak_mi_disable(pMM->pView_LineNumbers);

    ak_mi_disable(pMM->pFind_Find);
    ak_mi_disable(pMM->pFind_Replace);
    ak_mi_disable(pMM->pFind_GoTo);
}


void drge_editor_enable_sub_editor_menu_items(drgui_element* pMMTool, bool readOnly)
{
    drge_editor_main_menu_tool* pMM = ak_get_tool_extra_data(pMMTool);
    if (pMM == NULL) {
        return;
    }

    ak_mi_enable(pMM->pFile_SaveAs);
    ak_mi_enable(pMM->pFile_SaveAll);
    ak_mi_enable(pMM->pFile_Close);
    ak_mi_enable(pMM->pFile_CloseAll);

    if (!readOnly)
    {
        ak_mi_enable(pMM->pFile_Save);
    }
}

void drge_editor_enable_text_editor_menu_items(drgui_element* pMMTool, bool readOnly)
{
    drge_editor_enable_sub_editor_menu_items(pMMTool, readOnly);

    drge_editor_main_menu_tool* pMM = ak_get_tool_extra_data(pMMTool);
    if (pMM == NULL) {
        return;
    }

    ak_mi_enable(pMM->pEdit_SelectAll);

    ak_mi_enable(pMM->pView_StatusBar);
    ak_mi_enable(pMM->pView_LineNumbers);

    ak_mi_enable(pMM->pFind_Find);
    ak_mi_enable(pMM->pFind_GoTo);

    if (!readOnly)
    {
        ak_mi_enable(pMM->pEdit_Undo);
        ak_mi_enable(pMM->pEdit_Redo);
        ak_mi_enable(pMM->pEdit_Cut);
        ak_mi_enable(pMM->pEdit_Copy);
        ak_mi_enable(pMM->pEdit_Paste);
        ak_mi_enable(pMM->pEdit_Delete);

        ak_mi_enable(pMM->pFind_Replace);
    }
}




static ak_menu_bar_item* drge_editor_create_main_menu_bar_item(drgui_element* pMMTool, const char* text, ak_window* pMenu)
{
    if (text == NULL) {
        return NULL;
    }

    drge_editor_main_menu_tool* pMM = ak_get_tool_extra_data(pMMTool);
    if (pMM == NULL) {
        return NULL;
    }

    ak_menu_bar_item* pMBI = ak_create_menu_bar_item(pMM->pMBElement, pMenu, 0, NULL);
    if (pMBI == NULL) {
        return NULL;
    }

    ak_mbi_set_text(pMBI, text);

    return pMBI;
}

static ak_menu_item* drge_editor_create_main_menu_item(ak_window* pMenuWindow, const char* text, const char* shortcutStr, drgui_image* pIcon, const char* pActionName)
{
    ak_menu_item* pMI = ak_create_menu_item(pMenuWindow, sizeof(ide_main_menu_item), NULL);
    ak_mi_set_text(pMI, text);
    ak_mi_set_shortcut_text(pMI, shortcutStr);
    ak_mi_set_icon(pMI, pIcon);
    ak_mi_set_on_picked(pMI, drge_editor_on_main_menu_item_picked);

    ide_main_menu_item* pItem = ak_mi_get_extra_data(pMI);
    assert(pItem != NULL);

    pItem->pActionName  = pActionName;
    pItem->hideOnPicked = true;

    return pMI;
}

static ak_application* drge_editor_get_application_from_mmi(ak_menu_item* pMI)
{
    return ak_get_window_application(ak_mi_get_menu(pMI));
}

static void drge_editor_on_main_menu_item_picked(ak_menu_item* pMI)
{
    ide_main_menu_item* pItem = ak_mi_get_extra_data(pMI);
    assert(pItem != NULL);

    if (pItem->hideOnPicked) {
        ak_menu_hide(ak_mi_get_menu(pMI));
    }

    ak_handle_action(drge_editor_get_application_from_mmi(pMI), pItem->pActionName);
}
