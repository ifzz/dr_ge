// Public domain. See "unlicense" statement at the end of dr_ge.h.

typedef struct
{
    // A pointer to the image asset.
    drge_image_asset* pImageAsset;

    // The GUI image to draw.
    drgui_image* pGUIImage;

} drge_image_subeditor_data;

void drge_image_editor__on_paint(drgui_element* pImageEditor, drgui_rect relativeRect, void* pPaintData)
{
    drge_image_subeditor_data* pIEData = drge_subeditor_get_extra_data(pImageEditor);
    assert(pIEData != NULL);

    
    if (pIEData->pGUIImage != NULL){
        drgui_draw_image_args args;
        args.dstWidth        = (float)pIEData->pImageAsset->width;      // <-- TODO: Handle zooming.
        args.dstHeight       = (float)pIEData->pImageAsset->height;     // <-- TODO: Handle zooming.
        args.srcX            = 0;
        args.srcY            = 0;
        args.srcWidth        = (float)pIEData->pImageAsset->width;
        args.srcHeight       = (float)pIEData->pImageAsset->height;
        args.dstBoundsX      = 0;
        args.dstBoundsY      = 0;
        args.dstBoundsWidth  = drgui_get_width(pImageEditor);
        args.dstBoundsHeight = drgui_get_height(pImageEditor);
        args.foregroundTint  = drgui_rgb(255, 255, 255);
        args.backgroundColor = drgui_rgb(48, 48, 48);
        args.boundsColor     = drgui_rgb(48, 48, 48);
        args.options         = DRGUI_IMAGE_ALIGN_CENTER | DRGUI_IMAGE_DRAW_BACKGROUND | DRGUI_IMAGE_DRAW_BOUNDS;
        drgui_draw_image(pImageEditor, pIEData->pGUIImage, &args, pPaintData);
    } else {
        drgui_draw_rect(pImageEditor, drgui_get_local_rect(pImageEditor), drgui_rgb(48, 48, 48), pPaintData);
    }
}

drge_subeditor* drge_editor_create_image_editor(drge_editor* pEditor, const char* fileAbsolutePath)
{
    // An image editor is just a sub-editor.
    drge_subeditor* pImageEditor = drge_editor_create_sub_editor(pEditor, DRGE_EDITOR_TOOL_TYPE_IMAGE_EDITOR, fileAbsolutePath, sizeof(drge_image_subeditor_data));
    if (pImageEditor == NULL) {
        return NULL;
    }

    drge_image_subeditor_data* pIEData = drge_subeditor_get_extra_data(pImageEditor);
    assert(pIEData != NULL);
    memset(pIEData, 0, sizeof(*pIEData));

    drge_asset* pAsset = drge_load_asset(pEditor->pContext, fileAbsolutePath);
    if (pAsset != NULL) {
        pIEData->pImageAsset = drge_to_image_asset(pAsset);
        assert(pIEData->pImageAsset != NULL);   // <-- The asset type should have been confirmed to be an image at a higher level.

        // TODO: Might need to convert the data.
        pIEData->pGUIImage = drgui_create_image(ak_get_application_gui(pEditor->pAKApp), pIEData->pImageAsset->width, pIEData->pImageAsset->height, pIEData->pImageAsset->width*4, pIEData->pImageAsset->pImageData);
    }
    

    drgui_set_on_paint(pImageEditor, drge_image_editor__on_paint);

    return pImageEditor;
}

void drge_editor_delete_image_editor(drge_subeditor* pImageEditor)
{
    drge_image_subeditor_data* pIEData = drge_subeditor_get_extra_data(pImageEditor);
    if (pIEData != NULL) {
        drge_unload_asset((drge_asset*)pIEData->pImageAsset);
    }

    drge_editor_delete_sub_editor(pImageEditor);
}