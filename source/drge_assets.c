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

    if (_stricmp(ext, "wav") == 0 ||
        _stricmp(ext, "ogg") == 0)
    {
        return drge_asset_type_sound;
    }

    if (_stricmp(ext, "txt") == 0)
    {
        return drge_asset_type_text;
    }


    return drge_asset_type_unknown;
}