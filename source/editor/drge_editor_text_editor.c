// Public domain. See "unlicense" statement at the end of dr_ge.h.

typedef struct
{
    // The text editor element where the actual text editing is done.
    drgui_element* pTextBox;

} drge_text_subeditor_data;

void drge_text_editor__tb_on_mouse_enter(drgui_element* pTB)
{
    ak_window* pWindow = ak_get_element_window(pTB);
    assert(pWindow != NULL);

    ak_set_window_cursor(pWindow, ak_cursor_type_ibeam);
}

void drge_text_editor__tb_on_mouse_leave(drgui_element* pTB)
{
    ak_window* pWindow = ak_get_element_window(pTB);
    assert(pWindow != NULL);

    ak_set_window_cursor(pWindow, ak_cursor_type_default);
}

void drge_text_editor__on_handle_action(drgui_element* pTextEditor, const char* pActionName)
{
    drge_text_subeditor_data* pTEData = ak_get_tool_extra_data((drgui_element*)pTextEditor);
    assert(pTEData != NULL);

    if (strcmp(pActionName, "Edit.SelectAll") == 0) {
        drgui_textbox_select_all(pTEData->pTextBox);
        return;
    }


    if (strcmp(pActionName, "Edit.Copy") == 0) {
        size_t selectedTextLength = drgui_textbox_get_selected_text(pTEData->pTextBox, NULL, 0);
        if (selectedTextLength > 0)
        {
            char* selectedText = malloc(selectedTextLength + 1);
            drgui_textbox_get_selected_text(pTEData->pTextBox, selectedText, selectedTextLength + 1);

            ak_clipboard_set_text(selectedText, selectedTextLength);

            free(selectedText);
        }

        return;
    }

    if (strcmp(pActionName, "Edit.Cut") == 0 && !drge_subeditor_is_read_only((drge_subeditor*)pTextEditor)) {
        size_t selectedTextLength = drgui_textbox_get_selected_text(pTEData->pTextBox, NULL, 0);
        if (selectedTextLength > 0)
        {
            char* selectedText = malloc(selectedTextLength + 1);
            drgui_textbox_get_selected_text(pTEData->pTextBox, selectedText, selectedTextLength + 1);

            ak_clipboard_set_text(selectedText, selectedTextLength);

            drgui_textbox_delete_selected_text(pTEData->pTextBox);
            free(selectedText);
        }

        return;
    }

    if (strcmp(pActionName, "Edit.Paste") == 0 && !drge_subeditor_is_read_only((drge_subeditor*)pTextEditor)) {
            char* clipboardText = ak_clipboard_get_text();
            drgui_textbox_insert_text_at_cursor(pTEData->pTextBox, clipboardText);
            ak_clipboard_free_text(clipboardText);
    }

    if (strcmp(pActionName, "Edit.Delete") == 0 && !drge_subeditor_is_read_only((drge_subeditor*)pTextEditor)) {
        if (drgui_textbox_is_anything_selected(pTEData->pTextBox)) {
            drgui_textbox_delete_selected_text(pTEData->pTextBox);
        } else {
            drgui_textbox_delete_character_to_right_of_cursor(pTEData->pTextBox);
        }
    }


    if (strcmp(pActionName, "Edit.Undo") == 0 && !drge_subeditor_is_read_only((drge_subeditor*)pTextEditor)) {
        drgui_textbox_undo(pTEData->pTextBox);
        return;
    }
    if (strcmp(pActionName, "Edit.Redo") == 0 && !drge_subeditor_is_read_only((drge_subeditor*)pTextEditor)) {
        drgui_textbox_redo(pTEData->pTextBox);
        return;
    }
}

drge_subeditor* drge_editor_create_text_editor(drge_editor* pEditor, const char* fileAbsolutePath)
{
    // A text editor is just a sub-editor.
    drge_subeditor* pTextEditor = drge_editor_create_sub_editor(pEditor, DRGE_EDITOR_TOOL_TYPE_TEXT_EDITOR, fileAbsolutePath);
    if (pTextEditor == NULL) {
        return NULL;
    }

    drgui_set_on_size((drgui_element*)pTextEditor, drgui_on_size_fit_children_to_parent);
    ak_tool_set_on_handle_action((drgui_element*)pTextEditor, drge_text_editor__on_handle_action);

    drge_text_subeditor_data* pTEData = ak_get_tool_extra_data((drgui_element*)pTextEditor);
    pTEData->pTextBox = drgui_create_textbox(ak_get_application_gui(pEditor->pAKApp), (drgui_element*)pTextEditor, 0, NULL);
    drgui_set_on_mouse_enter(pTEData->pTextBox, drge_text_editor__tb_on_mouse_enter);
    drgui_set_on_mouse_leave(pTEData->pTextBox, drge_text_editor__tb_on_mouse_leave);
    drgui_textbox_set_vertical_align(pTEData->pTextBox, drgui_text_layout_alignment_top);
    
    
    ak_theme* pTheme = ak_get_application_theme(pEditor->pAKApp);
    drgui_textbox_set_text_color(pTEData->pTextBox, pTheme->defaultText.textColor);
    drgui_textbox_set_background_color(pTEData->pTextBox, pTheme->defaultText.backgroundColor);
    drgui_textbox_set_active_line_background_color(pTEData->pTextBox, drgui_rgb(40, 40, 40));
    drgui_textbox_set_cursor_color(pTEData->pTextBox, pTheme->defaultText.textColor);
    drgui_textbox_set_font(pTEData->pTextBox, pTheme->defaultText.pFont);
    drgui_textbox_set_border_width(pTEData->pTextBox, 0);
    drgui_textbox_set_padding(pTEData->pTextBox, 2);

    char* fileData = drvfs_open_and_read_text_file(pEditor->pContext->pVFS, fileAbsolutePath, NULL);
    drgui_textbox_set_text(pTEData->pTextBox, fileData);

    drvfs_free(fileData);
    return pTextEditor;
}

void drge_editor_delete_text_editor(drge_subeditor* pTextEditor)
{
    drge_editor_delete_sub_editor(pTextEditor);
}
