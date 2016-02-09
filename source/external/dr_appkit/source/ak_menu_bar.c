// Public domain. See "unlicense" statement at the end of this file.

typedef struct ak_menu_bar ak_menu_bar;

struct ak_menu_bar
{
    /// The first menu item.
    ak_menu_bar_item* pFirstItem;

    /// The last menu item.
    ak_menu_bar_item* pLastItem;


    /// Keeps track of the hovered item.
    ak_menu_bar_item* pFocusedItem;

    /// Keeps track of whether or not the menu bar is expanded. That is, whether or not a menu is opened.
    bool isExpanded;


    /// Keeps track of whether or not the next mouse down event should be ignored. We ignore mouse down events when
    /// the mouse was clicked outside the region popup window (and thus resulting in an automatic hide) but inside
    /// the region of the associated menu bar item. If we don't do this we will not achieve the toggle effect.
    bool blockNextMouseDown;


    /// Keeps track of whether or not the mouse is over the menu bar.
    bool isMouseOver;

    /// The position of the mouse on the x axis based on the last occurance of the on_mouse_move event.
    int relativeMousePosX;

    /// The position of the mouse on the y axis based on the last occurance of the on_mouse_move event.
    int relativeMousePosY;


    /// The font to use for the text of menu bar items.
    drgui_font* pFont;

    /// The color of the text of menu bar items.
    drgui_color textColor;

    /// The background color of the menu bar.
    drgui_color backgroundColor;

    /// The background color of hovered menu bar items.
    drgui_color backgroundColorHovered;

    /// The background color of expanded menu bar items.
    drgui_color backgroundColorExpanded;

    /// The border color of expanded menu bar items.
    drgui_color borderColorExpanded;

    /// The width of the border of expanded menu bar items.
    float borderWidthExpanded;

    /// The amount of padding to apply to menu bar items on each side on the x axis.
    float itemPaddingX;


    /// The function to call when an item needs to be measured.
    ak_mbi_on_measure_proc onItemMeasure;

    /// The function to call when an item needs to be painted.
    ak_mbi_on_paint_proc onItemPaint;


    /// The size of the extra data.
    size_t extraDataSize;

    /// A pointer to the extra data.
    char pExtraData[1];
};

struct ak_menu_bar_item
{
    /// A pointer to the menu bar element that owns the given menu bar item.
    drgui_element* pMBElement;

    /// The menu that's associated with the given menu bar.
    ak_window* pMenu;


    /// The text of the item.
    char text[AK_MAX_MENU_BAR_ITEM_TEXT_LENGTH];


    /// The next item in the list.
    ak_menu_bar_item* pNextItem;

    /// The previous item in the list.
    ak_menu_bar_item* pPrevItem;


    /// The size of the extra data.
    size_t extraDataSize;

    /// A pointer to the extra data.
    char pExtraData[1];
};



///////////////////////////////////////////////////////////////////////////////
//
// Menu Bar
//
///////////////////////////////////////////////////////////////////////////////

/// Finds the menu bar item under the given point.
static ak_menu_bar_item* ak_mb_find_item_under_point(drgui_element* pMBElement, float relativePosX, float relativePosY);

/// Finds the metrics of the given menu bar item (position and size).
static bool ak_mb_find_item_metrics(ak_menu_bar_item* pMBI, float* pPosXOut, float* pPosYOut, float* pWidthOut, float* pHeightOut);

/// The default implementation of the function to call when a menu bar item needs to be measured.
static void ak_on_mmbi_measure_default(ak_menu_bar_item* pMBI, float* pWidthOut, float* pHeightOut);

/// The default implementation of the function to call when a menu bar item needs to be painted.
static void ak_on_mmbi_paint_default(drgui_element* pMBElement, ak_menu_bar_item* pMBI, drgui_rect clippingRect, float offsetX, float offsetY, float width, float height, void* pPaintData);

/// Called when a menu is hidden.
static void ak_mb_on_menu_hide(ak_window* pMenu, unsigned int flags, void* pUserData);

/// Called when a menu is shown.
static void ak_mb_on_menu_show(ak_window* pMenu, void* pUserData);

drgui_element* ak_create_menu_bar(drgui_context* pContext, drgui_element* pParent, size_t extraDataSize, const void* pExtraData)
{
    drgui_element* pMBElement = drgui_create_element(pContext, pParent, sizeof(ak_menu_bar) - sizeof(char) + extraDataSize, NULL);
    if (pMBElement == NULL) {
        return NULL;
    }

    ak_menu_bar* pMB = drgui_get_extra_data(pMBElement);
    assert(pMB != NULL);

    pMB->pFirstItem              = NULL;
    pMB->pLastItem               = NULL;
    pMB->pFocusedItem            = NULL;
    pMB->isExpanded              = false;
    pMB->blockNextMouseDown      = false;
    pMB->isMouseOver             = false;
    pMB->relativeMousePosX       = 0;
    pMB->relativeMousePosY       = 0;
    pMB->pFont                   = NULL;
    pMB->textColor               = drgui_rgb(224, 224, 224);
    pMB->backgroundColor         = drgui_rgb(40, 40, 40);
    pMB->backgroundColorHovered  = drgui_rgb(96, 96, 96);
    pMB->backgroundColorExpanded = drgui_rgb(32, 32, 32);
    pMB->borderColorExpanded     = drgui_rgb(64, 64, 64);
    pMB->borderWidthExpanded     = 1;
    pMB->itemPaddingX            = 8;
    pMB->onItemMeasure           = ak_on_mmbi_measure_default;
    pMB->onItemPaint             = ak_on_mmbi_paint_default;

    pMB->extraDataSize = extraDataSize;
    if (pExtraData != NULL) {
        memcpy(pMB->pExtraData, pExtraData, extraDataSize);
    }

    // Events.
    drgui_set_on_mouse_leave(pMBElement, ak_mb_on_mouse_leave);
    drgui_set_on_mouse_move(pMBElement, ak_mb_on_mouse_move);
    drgui_set_on_mouse_button_down(pMBElement, ak_mb_on_mouse_button_down);
    drgui_set_on_paint(pMBElement, ak_mb_on_paint);

    return pMBElement;
}

void ak_delete_menu_bar(drgui_element* pMBElement)
{
    ak_menu_bar* pMB = drgui_get_extra_data(pMBElement);
    if (pMB == NULL) {
        return;
    }

    while (pMB->pFirstItem != NULL)
    {
        ak_delete_menu_bar_item(pMB->pFirstItem);
    }

    drgui_delete_element(pMBElement);
}


size_t ak_mb_get_extra_data_size(drgui_element* pMBElement)
{
    ak_menu_bar* pMB = drgui_get_extra_data(pMBElement);
    if (pMB == NULL) {
        return 0;
    }

    return pMB->extraDataSize;
}

void* ak_mb_get_extra_data(drgui_element* pMBElement)
{
    ak_menu_bar* pMB = drgui_get_extra_data(pMBElement);
    if (pMB == NULL) {
        return NULL;
    }

    return pMB->pExtraData;
}


void ak_mb_set_font(drgui_element* pMBElement, drgui_font* pFont)
{
    ak_menu_bar* pMB = drgui_get_extra_data(pMBElement);
    if (pMB == NULL) {
        return;
    }

    pMB->pFont = pFont;
}

drgui_font* ak_mb_get_font(drgui_element* pMBElement)
{
    ak_menu_bar* pMB = drgui_get_extra_data(pMBElement);
    if (pMB == NULL) {
        return NULL;
    }

    return pMB->pFont;
}

void ak_mb_set_text_color(drgui_element* pMBElement, drgui_color color)
{
    ak_menu_bar* pMB = drgui_get_extra_data(pMBElement);
    if (pMB == NULL) {
        return;
    }

    pMB->textColor = color;
}

drgui_color ak_mb_get_text_color(drgui_element* pMBElement)
{
    ak_menu_bar* pMB = drgui_get_extra_data(pMBElement);
    if (pMB == NULL) {
        return drgui_rgb(0, 0, 0);
    }

    return pMB->textColor;
}

void ak_mb_set_default_background_color(drgui_element* pMBElement, drgui_color color)
{
    ak_menu_bar* pMB = drgui_get_extra_data(pMBElement);
    if (pMB == NULL) {
        return;
    }

    pMB->backgroundColor = color;
}

drgui_color ak_mb_get_default_background_color(drgui_element* pMBElement)
{
    ak_menu_bar* pMB = drgui_get_extra_data(pMBElement);
    if (pMB == NULL) {
        return drgui_rgb(0, 0, 0);
    }

    return pMB->backgroundColor;
}

void ak_mb_set_hovered_background_color(drgui_element* pMBElement, drgui_color color)
{
    ak_menu_bar* pMB = drgui_get_extra_data(pMBElement);
    if (pMB == NULL) {
        return;
    }

    pMB->backgroundColorHovered = color;
}

drgui_color ak_mb_get_hovered_background_color(drgui_element* pMBElement)
{
    ak_menu_bar* pMB = drgui_get_extra_data(pMBElement);
    if (pMB == NULL) {
        return drgui_rgb(0, 0, 0);
    }

    return pMB->backgroundColorHovered;
}

void ak_mb_set_expanded_background_color(drgui_element* pMBElement, drgui_color color)
{
    ak_menu_bar* pMB = drgui_get_extra_data(pMBElement);
    if (pMB == NULL) {
        return;
    }

    pMB->backgroundColorExpanded = color;
}

drgui_color ak_mb_get_expanded_background_color(drgui_element* pMBElement)
{
    ak_menu_bar* pMB = drgui_get_extra_data(pMBElement);
    if (pMB == NULL) {
        return drgui_rgb(0, 0, 0);
    }

    return pMB->backgroundColorExpanded;
}

void ak_mb_set_expanded_border_color(drgui_element* pMBElement, drgui_color color)
{
    ak_menu_bar* pMB = drgui_get_extra_data(pMBElement);
    if (pMB == NULL) {
        return;
    }

    pMB->borderColorExpanded = color;
}

drgui_color ak_mb_get_expanded_border_color(drgui_element* pMBElement)
{
    ak_menu_bar* pMB = drgui_get_extra_data(pMBElement);
    if (pMB == NULL) {
        return drgui_rgb(0, 0, 0);
    }

    return pMB->borderColorExpanded;
}

void ak_mb_set_item_padding_x(drgui_element* pMBElement, float padding)
{
    ak_menu_bar* pMB = drgui_get_extra_data(pMBElement);
    if (pMB == NULL) {
        return;
    }

    pMB->itemPaddingX = padding;
}

float ak_mb_get_item_padding_x(drgui_element* pMBElement)
{
    ak_menu_bar* pMB = drgui_get_extra_data(pMBElement);
    if (pMB == NULL) {
        return 0;
    }

    return pMB->itemPaddingX;
}


void ak_mb_show_item_menu(drgui_element* pMBElement, ak_menu_bar_item* pMBI)
{
    if (pMBI == NULL || pMBI->pMBElement != pMBElement) {
        return;
    }

    ak_menu_bar* pMB = drgui_get_extra_data(pMBElement);
    if (pMB == NULL) {
        return;
    }

    float scaleX;
    float scaleY;
    drgui_get_absolute_inner_scale(pMBElement, &scaleX, &scaleY);


    // Hide the currently visible menu first, if any.
    ak_mb_hide_item_menu(pMBElement);

    // Now show the menu in the correct position. This depends on the layout of the item.
    float itemPosX;
    float itemPosY;
    float itemWidth;
    float itemHeight;
    if (ak_mb_find_item_metrics(pMBI, &itemPosX, &itemPosY, &itemWidth, &itemHeight))
    {
        // TODO: If the menu would fall outside the container window, clamp the position such that it does not overhang.
        float menuPosX = itemPosX;
        float menuPosY = itemPosY;
        ak_menu_set_position(pMBI->pMenu, (int)(menuPosX * scaleX), (int)(menuPosY * scaleY));

        // The mask is based on the position of the menu relative to the position of the menu bar item.
        ak_menu_set_border_mask(pMBI->pMenu, ak_menu_border_top, itemPosX - menuPosX + pMB->borderWidthExpanded, (itemWidth - pMB->borderWidthExpanded*2));
    }

    ak_menu_show(pMBI->pMenu);
}

void ak_mb_hide_item_menu(drgui_element* pMBElement)
{
    ak_menu_bar* pMB = drgui_get_extra_data(pMBElement);
    if (pMB == NULL) {
        return;
    }

    if (pMB->isExpanded && pMB->pFocusedItem != NULL)
    {
        ak_menu_hide(pMB->pFocusedItem->pMenu);
    }
}


void ak_mb_set_on_mbi_measure(drgui_element* pMBElement, ak_mbi_on_measure_proc proc)
{
    ak_menu_bar* pMB = drgui_get_extra_data(pMBElement);
    if (pMB == NULL) {
        return;
    }

    pMB->onItemMeasure = proc;
}

void ak_mb_set_on_mbi_paint(drgui_element* pMBElement, ak_mbi_on_paint_proc proc)
{
    ak_menu_bar* pMB = drgui_get_extra_data(pMBElement);
    if (pMB == NULL) {
        return;
    }

    pMB->onItemPaint = proc;
}


void ak_mb_on_mouse_leave(drgui_element* pMBElement)
{
    ak_menu_bar* pMB = drgui_get_extra_data(pMBElement);
    if (pMB == NULL) {
        return;
    }

    pMB->isMouseOver = false;

    if (pMB->pFocusedItem != NULL)
    {
        if (!pMB->isExpanded)
        {
            pMB->pFocusedItem = NULL;
            drgui_dirty(pMBElement, drgui_get_local_rect(pMBElement));
        }
    }
}

void ak_mb_on_mouse_move(drgui_element* pMBElement, int relativeMousePosX, int relativeMousePosY, int stateFlags)
{
    (void)stateFlags;

    ak_menu_bar* pMB = drgui_get_extra_data(pMBElement);
    if (pMB == NULL) {
        return;
    }

    pMB->isMouseOver       = true;
    pMB->relativeMousePosX = relativeMousePosX;
    pMB->relativeMousePosY = relativeMousePosY;

    ak_menu_bar_item* pOldFocusedItem = pMB->pFocusedItem;
    ak_menu_bar_item* pNewFocusedItem = ak_mb_find_item_under_point(pMBElement, (float)relativeMousePosX, (float)relativeMousePosY);

    if (pOldFocusedItem != pNewFocusedItem)
    {
        if (pMB->isExpanded) {
            ak_mb_show_item_menu(pMBElement, pNewFocusedItem);
        } else {
            pMB->pFocusedItem = pNewFocusedItem;
        }


        // Schedule a redraw to show the new hovered state.
        drgui_dirty(pMBElement, drgui_get_local_rect(pMBElement));
    }
}

void ak_mb_on_mouse_button_down(drgui_element* pMBElement, int mouseButton, int relativeMousePosX, int relativeMousePosY, int stateFlags)
{
    (void)stateFlags;
    (void)mouseButton;
    (void)relativeMousePosX;
    (void)relativeMousePosY;

    ak_menu_bar* pMB = drgui_get_extra_data(pMBElement);
    if (pMB == NULL) {
        return;
    }

    if (pMB->blockNextMouseDown) {
        pMB->blockNextMouseDown = false;
        return;
    }


    if (pMB->pFocusedItem != NULL)
    {
        if (pMB->isExpanded) {
            ak_mb_hide_item_menu(pMBElement);
        } else {
            ak_mb_show_item_menu(pMBElement, pMB->pFocusedItem);
        }

        // Schedule a redraw to show the new expanded state.
        drgui_dirty(pMBElement, drgui_get_local_rect(pMBElement));
    }
}

void ak_mb_on_paint(drgui_element* pMBElement, drgui_rect relativeClippingRect, void* pPaintData)
{
    ak_menu_bar* pMB = drgui_get_extra_data(pMBElement);
    if (pMB == NULL) {
        return;
    }

    float runningPosX = 0;
    if (pMB->onItemMeasure && pMB->onItemPaint)
    {
        for (ak_menu_bar_item* pMBI = pMB->pFirstItem; pMBI != NULL; pMBI = pMBI->pNextItem)
        {
            float width;
            float height;
            pMB->onItemMeasure(pMBI, &width, &height);
            pMB->onItemPaint(pMBElement, pMBI, relativeClippingRect, runningPosX, 0, width, height, pPaintData);

            runningPosX += width;
        }
    }

    // The rest of the background needs to be drawn using the default background color.
    drgui_draw_rect(pMBElement, drgui_make_rect(runningPosX, 0, drgui_get_width(pMBElement), drgui_get_height(pMBElement)), pMB->backgroundColor, pPaintData);
}


static ak_menu_bar_item* ak_mb_find_item_under_point(drgui_element* pMBElement, float relativePosX, float relativePosY)
{
    ak_menu_bar* pMB = drgui_get_extra_data(pMBElement);
    assert(pMB != NULL);

    float runningPosX = 0;
    if (pMB->onItemMeasure)
    {
        for (ak_menu_bar_item* pMBI = pMB->pFirstItem; pMBI != NULL; pMBI = pMBI->pNextItem)
        {
            float width;
            float height;
            pMB->onItemMeasure(pMBI, &width, &height);

            if (relativePosX >= runningPosX && relativePosX < runningPosX + width && relativePosY >= 0 && relativePosY < height) {
                return pMBI;
            }

            runningPosX += width;
        }
    }

    return NULL;
}


static bool ak_mb_find_item_metrics(ak_menu_bar_item* pMBI, float* pPosXOut, float* pPosYOut, float* pWidthOut, float* pHeightOut)
{
    ak_menu_bar* pMB = drgui_get_extra_data(pMBI->pMBElement);
    if (pMB == NULL) {
        return false;
    }

    float runningPosX = 0;
    if (pMB->onItemMeasure)
    {
        for (ak_menu_bar_item* pNextMBI = pMB->pFirstItem; pNextMBI != NULL; pNextMBI = pNextMBI->pNextItem)
        {
            float width;
            float height;
            pMB->onItemMeasure(pNextMBI, &width, &height);

            if (pNextMBI == pMBI)
            {
                if (pPosXOut) {
                    *pPosXOut = runningPosX;
                }
                if (pPosYOut) {
                    *pPosYOut = height;
                }

                if (pWidthOut) {
                    *pWidthOut = width;
                }
                if (pHeightOut) {
                    *pHeightOut = height;
                }

                return true;
            }

            runningPosX += width;
        }
    }

    return false;
}

static void ak_on_mmbi_measure_default(ak_menu_bar_item* pMBI, float* pWidthOut, float* pHeightOut)
{
    assert(pWidthOut  != NULL);
    assert(pHeightOut != NULL);

    ak_menu_bar* pMB = drgui_get_extra_data(pMBI->pMBElement);
    if (pMB == NULL)
    {
        *pWidthOut  = 0;
        *pHeightOut = 0;
        return;
    }

    float innerScaleX;
    float innerScaleY;
    drgui_get_absolute_inner_scale(pMBI->pMBElement, &innerScaleX, &innerScaleY);


    float textWidth;
    float textHeight;
    if (!drgui_measure_string(pMB->pFont, pMBI->text, strlen(pMBI->text), innerScaleX, innerScaleY, &textWidth, &textHeight)) {
        textWidth = 0;
    }

    *pWidthOut  = textWidth + (pMB->itemPaddingX*2);
    *pHeightOut = drgui_get_height(pMBI->pMBElement);
}

static void ak_on_mmbi_paint_default(drgui_element* pMBElement, ak_menu_bar_item* pMBI, drgui_rect clippingRect, float offsetX, float offsetY, float width, float height, void* pPaintData)
{
    (void)clippingRect;

    ak_menu_bar* pMB = drgui_get_extra_data(pMBI->pMBElement);
    if (pMB == NULL) {
        return;
    }

    float innerScaleX;
    float innerScaleY;
    drgui_get_absolute_inner_scale(pMBElement, &innerScaleX, &innerScaleY);

    float textWidth;
    float textHeight;
    if (!drgui_measure_string(pMB->pFont, pMBI->text, strlen(pMBI->text), innerScaleX, innerScaleY, &textWidth, &textHeight)) {
        textWidth  = 0;
        textHeight = 0;
    }

    float borderWidth = 0;
    drgui_color bgcolor = pMB->backgroundColor;
    if (pMB->pFocusedItem == pMBI) {
        bgcolor = pMB->backgroundColorHovered;

        if (pMB->isExpanded)
        {
            bgcolor = pMB->backgroundColorExpanded;
            borderWidth = 1;
        }
    }


    float textPosX = pMB->itemPaddingX;
    float textPosY = (height - textHeight) / 2;

    // Padding around text.
    drgui_draw_rect(pMBElement, drgui_make_rect(offsetX + 0,                    offsetY + 0,                     offsetX + textPosX,                                 offsetY + height),   bgcolor, pPaintData); // Left
    drgui_draw_rect(pMBElement, drgui_make_rect(offsetX + textPosX + textWidth, offsetY + 0,                     offsetX + textPosX + textWidth + pMB->itemPaddingX, offsetY + height),   bgcolor, pPaintData); // Right
    drgui_draw_rect(pMBElement, drgui_make_rect(offsetX + textPosX,             offsetY + 0,                     offsetX + textPosX + textWidth,                     offsetY + textPosY), bgcolor, pPaintData); // Top
    drgui_draw_rect(pMBElement, drgui_make_rect(offsetX + textPosX,             offsetY + textPosY + textHeight, offsetX + textPosX + textWidth,                     offsetY + height),   bgcolor, pPaintData); // Bottom

    // Border.
    if (borderWidth > 0)
    {
        drgui_draw_rect(pMBElement, drgui_make_rect(offsetX + borderWidth,         offsetY + 0, offsetX + width - borderWidth, offsetY + borderWidth), pMB->borderColorExpanded, pPaintData);
        drgui_draw_rect(pMBElement, drgui_make_rect(offsetX + 0,                   offsetY + 0, offsetX + borderWidth,         offsetY + height),      pMB->borderColorExpanded, pPaintData);
        drgui_draw_rect(pMBElement, drgui_make_rect(offsetX + width - borderWidth, offsetY + 0, offsetX + width,               offsetY + height),      pMB->borderColorExpanded, pPaintData);
    }

    // Text.
    drgui_draw_text(pMBElement, pMB->pFont, pMBI->text, (int)strlen(pMBI->text), offsetX + textPosX, offsetY + textPosY, pMB->textColor, bgcolor /*drgui_rgb(255, 128, 128)*/, pPaintData);
}

static void ak_mb_on_menu_hide(ak_window* pMenu, unsigned int flags, void* pUserData)
{
    (void)pMenu;

    drgui_element* pMBElement = pUserData;
    if (pMBElement == NULL) {
        return;
    }

    ak_menu_bar* pMB = drgui_get_extra_data(pMBElement);
    if (pMB == NULL) {
        return;
    }

    ak_menu_bar_item* pItemUnderPoint = ak_mb_find_item_under_point(pMBElement, (float)pMB->relativeMousePosX, (float)pMB->relativeMousePosY);

    pMB->blockNextMouseDown = pMB->isMouseOver && pItemUnderPoint && (flags & AK_AUTO_HIDE_FROM_LOST_FOCUS) != 0;
    pMB->isExpanded = false;

    if (!pMB->blockNextMouseDown && pMB->isMouseOver) {
        pMB->pFocusedItem = pItemUnderPoint;
    } else {
        pMB->pFocusedItem = NULL;
    }

    // Schedule a redraw to show the new expanded state.
    drgui_dirty(pMBElement, drgui_get_local_rect(pMBElement));
}

static void ak_mb_on_menu_show(ak_window* pMenu, void* pUserData)
{
    drgui_element* pMBElement = pUserData;
    if (pMBElement == NULL) {
        return;
    }

    ak_menu_bar* pMB = drgui_get_extra_data(pMBElement);
    if (pMB == NULL) {
        return;
    }

    // We need to find the item this menu belongs to and then set that as the focused item.
    ak_menu_bar_item* pNewFocusedItem = NULL;
    for (ak_menu_bar_item* pMBI = pMB->pFirstItem; pMBI != NULL; pMBI = pMBI->pNextItem)
    {
        if (pMBI->pMenu == pMenu) {
            pNewFocusedItem = pMBI;
            break;
        }
    }

    if (pNewFocusedItem != NULL) {
        pMB->pFocusedItem = pNewFocusedItem;
        pMB->isExpanded = true;
    }

    // Schedule a redraw to show the new expanded state.
    drgui_dirty(pMBElement, drgui_get_local_rect(pMBElement));
}


///////////////////////////////////////////////////////////////////////////////
//
// Menu Bar Item
//
///////////////////////////////////////////////////////////////////////////////

/// Apopends the given item to the given menu bar.
static void ak_mbi_append(ak_menu_bar_item* pMBI, drgui_element* pMBElement);

/// Detaches the given item from the menu bar.
static void ak_mbi_detach(ak_menu_bar_item* pMBI);

ak_menu_bar_item* ak_create_menu_bar_item(drgui_element* pMBElement, ak_window* pMenu, size_t extraDataSize, const void* pExtraData)
{
    if (pMBElement == NULL) {
        return NULL;
    }

    ak_menu_bar_item* pMBI = malloc(sizeof(ak_menu_bar_item) - sizeof(char) + extraDataSize);
    if (pMBI == NULL) {
        return NULL;
    }

    pMBI->pMBElement    = pMBElement;
    pMBI->pMenu         = pMenu;
    pMBI->text[0]       = '\0';
    pMBI->pNextItem     = NULL;
    pMBI->pPrevItem     = NULL;

    pMBI->extraDataSize = extraDataSize;
    if (pExtraData != NULL) {
        memcpy(pMBI->pExtraData, pExtraData, extraDataSize);
    }

    ak_menu_set_on_show(pMenu, ak_mb_on_menu_show, pMBElement);
    ak_menu_set_on_hide(pMenu, ak_mb_on_menu_hide, pMBElement);

    ak_mbi_append(pMBI, pMBElement);

    return pMBI;
}

void ak_delete_menu_bar_item(ak_menu_bar_item* pMBI)
{
    if (pMBI == NULL) {
        return;
    }

    ak_mbi_detach(pMBI);
    free(pMBI);
}


drgui_element* ak_mbi_get_menu_bar(ak_menu_bar_item* pMBI)
{
    if (pMBI == NULL) {
        return NULL;
    }

    return pMBI->pMBElement;
}

ak_window* ak_mbi_get_menu(ak_menu_bar_item* pMBI)
{
    if (pMBI == NULL) {
        return NULL;
    }

    return pMBI->pMenu;
}

size_t ak_mbi_get_extra_data_size(ak_menu_bar_item* pMBI)
{
    if (pMBI == NULL) {
        return 0;
    }

    return pMBI->extraDataSize;
}

void* ak_mbi_get_extra_data(ak_menu_bar_item* pMBI)
{
    if (pMBI == NULL) {
        return NULL;
    }

    return pMBI->pExtraData;
}


void ak_mbi_set_text(ak_menu_bar_item* pMBI, const char* text)
{
    if (pMBI == NULL) {
        return;
    }

    strcpy_s(pMBI->text, sizeof(pMBI->text), text);
}

const char* ak_mbi_get_text(ak_menu_bar_item* pMBI)
{
    if (pMBI == NULL) {
        return NULL;
    }

    return pMBI->text;
}


static void ak_mbi_append(ak_menu_bar_item* pMBI, drgui_element* pMBElement)
{
    assert(pMBI != NULL);

    ak_menu_bar* pMB = drgui_get_extra_data(pMBElement);
    assert(pMB != NULL);

    pMBI->pMBElement = pMBElement;
    if (pMB->pFirstItem == NULL)
    {
        assert(pMB->pLastItem == NULL);

        pMB->pFirstItem = pMBI;
        pMB->pLastItem  = pMBI;
    }
    else
    {
        assert(pMB->pLastItem != NULL);

        pMBI->pPrevItem = pMB->pLastItem;

        pMB->pLastItem->pNextItem = pMBI;
        pMB->pLastItem = pMBI;
    }


    // The content of the menu has changed so we'll need to schedule a redraw.
    drgui_dirty(pMBElement, drgui_get_local_rect(pMBElement));
}

static void ak_mbi_detach(ak_menu_bar_item* pMBI)
{
    assert(pMBI != NULL);

    ak_menu_bar* pMB = drgui_get_extra_data(pMBI->pMBElement);
    assert(pMB != NULL);


    if (pMBI->pNextItem != NULL) {
        pMBI->pNextItem->pPrevItem = pMBI->pPrevItem;
    }

    if (pMBI->pPrevItem != NULL) {
        pMBI->pPrevItem->pNextItem = pMBI->pNextItem;
    }


    if (pMBI == pMB->pFirstItem) {
        pMB->pFirstItem = pMBI->pNextItem;
    }

    if (pMBI == pMB->pLastItem) {
        pMB->pLastItem = pMBI->pPrevItem;
    }


    pMBI->pNextItem  = NULL;
    pMBI->pPrevItem  = NULL;
    pMBI->pMBElement = NULL;


    // The content of the menu has changed so we'll need to schedule a redraw.
    drgui_dirty(pMBI->pMBElement, drgui_get_local_rect(pMBI->pMBElement));
}


/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>
*/
