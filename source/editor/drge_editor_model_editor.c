// Public domain. See "unlicense" statement at the end of dr_ge.h.

typedef struct
{
    // A pointer to the asset.
    drge_model_asset* pModelAsset;

    // The graphics world we use to draw the model.
    drge_graphics_world* pGraphicsWorld;

    // The texture we'll be using as the render target.
    drge_graphics_resource* pTextureRT;

} drge_model_subeditor_data;

void drge_model_editor__on_paint(drgui_element* pModelEditor, drgui_rect relativeRect, void* pPaintData)
{
    drge_model_subeditor_data* pMEData = drge_subeditor_get_extra_data(pModelEditor);
    assert(pMEData != NULL);

    drgui_draw_rect(pModelEditor, drgui_get_local_rect(pModelEditor), drgui_rgb(48, 48, 48), pPaintData);
}

drge_subeditor* drge_editor_create_model_editor(drge_editor* pEditor, const char* fileAbsolutePath)
{
    // An image editor is just a sub-editor.
    drge_subeditor* pModelEditor = drge_editor_create_sub_editor(pEditor, DRGE_EDITOR_TOOL_TYPE_MODEL_EDITOR, fileAbsolutePath, sizeof(drge_model_subeditor_data));
    if (pModelEditor == NULL) {
        return NULL;
    }

    drge_model_subeditor_data* pMEData = drge_subeditor_get_extra_data(pModelEditor);
    assert(pMEData != NULL);
    memset(pMEData, 0, sizeof(*pMEData));

    drge_asset* pAsset = drge_load_asset(pEditor->pContext, fileAbsolutePath);
    if (pAsset == NULL) {
        goto on_error;
    }

    pMEData->pModelAsset = drge_to_model_asset(pAsset);
    assert(pMEData->pModelAsset != NULL);   // <-- The asset type should have been confirmed to be a model at a higher level.

    

    pMEData->pGraphicsWorld = drge_create_graphics_world(pEditor->pContext->pVulkan);
    if (pMEData->pGraphicsWorld == NULL) {
        goto on_error;
    }

    drge_graphics_texture_info textureRTInfo;
    textureRTInfo.width         = 1280;
    textureRTInfo.height        = 720;
    textureRTInfo.depth         = 1;
    textureRTInfo.format        = VK_FORMAT_R8G8B8A8_UNORM;
    textureRTInfo.samplesMSAA   = 1;
    textureRTInfo.usage         = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    textureRTInfo.type          = VK_IMAGE_VIEW_TYPE_2D;
    textureRTInfo.useMipmapping = false;
    textureRTInfo.pData         = NULL;
    pMEData->pTextureRT = drge_graphics_world_create_texture_resource(pMEData->pGraphicsWorld, &textureRTInfo);
    if (pMEData->pTextureRT == NULL) {
        goto on_error;
    }
    

    drgui_set_on_paint(pModelEditor, drge_model_editor__on_paint);

    return pModelEditor;

on_error:
    if (pMEData != NULL) {
        if (pMEData->pGraphicsWorld != NULL) {
            drge_delete_graphics_world(pMEData->pGraphicsWorld);
        }

        drge_editor_delete_sub_editor(pModelEditor);
    }

    if (pAsset != NULL) {
        drge_unload_asset(pAsset);
    }
    
    return NULL;
}

void drge_editor_delete_model_editor(drge_subeditor* pModelEditor)
{
    drge_model_subeditor_data* pMEData = drge_subeditor_get_extra_data(pModelEditor);
    if (pMEData != NULL) {
        drge_unload_asset((drge_asset*)pMEData->pModelAsset);
    }

    drge_editor_delete_sub_editor(pModelEditor);
}