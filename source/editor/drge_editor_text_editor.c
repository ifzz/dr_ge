// Public domain. See "unlicense" statement at the end of dr_ge.h.

typedef struct
{
    // The text editor element where the actual text editing is done.
    drgui_element* pTextBox;

} drge_text_subeditor_data;

static void drge_text_editor__on_capture_keyboard(drgui_element* pTextEditor, drgui_element* pPrevCapturedElement)
{
    // When the main tool receives keyboard focus we actually want to divert it to the text box.
    drge_text_subeditor_data* pTEData = drge_subeditor_get_extra_data(pTextEditor);
    assert(pTEData != NULL);

    if (pPrevCapturedElement != pTEData->pTextBox) {
        drgui_capture_keyboard(pTEData->pTextBox);
    }
}

static void drge_text_editor__on_handle_action(drgui_element* pTextEditor, const char* pActionName)
{
    drge_text_subeditor_data* pTEData = drge_subeditor_get_extra_data(pTextEditor);
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


static void drge_text_editor__on_key_down__textbox(drgui_element* pTextBox, drgui_key key, int stateFlags)
{
    drgui_element* pTextEditor = *(drgui_element**)ak_textbox_get_extra_data(pTextBox);
    assert(pTextEditor != NULL);

    drge_text_subeditor_data* pTEData = drge_subeditor_get_extra_data(pTextEditor);
    assert(pTEData != NULL);

    switch (key)
    {
        case DRGUI_ESCAPE:
        {
            drge_editor_focus_command_bar(drge_subeditor_get_editor(pTextEditor));
        } break;

        default: drgui_textbox_on_key_down(pTEData->pTextBox, key, stateFlags);
    }
}

static void drge_text_editor__on_cursor_move__textbox(drgui_element* pTextBox)
{
    // The editor's status bar will need to be updated based on the caret's new position.
    drgui_element* pTextEditor = *(drgui_element**)ak_textbox_get_extra_data(pTextBox);
    assert(pTextEditor != NULL);

    drge_editor_update_status_bar(drge_subeditor_get_editor(pTextEditor));
}

drge_subeditor* drge_editor_create_text_editor(drge_editor* pEditor, const char* fileAbsolutePath)
{
    // A text editor is just a sub-editor.
    drge_subeditor* pTextEditor = drge_editor_create_sub_editor(pEditor, DRGE_EDITOR_TOOL_TYPE_TEXT_EDITOR, fileAbsolutePath, sizeof(drge_text_subeditor_data));
    if (pTextEditor == NULL) {
        return NULL;
    }

    drgui_set_on_size((drgui_element*)pTextEditor, drgui_on_size_fit_children_to_parent);
    drgui_set_on_capture_keyboard((drgui_element*)pTextEditor, drge_text_editor__on_capture_keyboard);
    ak_tool_set_on_handle_action((drgui_element*)pTextEditor, drge_text_editor__on_handle_action);

    drge_text_subeditor_data* pTEData = drge_subeditor_get_extra_data(pTextEditor);
    pTEData->pTextBox = ak_create_textbox(pEditor->pAKApp, (drgui_element*)pTextEditor, sizeof(&pTextEditor), &pTextEditor);
    drgui_set_on_key_down(pTEData->pTextBox, drge_text_editor__on_key_down__textbox);
    drgui_textbox_set_on_cursor_move(pTEData->pTextBox, drge_text_editor__on_cursor_move__textbox);
    drgui_textbox_set_vertical_align(pTEData->pTextBox, drgui_text_layout_alignment_top);
    
    
    ak_theme* pTheme = ak_get_application_theme(pEditor->pAKApp);
    drgui_textbox_set_active_line_background_color(pTEData->pTextBox, drgui_rgb(40, 40, 40));
    drgui_textbox_set_border_width(pTEData->pTextBox, 0);

    char* fileData = drvfs_open_and_read_text_file(pEditor->pContext->pVFS, fileAbsolutePath, NULL);
    drgui_textbox_set_text(pTEData->pTextBox, fileData);

    drvfs_free(fileData);
    return pTextEditor;
}

void drge_editor_delete_text_editor(drge_subeditor* pTextEditor)
{
    drge_editor_delete_sub_editor(pTextEditor);
}


void drge_editor_text_subeditor__capture_keyboard(drge_subeditor* pTextEditor)
{
    drge_text_subeditor_data* pTEData = drge_subeditor_get_extra_data(pTextEditor);
    assert(pTEData != NULL);

    drgui_capture_keyboard(pTEData->pTextBox);
}

void drge_editor_text_subeditor__release_keyboard(drge_subeditor* pTextEditor)
{
    drge_text_subeditor_data* pTEData = drge_subeditor_get_extra_data(pTextEditor);
    assert(pTEData != NULL);

    drgui_release_keyboard(pTextEditor->pContext);
}


void drge_editor_text_subeditor__show_line_numbers(drge_subeditor* pTextEditor)
{
    drge_text_subeditor_data* pTEData = drge_subeditor_get_extra_data(pTextEditor);
    assert(pTEData != NULL);

    // TODO: Implement me.
}

void drge_editor_text_subeditor__hide_line_numbers(drge_subeditor* pTextEditor)
{
    drge_text_subeditor_data* pTEData = drge_subeditor_get_extra_data(pTextEditor);
    assert(pTEData != NULL);

    // TODO: Implement me.
}


size_t drge_text_editor__get_cursor_line(drge_subeditor* pTextEditor)
{
    drge_text_subeditor_data* pTEData = drge_subeditor_get_extra_data(pTextEditor);
    assert(pTEData != NULL);

    return drgui_textbox_get_cursor_line(pTEData->pTextBox);
}

size_t drge_text_editor__get_cursor_column(drge_subeditor* pTextEditor)
{
    drge_text_subeditor_data* pTEData = drge_subeditor_get_extra_data(pTextEditor);
    assert(pTEData != NULL);

    return drgui_textbox_get_cursor_column(pTEData->pTextBox);
}
