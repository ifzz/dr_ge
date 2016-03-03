// Public domain. See "unlicense" statement at the end of dr_ge.h.

// A sub-editor is just a GUI element and can be safely cast between the two.
typedef struct drgui_element drge_subeditor;

typedef bool (* drge_subeditor_save_proc)(drge_subeditor* pSubEditor, const char* absolutePathToSaveAs);


// Creates a sub-editor tool. Sub-editors are usually tied to a file, but not always so filePath can be NULL. Note
// that this is just an allocation function - you will likely want to call this from a higher level function such
// as drge_editor_create_text_editor().
drge_subeditor* drge_editor_create_sub_editor(drge_editor* pEditor, const char* type, const char* fileAbsolutePath, size_t extraDataSize);

// Deletes the given sub editor.
void drge_editor_delete_sub_editor(drge_subeditor* pSubEditor);

// Retrieves a pointer to the editor that owns this sub-editor.
drge_editor* drge_subeditor_get_editor(drge_subeditor* pSubEditor);

// Retrieves the absolute path of the file the given sub-editor is linked to.
const char* drge_subeditor_get_absolute_path(drge_subeditor* pSubEditor);

// Retrieves the size of the extra data associated with the given sub-editor.
size_t drge_subeditor_get_extra_data_size(drge_subeditor* pSubEditor);

// Retrieves a pointer to the extra data associated with the given sub-editor.
void* drge_subeditor_get_extra_data(drge_subeditor* pSubEditor);


// Saves the given sub-editor to the given file path.
bool drge_subeditor_save_to_file(drge_subeditor* pSubEditor, const char* absolutePathToSaveAs);


// Sets the function to call when the given sub-editor needs to be saved.
void drge_subeditor_set_save_proc(drge_subeditor* pSubEditor, drge_subeditor_save_proc proc);


// Updates the title of the given sub-editor based on the name of the file it's linked to.
//
// If the path of the linked file is empty, the text will be set to "[New File]". If isModified is set to true, an
// asterix will be placed at the end.
void drge_subeditor_update_title(drge_subeditor* pSubEditor, bool isModified);

// Determines whether or not the given sub-editor is marked as read only. This is based on the file the sub-editor
// is linked to.
bool drge_subeditor_is_read_only(drge_subeditor* pSubEditor);