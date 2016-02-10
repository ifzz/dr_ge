// Public domain. See "unlicense" statement at the end of dr_ge.h.

// A sub-editor is just a GUI element and can be safely cast between the two.
typedef struct easygui_element drge_subeditor;

// Creates a sub-editor tool. Sub-editors are usually tied to a file, but not always so filePath can be NULL. Note
// that this is just an allocation function - you will likely want to call this from a higher level function such
// as drge_editor_create_text_editor().
drge_subeditor* drge_editor_create_sub_editor(drge_editor* pEditor, const char* type, const char* fileAbsolutePath);

// Deletes the given sub editor.
void drge_editor_delete_sub_editor(drge_subeditor* pSubEditor);