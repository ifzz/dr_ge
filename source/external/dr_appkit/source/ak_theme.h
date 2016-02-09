// Public domain. See "unlicense" statement at the end of this file.

#ifndef ak_theme_h
#define ak_theme_h

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ak_application ak_application;
typedef struct ak_theme ak_theme;
typedef struct ak_text_theme ak_text_theme;

struct ak_text_theme
{
    /// The font.
    drgui_font* pFont;

    /// The text color.
    drgui_color textColor;

    /// The background color.
    drgui_color backgroundColor;
};

struct ak_theme
{
    //// Colors ////

    // TESTING
    drgui_color baseColor;



    //// Tabs ////

    /// The default color of the background of a tab.
    drgui_color tabColor;

    /// The background color of hovered tabs.
    drgui_color tabHoveredColor;

    /// The background color of active tabs.
    drgui_color tabActiveColor;

    /// The padding of each tab.
    float tabPaddingLeft;
    float tabPaddingTop;
    float tabPaddingRight;
    float tabPaddingBottom;



    //// Menus ////

    /// The height of a menu bar.
    float menuBarHeight;

    /// The padding to apply to menu bar items.
    float menuBarItemPaddingX;




    //// Fonts ////

    /// The font to use for normal UI elements, but not text editors.
    drgui_font* pUIFont;

    /// The color to use for the normal UI elements, but not text editor.
    drgui_color uiFontColor;

    /// The metrics of the cross glyph.
    drgui_glyph_metrics uiCrossMetrics;



    //// Default Text Editor ///

    /// The theme for the text of the default text editor - those that are not tied to a particular syntax highlighter.
    ak_text_theme defaultText;

};


/// Loads the default theme.
void ak_theme_load_defaults(ak_application* pApplication, ak_theme* pTheme);

/// Loads a theme config file.
void ak_theme_load_from_file(ak_application* pApplication, ak_theme* pTheme, const char* absolutePath);

/// Unloads the given theme.
void ak_theme_unload(ak_theme* pTheme);


#ifdef __cplusplus
}
#endif

#endif


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