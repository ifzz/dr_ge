
typedef struct
{
    /// the line number.
    size_t lineNumber;

    /// The column number.
    size_t columnNumber;

    /// The character number.
    size_t characterNumber;

} drge_editor_command_bar__text_info;

typedef struct
{
    // A pointer to the editor that owns this command bar.
    drge_editor* pEditor;


    /// The command text box.
    drgui_element* pCmdTB;


    /// The container for when an unknown tool type is active.
    drgui_element* pNullInfo;


    /// The container for text editor info.
    drgui_element* pTextEditorInfo;

    /// The text editor info.
    drge_editor_command_bar__text_info textEditorInfo;


    /// A pointer to the currently active info container. This is changed depending on the context.
    drgui_element* pCurrentInfoContainer;


    /// The font to use when drawing text in info containers.
    drgui_font* pFont;

    /// The text color.
    drgui_color textColor;

    /// The background color.
    drgui_color bgColor;

} drge_editor_command_bar;

// A helper function for retrieving the info container for the given tool type.
static drgui_element* drge_editor_command_bar__get_info_container_by_tool_type(drgui_element* pCmdBarTool, const char* pToolType);

// on_size()
static void drge_editor_command_bar__on_size(drgui_element* pCmdBarTool, float newWidth, float newHeight);

// on_paint()
static void drge_editor_command_bar__on_paint(drgui_element* pCmdBarTool, drgui_rect relativeRect, void* pPaintData);

// on_cmdtb_key_down
static void drge_editor_command_bar__on_cmdtb_key_down(drgui_element* pCmdTB, drgui_key key, int stateFlags);

// on_cmdtb_printable_key_down
static void drge_editor_command_bar__on_cmdtb_printable_key_down(drgui_element* pCmdTB, unsigned int utf32, int stateFlags);

// on_cmdtb_capture_mouse
static void drge_editor_command_bar__on_cmdtb_capture_keyboard(drgui_element* pCmdTB, drgui_element* pPrevCapturedElement);

// on_cmdtb_release_mouse
static void drge_editor_command_bar__on_cmdtb_release_keyboard(drgui_element* pCmdTB, drgui_element* pNewCapturedElement);


// A helper function for creating a container for a specific tool type.
static drgui_element* drge_editor_command_bar__create_info_container(drgui_element* pCmdBarTool, drgui_on_paint_proc onPaint);

// A helper function for creating the info container for unkown or unsupported tools.
static drgui_element* drge_editor_command_bar__create_info_container__null(drgui_element* pCmdBarTool);

// A helper function for creating the info container for DRGE_EDITOR_TOOL_TYPE_COMMAND_BAR tools.
static drgui_element* drge_editor_command_bar__create_info_container__text_editor(drgui_element* pCmdBarTool);


drgui_element* drge_editor_create_command_bar_tool(drge_editor* pEditor, drgui_element* pParent)
{
    if (pEditor == NULL) {
        return NULL;
    }

    ak_theme* pTheme = ak_get_application_theme(pEditor->pAKApp);
    assert(pTheme != NULL);

    drgui_element* pCmdBarTool = ak_create_tool(pEditor->pAKApp, pParent, DRGE_EDITOR_TOOL_TYPE_COMMAND_BAR, sizeof(drge_editor_command_bar), NULL);
    if (pCmdBarTool == NULL) {
        return NULL;
    }

    drgui_set_on_size(pCmdBarTool, drge_editor_command_bar__on_size);
    drgui_set_on_paint(pCmdBarTool, drge_editor_command_bar__on_paint);


    drge_editor_command_bar* pCmdBar = ak_get_tool_extra_data(pCmdBarTool);
    assert(pCmdBar != NULL);

    pCmdBar->pEditor = pEditor;
    pCmdBar->pCmdTB = ak_create_textbox(pEditor->pAKApp, pCmdBarTool, sizeof(&pCmdBarTool), &pCmdBarTool);
    drgui_set_on_capture_keyboard(pCmdBar->pCmdTB, drge_editor_command_bar__on_cmdtb_capture_keyboard);
    drgui_set_on_release_keyboard(pCmdBar->pCmdTB, drge_editor_command_bar__on_cmdtb_release_keyboard);
    drgui_set_on_key_down(pCmdBar->pCmdTB, drge_editor_command_bar__on_cmdtb_key_down);
    drgui_set_on_printable_key_down(pCmdBar->pCmdTB, drge_editor_command_bar__on_cmdtb_printable_key_down);

    drgui_textbox_set_vertical_align(pCmdBar->pCmdTB, drgui_text_layout_alignment_center);
    drgui_textbox_set_border_width(pCmdBar->pCmdTB, 0);
    drgui_textbox_set_padding(pCmdBar->pCmdTB, 2);
    drgui_textbox_set_background_color(pCmdBar->pCmdTB, drgui_rgb(64, 64, 64));
    drgui_textbox_set_active_line_background_color(pCmdBar->pCmdTB, drgui_rgb(64, 64, 64));

    pCmdBar->pNullInfo       = drge_editor_command_bar__create_info_container__null(pCmdBarTool);
    pCmdBar->pTextEditorInfo = drge_editor_command_bar__create_info_container__text_editor(pCmdBarTool);
    pCmdBar->textEditorInfo.lineNumber = 0;
    pCmdBar->textEditorInfo.columnNumber = 0;
    pCmdBar->textEditorInfo.characterNumber = 0;
    pCmdBar->pCurrentInfoContainer = NULL;
    pCmdBar->pFont = NULL;
    pCmdBar->textColor = drgui_rgb(224, 224, 224);
    pCmdBar->bgColor = drgui_rgb(64, 64, 64);

    return pCmdBarTool;
}

void drge_editor_delete_command_bar_tool(drgui_element* pCmdBarTool)
{
    drge_editor_command_bar* pCmdBar = ak_get_tool_extra_data(pCmdBarTool);
    if (pCmdBar == NULL) {
        return;
    }

    drgui_delete_element(pCmdBar->pTextEditorInfo);

    ak_delete_tool(pCmdBarTool);
}


void drge_editor_command_bar_set_context_by_tool_type(drgui_element* pCmdBarTool, const char* pToolType)
{
    drge_editor_command_bar* pCmdBar = ak_get_tool_extra_data(pCmdBarTool);
    if (pCmdBar == NULL) {
        return;
    }

    // We need to hide the old info container first.
    drgui_hide(pCmdBar->pCurrentInfoContainer);

    pCmdBar->pCurrentInfoContainer = drge_editor_command_bar__get_info_container_by_tool_type(pCmdBarTool, pToolType);
    if (pCmdBar->pCurrentInfoContainer != NULL)
    {
        // The container needs to be resized which we'll do by just emulating an on_size event.
        drge_editor_command_bar__on_size(pCmdBarTool, drgui_get_width(pCmdBarTool), drgui_get_height(pCmdBarTool));
        drgui_show(pCmdBar->pCurrentInfoContainer);
    }
}


void drge_editor_command_bar_set_font(drgui_element* pCmdBarTool, drgui_font* pFont)
{
    drge_editor_command_bar* pCmdBar = ak_get_tool_extra_data(pCmdBarTool);
    if (pCmdBar == NULL) {
        return;
    }

    pCmdBar->pFont = pFont;

    if (pCmdBar->pCurrentInfoContainer) {
        drgui_dirty(pCmdBar->pCurrentInfoContainer, drgui_get_local_rect(pCmdBar->pCurrentInfoContainer));
    }
}

void drge_editor_command_bar_set_text_color(drgui_element* pCmdBarTool, drgui_color color)
{
    drge_editor_command_bar* pCmdBar = ak_get_tool_extra_data(pCmdBarTool);
    if (pCmdBar == NULL) {
        return;
    }

    pCmdBar->textColor = color;

    if (pCmdBar->pCurrentInfoContainer) {
        drgui_dirty(pCmdBar->pCurrentInfoContainer, drgui_get_local_rect(pCmdBar->pCurrentInfoContainer));
    }
}

void drge_editor_command_bar_set_background_color(drgui_element* pCmdBarTool, drgui_color color)
{
    drge_editor_command_bar* pCmdBar = ak_get_tool_extra_data(pCmdBarTool);
    if (pCmdBar == NULL) {
        return;
    }

    pCmdBar->bgColor = color;

    if (pCmdBar->pCurrentInfoContainer) {
        drgui_dirty(pCmdBar->pCurrentInfoContainer, drgui_get_local_rect(pCmdBar->pCurrentInfoContainer));
    }
}


void drge_editor_command_bar_capture_keyboard(drgui_element* pCmdBarTool)
{
    drge_editor_command_bar* pCmdBar = ak_get_tool_extra_data(pCmdBarTool);
    if (pCmdBar == NULL) {
        return;
    }

    drgui_capture_keyboard(pCmdBar->pCmdTB);
}

void drge_editor_command_bar_release_keyboard(drgui_element* pCmdBarTool)
{
    drge_editor_command_bar* pCmdBar = ak_get_tool_extra_data(pCmdBarTool);
    if (pCmdBar == NULL) {
        return;
    }

    if (drgui_get_element_with_keyboard_capture(pCmdBarTool->pContext) == pCmdBar->pCmdTB) {
        drgui_release_keyboard(pCmdBarTool->pContext);
    }
}

void drge_editor_command_bar_set_command_text(drgui_element* pCmdBarTool, const char* text)
{
    drge_editor_command_bar* pCmdBar = ak_get_tool_extra_data(pCmdBarTool);
    if (pCmdBar == NULL) {
        return;
    }

    drgui_textbox_set_text(pCmdBar->pCmdTB, text);
    drgui_textbox_move_cursor_to_end_of_text(pCmdBar->pCmdTB);
}


static drgui_element* drge_editor_command_bar__get_info_container_by_tool_type(drgui_element* pCmdBarTool, const char* pToolType)
{
    drge_editor_command_bar* pCmdBar = ak_get_tool_extra_data(pCmdBarTool);
    if (pCmdBar == NULL || pToolType == NULL) {
        return NULL;
    }

    if (ak_is_of_tool_type(pToolType, DRGE_EDITOR_TOOL_TYPE_TEXT_EDITOR)) {
        return pCmdBar->pTextEditorInfo;
    } else {
        return pCmdBar->pNullInfo;
    }
}

static void drge_editor_command_bar__on_size(drgui_element* pCmdBarTool, float newWidth, float newHeight)
{
    // The current info container needs to be resized to half the width of the main element.
    drge_editor_command_bar* pCmdBar = ak_get_tool_extra_data(pCmdBarTool);
    if (pCmdBar == NULL) {
        return;
    }

    drgui_set_size(pCmdBar->pCmdTB, newWidth/2, newHeight);

    if (pCmdBar->pCurrentInfoContainer)
    {
        drgui_set_size(pCmdBar->pCurrentInfoContainer, newWidth / 2, newHeight);
        drgui_set_relative_position(pCmdBar->pCurrentInfoContainer, newWidth / 2, 0); // Right align.
    }
}

static void drge_editor_command_bar__on_paint(drgui_element* pCmdBarTool, drgui_rect relativeRect, void* pPaintData)
{
    (void)relativeRect;
    (void)pPaintData;

    drge_editor_command_bar* pCmdBar = ak_get_tool_extra_data(pCmdBarTool);
    if (pCmdBar == NULL) {
        return;
    }
}


static void drge_editor_command_bar__on_cmdtb_key_down(drgui_element* pCmdTB, drgui_key key, int stateFlags)
{
    drgui_element* pCmdBarTool = *(drgui_element**)ak_textbox_get_extra_data(pCmdTB);
    assert(pCmdBarTool != NULL);

    switch (key)
    {
        case DRGUI_ESCAPE:
        {
            drge_editor_command_bar_release_keyboard(pCmdBarTool);
        } break;

        default:
        {
            drgui_textbox_on_key_down(pCmdTB, key, stateFlags);
        } break;
    }
}

static void drge_editor_command_bar__on_cmdtb_printable_key_down(drgui_element* pCmdTB, unsigned int utf32, int stateFlags)
{
    // We want to intercept the enter key and prevent it from being sent to the normal event handler.
    if (utf32 == '\n' || utf32 == '\r') {
        drgui_element* pCmdBarTool = *(drgui_element**)ak_textbox_get_extra_data(pCmdTB);
        assert(pCmdBarTool != NULL);

        drge_editor_command_bar* pCmdBar = ak_get_tool_extra_data(pCmdBarTool);
        assert(pCmdBar != NULL);

        int execResult = 0;

        size_t cmdLen = drgui_textbox_get_text(pCmdBar->pCmdTB, NULL, 0);
        char* cmd = malloc(cmdLen + 1);
        if (drgui_textbox_get_text(pCmdBar->pCmdTB, cmd, cmdLen + 1) == cmdLen) {
            execResult = ak_exec(ak_get_tool_application(pCmdBarTool), cmd);
        }

        if ((execResult & DRGE_CMDBAR_NO_CLEAR) == 0) {
            drgui_textbox_set_text(pCmdBar->pCmdTB, "");
        }

        if ((execResult & DRGE_CMDBAR_RELEASE_KEYBOARD) != 0) {
            drge_editor_command_bar_release_keyboard(pCmdBarTool);
        }

        free(cmd);
    } else {
        drgui_textbox_on_printable_key_down(pCmdTB, utf32, stateFlags);
    }
}

static void drge_editor_command_bar__on_cmdtb_capture_keyboard(drgui_element* pCmdTB, drgui_element* pPrevCapturedElement)
{
    drgui_element* pCmdBarTool = *(drgui_element**)ak_textbox_get_extra_data(pCmdTB);
    assert(pCmdBarTool != NULL);

    drge_editor_command_bar* pCmdBar = ak_get_tool_extra_data(pCmdBarTool);
    assert(pCmdBar != NULL);

    ak_textbox_on_capture_keyboard(pCmdBar->pCmdTB, pPrevCapturedElement);
    drge_editor_on_command_bar_capture_keyboard(pCmdBar->pEditor, pPrevCapturedElement);
}

static void drge_editor_command_bar__on_cmdtb_release_keyboard(drgui_element* pCmdTB, drgui_element* pNewCapturedElement)
{
    drgui_element* pCmdBarTool = *(drgui_element**)ak_textbox_get_extra_data(pCmdTB);
    assert(pCmdBarTool != NULL);

    drge_editor_command_bar* pCmdBar = ak_get_tool_extra_data(pCmdBarTool);
    assert(pCmdBar != NULL);

    ak_textbox_on_release_keyboard(pCmdBar->pCmdTB, pNewCapturedElement);
    drge_editor_on_command_bar_release_keyboard(pCmdBar->pEditor, pNewCapturedElement);
}



//// NULL Context ////

static void drge_editor_command_bar__on_paint__null(drgui_element* pContainerElement, drgui_rect relativeRect, void* pPaintData)
{
    drgui_element* pCmdBarTool = *(drgui_element**)drgui_get_extra_data(pContainerElement);
    assert(pCmdBarTool != NULL);

    drge_editor_command_bar* pCmdBar = ak_get_tool_extra_data(pCmdBarTool);
    if (pCmdBar == NULL) {
        return;
    }

    drgui_draw_rect(pContainerElement, drgui_get_local_rect(pContainerElement), pCmdBar->bgColor, pPaintData);
}



//// Text Editor Context ////

void drge_editor_command_bar_set_text_editor_line_number(drgui_element* pCmdBarTool, size_t lineNumber)
{
    drge_editor_command_bar* pCmdBar = ak_get_tool_extra_data(pCmdBarTool);
    if (pCmdBar == NULL) {
        return;
    }

    pCmdBar->textEditorInfo.lineNumber = lineNumber;

    if (pCmdBar->pCurrentInfoContainer == pCmdBar->pTextEditorInfo) {
        drgui_dirty(pCmdBar->pCurrentInfoContainer, drgui_get_local_rect(pCmdBar->pCurrentInfoContainer));
    }
}

void drge_editor_command_bar_set_text_editor_column_number(drgui_element* pCmdBarTool, size_t columnNumber)
{
    drge_editor_command_bar* pCmdBar = ak_get_tool_extra_data(pCmdBarTool);
    if (pCmdBar == NULL) {
        return;
    }

    pCmdBar->textEditorInfo.columnNumber = columnNumber;

    if (pCmdBar->pCurrentInfoContainer == pCmdBar->pTextEditorInfo) {
        drgui_dirty(pCmdBar->pCurrentInfoContainer, drgui_get_local_rect(pCmdBar->pCurrentInfoContainer));
    }
}

void drge_editor_command_bar_set_text_editor_character_number(drgui_element* pCmdBarTool, size_t characterNumber)
{
    drge_editor_command_bar* pCmdBar = ak_get_tool_extra_data(pCmdBarTool);
    if (pCmdBar == NULL) {
        return;
    }

    pCmdBar->textEditorInfo.characterNumber = characterNumber;

    if (pCmdBar->pCurrentInfoContainer == pCmdBar->pTextEditorInfo) {
        drgui_dirty(pCmdBar->pCurrentInfoContainer, drgui_get_local_rect(pCmdBar->pCurrentInfoContainer));
    }
}

static void drge_editor_command_bar__on_paint__text_editor(drgui_element* pContainerElement, drgui_rect relativeRect, void* pPaintData)
{
    (void)relativeRect;

    drgui_element* pCmdBarTool = *(drgui_element**)drgui_get_extra_data(pContainerElement);
    assert(pCmdBarTool != NULL);

    drge_editor_command_bar* pCmdBar = ak_get_tool_extra_data(pCmdBarTool);
    if (pCmdBar == NULL) {
        return;
    }

    float scaleX = 1;
    float scaleY = 1;

    const float segmentWidth = 96;
    const float containerHeight = drgui_get_height(pContainerElement);

    drgui_font_metrics fontMetrics;
    drgui_get_font_metrics(pCmdBar->pFont, scaleX, scaleY, &fontMetrics);


    char lnStr[64];
    int lnStrLen = snprintf(lnStr, sizeof(lnStr), "Ln %llu", (uint64_t)pCmdBar->textEditorInfo.lineNumber);

    char colStr[64];
    int colStrLen = snprintf(colStr, sizeof(colStr), "Col %llu", (uint64_t)pCmdBar->textEditorInfo.columnNumber);


    float offsetX = drgui_get_width(pContainerElement) - (segmentWidth * 2);
    float offsetY = ((containerHeight - fontMetrics.lineHeight) / 2) + 1;


    // The part to the left of the text.
    drgui_draw_rect(pContainerElement, drgui_make_rect(0, 0, offsetX, containerHeight), pCmdBar->bgColor, pPaintData);


    // Ln
    drgui_draw_text(pContainerElement, pCmdBar->pFont, lnStr, lnStrLen, offsetX, offsetY, pCmdBar->textColor, pCmdBar->bgColor, pPaintData);

    float lnWidth;
    drgui_measure_string(pCmdBar->pFont, lnStr, lnStrLen, scaleX, scaleY, &lnWidth, NULL);
    drgui_draw_rect(pContainerElement, drgui_make_rect(offsetX + lnWidth, 0, offsetX + segmentWidth, containerHeight), pCmdBar->bgColor, pPaintData);
    drgui_draw_rect(pContainerElement, drgui_make_rect(offsetX, 0, offsetX + lnWidth, offsetY), pCmdBar->bgColor, pPaintData);
    drgui_draw_rect(pContainerElement, drgui_make_rect(offsetX, offsetY + fontMetrics.lineHeight, offsetX + lnWidth, containerHeight), pCmdBar->bgColor, pPaintData);


    // Col
    offsetX += segmentWidth;
    drgui_draw_text(pContainerElement, pCmdBar->pFont, colStr, colStrLen, offsetX, offsetY, pCmdBar->textColor, pCmdBar->bgColor, pPaintData);

    float colWidth;
    drgui_measure_string(pCmdBar->pFont, colStr, colStrLen, scaleX, scaleY, &colWidth, NULL);
    drgui_draw_rect(pContainerElement, drgui_make_rect(offsetX + colWidth, 0, offsetX + segmentWidth, containerHeight), pCmdBar->bgColor, pPaintData);
    drgui_draw_rect(pContainerElement, drgui_make_rect(offsetX, 0, offsetX + colWidth, offsetY), pCmdBar->bgColor, pPaintData);
    drgui_draw_rect(pContainerElement, drgui_make_rect(offsetX, offsetY + fontMetrics.lineHeight, offsetX + colWidth, containerHeight), pCmdBar->bgColor, pPaintData);
}





static drgui_element* drge_editor_command_bar__create_info_container(drgui_element* pCmdBarTool, drgui_on_paint_proc onPaint)
{
    drge_editor_command_bar* pCmdBar = ak_get_tool_extra_data(pCmdBarTool);
    if (pCmdBar == NULL) {
        return NULL;
    }

    drgui_element* pInfoContainer = drgui_create_element(pCmdBarTool->pContext, pCmdBarTool, sizeof(&pCmdBarTool), &pCmdBarTool);
    if (pInfoContainer == NULL) {
        return NULL;
    }

    drgui_set_on_paint(pInfoContainer, onPaint);

    return pInfoContainer;
}

static drgui_element* drge_editor_command_bar__create_info_container__null(drgui_element* pCmdBarTool)
{
    return drge_editor_command_bar__create_info_container(pCmdBarTool, drge_editor_command_bar__on_paint__null);
}

static drgui_element* drge_editor_command_bar__create_info_container__text_editor(drgui_element* pCmdBarTool)
{
    return drge_editor_command_bar__create_info_container(pCmdBarTool, drge_editor_command_bar__on_paint__text_editor);
}