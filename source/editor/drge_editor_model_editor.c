// Public domain. See "unlicense" statement at the end of dr_ge.h.

typedef struct
{
    // A pointer to the asset.
    drge_model_asset* pModelAsset;

    // The graphics world we use to draw the model.
    drge_graphics_world* pGraphicsWorld;

    // The texture we'll be using as the render target.
    drge_graphics_resource* pTextureRT;

    // The GUI image object that the GUI uses to draw the 3D viewport. This is updated any time the 3D viewport changes.
    drgui_image* pViewportImage;

} drge_model_subeditor_data;

static void drge_model_editor__resize_viewport_images(drgui_element* pModelEditor, float newWidth, float newHeight)
{
    drge_model_subeditor_data* pMEData = drge_subeditor_get_extra_data(pModelEditor);
    assert(pMEData != NULL);


    // The render target image is immutable which means we need to delete and recreate.
    if (pMEData->pTextureRT != NULL) {
        drge_graphics_world_delete_resource(pMEData->pTextureRT);
    }
    
    drge_graphics_texture_info textureRTInfo;
    textureRTInfo.width         = (uint32_t)newWidth;
    textureRTInfo.height        = (uint32_t)newHeight;
    textureRTInfo.depth         = 1;
    textureRTInfo.format        = VK_FORMAT_R8G8B8A8_UNORM;
    textureRTInfo.samplesMSAA   = 1;
    textureRTInfo.usage         = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    textureRTInfo.type          = VK_IMAGE_VIEW_TYPE_2D;
    textureRTInfo.useMipmapping = false;
    textureRTInfo.pData         = NULL;
    pMEData->pTextureRT = drge_graphics_world_create_texture_resource(pMEData->pGraphicsWorld, &textureRTInfo);


    // The viewport image is immutable which means we need to delete and recreate.
    if (pMEData->pViewportImage != NULL) {
        drgui_delete_image(pMEData->pViewportImage);
    }
    
    pMEData->pViewportImage = drgui_create_image(ak_get_application_gui(drge_subeditor_get_editor(pModelEditor)->pAKApp), (unsigned int)newWidth, (unsigned int)newHeight, (unsigned int)newWidth*4, NULL);
}

static void drge_model_editor__redraw_world(drgui_element* pModelEditor)
{
    drge_model_subeditor_data* pMEData = drge_subeditor_get_extra_data(pModelEditor);
    assert(pMEData != NULL);

    //drge_graphics_world_draw(pMEData->pGraphicsWorld);

    // At this point the render target should contain the image data to show on the viewport. Here is where we grab the image data
    // of the render target and update the GUI viewport image.
    if (pMEData->pViewportImage != NULL)
    {
        void* pImageData = drgui_map_image_data(pMEData->pViewportImage, DRGUI_WRITE);
        if (pImageData != NULL)
        {
            drge_graphics_world_get_texture_data(pMEData->pTextureRT, pImageData);
            drgui_unmap_image_data(pMEData->pViewportImage);
        }
    }
}

static void drge_model_editor__on_paint(drgui_element* pModelEditor, drgui_rect relativeRect, void* pPaintData)
{
    drge_model_subeditor_data* pMEData = drge_subeditor_get_extra_data(pModelEditor);
    assert(pMEData != NULL);

    unsigned int viewportWidth;
    unsigned int viewportHeight;
    drgui_get_image_size(pMEData->pViewportImage, &viewportWidth, &viewportHeight);

    if (pMEData->pViewportImage != NULL){
        drgui_draw_image_args args;
        args.dstWidth        = (float)viewportWidth;
        args.dstHeight       = (float)viewportHeight;
        args.srcX            = 0;
        args.srcY            = 0;
        args.srcWidth        = (float)viewportWidth;
        args.srcHeight       = (float)viewportHeight;
        args.dstBoundsX      = 0;
        args.dstBoundsY      = 0;
        args.dstBoundsWidth  = drgui_get_width(pModelEditor);
        args.dstBoundsHeight = drgui_get_height(pModelEditor);
        args.foregroundTint  = drgui_rgb(255, 255, 255);
        args.backgroundColor = drgui_rgb(48, 48, 48);
        args.boundsColor     = drgui_rgb(48, 48, 48);
        args.options         = DRGUI_IMAGE_ALIGN_CENTER | DRGUI_IMAGE_DRAW_BACKGROUND | DRGUI_IMAGE_DRAW_BOUNDS | DRGUI_IMAGE_HINT_NO_ALPHA;
        drgui_draw_image(pModelEditor, pMEData->pViewportImage, &args, pPaintData);
    } else {
        drgui_draw_rect(pModelEditor, drgui_get_local_rect(pModelEditor), drgui_rgb(48, 48, 48), pPaintData);
    }
}

static void drge_model_editor__on_size(drgui_element* pModelEditor, float newWidth, float newHeight)
{
    drge_model_subeditor_data* pMEData = drge_subeditor_get_extra_data(pModelEditor);
    assert(pMEData != NULL);

    // The viewport image needs to be resized.
    drge_model_editor__resize_viewport_images(pModelEditor, newWidth, newHeight);

    // After resizing the viewport image we need to re-draw the world.
    drge_model_editor__redraw_world(pModelEditor);
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

    pMEData->pTextureRT = NULL;

    drgui_set_on_size(pModelEditor, drge_model_editor__on_size);
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