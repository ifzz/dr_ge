// Public domain. See "unlicense" statement at the end of dr_ge.h.

typedef enum
{
    drge_asset_type_unknown,
    drge_asset_type_image,
    drge_asset_type_model,
    drge_asset_type_material,
    drge_asset_type_sound,
    drge_asset_type_scene,
    drge_asset_type_text,
} drge_asset_type;

// Retrieves the type of the asset from the given file path.
drge_asset_type drge_get_asset_type_from_path(const char* path);