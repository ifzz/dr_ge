// Public domain. See "unlicense" statement at the end of dr_ge.h.

drge_asset_type drge_get_asset_type_from_path(const char* path)
{
    const char* ext = drpath_extension(path);
    if (ext == NULL || ext[0] == '\0') {
        return drge_asset_type_unknown;
    }

    if (_stricmp(ext, "png") == 0 ||
        _stricmp(ext, "tga") == 0 ||
        _stricmp(ext, "jpg") == 0 ||
        _stricmp(ext, "psd") == 0)
    {
        return drge_asset_type_image;
    }

    if (_stricmp(ext, "wav")  == 0 ||
        _stricmp(ext, "flac") == 0 ||
        _stricmp(ext, "ogg")  == 0)
    {
        return drge_asset_type_sound;
    }

    if (_stricmp(ext, "txt") == 0)
    {
        return drge_asset_type_text;
    }


    return drge_asset_type_unknown;
}


int drge__stbi_read(void* user, char *data, int size)
{
    drvfs_file* pFile = user;
    assert(pFile != NULL);

    size_t bytesRead;
    if (drvfs_read(pFile, data, (size_t)size, &bytesRead)) {
        return (int)bytesRead;
    }

    return 0;
}

void drge__stbi_skip(void* user, int n)
{
    drvfs_file* pFile = user;
    assert(pFile != NULL);

    drvfs_seek(pFile, n, drvfs_origin_current);
}

int drge__stbi_eof(void* user)
{
    drvfs_file* pFile = user;
    assert(pFile != NULL);

    return drvfs_eof(pFile);
}

drge_asset* drge__load_image_asset_from_file(drge_context* pContext, drvfs_file* pFile, const char* path)
{
    (void)path;

    assert(pContext != NULL);
    assert(pFile != NULL);
    assert(absolutePath != NULL);

    stbi_io_callbacks cb;
    cb.read = drge__stbi_read;
    cb.skip = drge__stbi_skip;
    cb.eof  = drge__stbi_eof;

    int imageWidth;
    int imageHeight;
    stbi_uc* pImageData = stbi_load_from_callbacks(&cb, pFile, &imageWidth, &imageHeight, NULL, 4);
    if (pImageData == NULL) {
        return NULL;
    }

    size_t imageDataSize = imageWidth*imageHeight*4;
    drge_image_asset* pImageAsset = malloc(sizeof(drge_image_asset) - sizeof(pImageAsset->pImageData) + imageDataSize);
    if (pImageAsset == NULL) {
        stbi_image_free(pImageData);
        return NULL;
    }

    pImageAsset->width  = imageWidth;
    pImageAsset->height = imageHeight;
    pImageAsset->format = drge_image_format_rgba8;
    memcpy(pImageAsset->pImageData, pImageData, imageDataSize);

    stbi_image_free(pImageData);
    return (drge_asset*)pImageAsset;
}

void drge__unload_image_asset(drge_asset* pAsset)
{
    assert(pAsset != NULL);
    assert(pAsset->type == drge_asset_type_image);

    drge_image_asset* pImageAsset = (drge_image_asset*)pAsset;
    if (pImageAsset == NULL) {
        return;
    }

    free(pImageAsset);
}


void drge__cache_asset(drge_asset* pAsset)
{
    assert(pAsset != NULL);
    assert(pAsset->referenceCount == 1);

    // TODO: Implement.
}

void drge__uncache_asset(drge_asset* pAsset)
{
    assert(pAsset != NULL);
    assert(pAsset->referenceCount == 0);

    // TODO: Implement.
}


drge_asset* drge_load_asset(drge_context* pContext, const char* path)
{
    drge_asset_type type = drge_get_asset_type_from_path(path);
    if (type == drge_asset_type_unknown) {
        return NULL;
    }

    drge_asset* pExistingAsset = drge_find_existing_asset_by_path(pContext, path);
    if (pExistingAsset != NULL) {
        drge_grab_asset(pExistingAsset);
        return pExistingAsset;
    }

    drvfs_file_info fi;
    if (!drvfs_get_file_info(pContext->pVFS, path, &fi)) {
        return NULL;    // File doesn't exist.
    }

    drvfs_file* pFile = drvfs_open(pContext->pVFS, path, DRVFS_READ, 0);
    if (pFile == NULL) {
        return NULL;
    }

   
    drge_asset* pAsset = NULL;
    switch (type)
    {
        case drge_asset_type_image:
        {
            pAsset = (drge_asset*)drge__load_image_asset_from_file(pContext, pFile, path);
        } break;


        default: break;
    }


    drvfs_close(pFile);
    if (pAsset == NULL) {
        return NULL;
    }

    pAsset->type           = type;
    pAsset->pContext       = pContext;
    pAsset->referenceCount = 0;
    strcpy_s(pAsset->absolutePath, sizeof(pAsset->absolutePath), fi.absolutePath);

    // Grab the asset so that the reference counter is incremented and the asset added to the cache.
    drge_grab_asset(pAsset);

    return pAsset;
}

void drge_unload_asset(drge_asset* pAsset)
{
    if (pAsset == NULL) {
        return;
    }

    if (drge_release_asset(pAsset) > 0) {
        return; // Reference count is still >0. Just return early.
    }

    switch (pAsset->type)
    {
    case drge_asset_type_image: drge__unload_image_asset(pAsset);
    default: break;
    }
}


void drge_grab_asset(drge_asset* pAsset)
{
    if (pAsset == NULL) {
        pAsset->referenceCount += 1;

        if (pAsset->referenceCount == 1) {
            drge__cache_asset(pAsset);
        }
    }
}

unsigned int drge_release_asset(drge_asset* pAsset)
{
    if (pAsset == NULL) {
        return 0;
    }

    if (pAsset->referenceCount > 0) {
        pAsset->referenceCount -= 1;

        if (pAsset->referenceCount == 0) {
            drge__uncache_asset(pAsset);
        }
    }

    return pAsset->referenceCount;
}


drge_asset* drge_find_existing_asset_by_path(drge_context* pContext, const char* path)
{
    if (pContext == NULL || path == NULL) {
        return NULL;
    }

    // TODO: Implement.
    return NULL;
}


drge_image_asset* drge_to_image_asset(drge_asset* pAsset)
{
    if (pAsset == NULL || pAsset->type != drge_asset_type_image) {
        return NULL;
    }

    return (drge_image_asset*)pAsset;
}

drge_material_asset* drge_to_material_asset(drge_asset* pAsset)
{
    if (pAsset == NULL || pAsset->type != drge_asset_type_material) {
        return NULL;
    }

    return (drge_material_asset*)pAsset;
}

drge_model_asset* drge_to_model_asset(drge_asset* pAsset)
{
    if (pAsset == NULL || pAsset->type != drge_asset_type_model) {
        return NULL;
    }

    return (drge_model_asset*)pAsset;
}