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


// Base structure representing an asset.
typedef struct
{
#define DRGE_BASE_ASSET_ATTRIBS \
    drge_asset_type type; \
    drge_context* pContext; \
    unsigned int referenceCount; \
    char absolutePath[DRVFS_MAX_PATH];

    DRGE_BASE_ASSET_ATTRIBS
} drge_asset;

// Structure containing information about an image asset.
typedef struct
{
    DRGE_BASE_ASSET_ATTRIBS

    // The width of the image.
    unsigned int width;

    // The height of the image.
    unsigned int height;

    // The format of the image data.
    drge_image_format format;

    // A pointer to the raw image data.
    char pImageData[1];

} drge_image_asset;

// Structure containing information about a material asset.
typedef struct
{
    DRGE_BASE_ASSET_ATTRIBS
} drge_material_asset;

// Structure containing information about a model asset.
typedef struct
{
    DRGE_BASE_ASSET_ATTRIBS
} drge_model_asset;



// Loads an asset from the given path.
drge_asset* drge_load_asset(drge_context* pContext, const char* path);

// Unloads the given asset.
void drge_unload_asset(drge_asset* pAsset);


// Increments the reference counter of the given asset.
void drge_grab_asset(drge_asset* pAsset);

// Decrements the reference counter of the given asset. This does not delete the asset.
unsigned int drge_release_asset(drge_asset* pAsset);


// Retrieves a pointer to an already-loaded asset by it's path.
drge_asset* drge_find_existing_asset_by_path(drge_context* pContext, const char* path);


// Dynamically casts the given asset to an image asset.
//
// Returns NULL if the asset is not an image asset.
drge_image_asset* drge_to_image_asset(drge_asset* pAsset);

// Dynamically casts the given asset to a material asset.
//
// Returns NULL if the asset is not a material asset.
drge_material_asset* drge_to_material_asset(drge_asset* pAsset);

// Dynamically casts the given asset to a model asset.
//
// Returns NULL if the asset is not a model asset.
drge_model_asset* drge_to_model_asset(drge_asset* pAsset);