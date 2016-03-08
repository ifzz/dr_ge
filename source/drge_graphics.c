// Public domain. See "unlicense" statement at the end of dr_ge.h.

drge_graphics_world* drge_create_graphics_world(drvk_context* pVulkan)
{
    if (pVulkan == NULL) {
        return NULL;
    }

    drge_graphics_world* pWorld = malloc(sizeof(*pWorld));
    if (pWorld == NULL) {
        return NULL;
    }

    pWorld->pVulkan            = pVulkan;
    pWorld->primaryDeviceIndex = 0;
    pWorld->primaryDevice      = drvkGetDevice(pVulkan, pWorld->primaryDeviceIndex);

    drvk_queue* pQueues[2];
    VkResult result = drvkFindQueuesWithFlags(pWorld->pVulkan, pWorld->primaryDeviceIndex, VK_QUEUE_GRAPHICS_BIT, 2, pQueues);
    if (result != VK_SUCCESS) {
        free(pWorld);
        return NULL;
    }
    
    pWorld->pResourceQueue = pQueues[0];


    VkCommandBufferAllocateInfo bufferAllocInfo;
    bufferAllocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    bufferAllocInfo.pNext              = NULL;
    bufferAllocInfo.commandBufferCount = 1;
    bufferAllocInfo.commandPool        = pWorld->pResourceQueue->commandPool;
    bufferAllocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    result = vkAllocateCommandBuffers(drvkGetDevice(pVulkan, 0), &bufferAllocInfo, &pWorld->resourceCmdBuffer);
    if (result != VK_SUCCESS) {
        free(pWorld);
        return NULL;
    }

    



    // Everything below is temp.
    //pWorld->pGraphicsQueue = drvkFindFirstQueueWithFlags(pVulkan, 0, VK_QUEUE_GRAPHICS_BIT);
    pWorld->pGraphicsQueue = pQueues[1];

    // Create the image we'll draw to.
    VkImageCreateInfo imageInfo;
    imageInfo.sType                 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.pNext                 = NULL;
    imageInfo.flags                 = 0;
    imageInfo.imageType             = VK_IMAGE_TYPE_2D;
    imageInfo.format                = VK_FORMAT_R8G8B8A8_UNORM;
    imageInfo.extent.width          = 1280;
    imageInfo.extent.height         = 720;
    imageInfo.extent.depth          = 1;
    imageInfo.mipLevels             = 1;
    imageInfo.arrayLayers           = 1;
    imageInfo.samples               = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling                = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage                 = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    imageInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.queueFamilyIndexCount = 0;
    imageInfo.pQueueFamilyIndices   = NULL;
    imageInfo.initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED;

    VkImage image;
    result = vkCreateImage(drvkGetDevice(pVulkan, 0), &imageInfo, NULL, &image);
    if (result != VK_SUCCESS) {
        return NULL;
    }


    // Allocate and bind the image's backing memory.
    VkMemoryRequirements imageMemoryReqs;
    vkGetImageMemoryRequirements(drvkGetDevice(pVulkan, 0), image, &imageMemoryReqs);
    
    VkMemoryAllocateInfo memoryAllocInfo;
    memoryAllocInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocInfo.pNext           = NULL;
    memoryAllocInfo.allocationSize  = imageMemoryReqs.size;
    memoryAllocInfo.memoryTypeIndex = drvkGetMemoryTypeIndex(pVulkan, 0, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkDeviceMemory imageMemory;
    result = vkAllocateMemory(drvkGetDevice(pVulkan, 0), &memoryAllocInfo, NULL, &imageMemory);
    if (result != VK_SUCCESS) {
        return NULL;
    }

    result = vkBindImageMemory(drvkGetDevice(pVulkan, 0), image, imageMemory, 0);
    if (result != VK_SUCCESS) {
        return NULL;
    }


    // Create the image view so we can 
    VkImageViewCreateInfo imageViewInfo;
    imageViewInfo.sType      = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewInfo.pNext      = NULL;
    imageViewInfo.flags      = 0;
    imageViewInfo.image      = image;
    imageViewInfo.viewType   = VK_IMAGE_VIEW_TYPE_2D;
    imageViewInfo.format     = VK_FORMAT_R8G8B8A8_UNORM;
    imageViewInfo.components = (VkComponentMapping){ VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
    imageViewInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    imageViewInfo.subresourceRange.baseMipLevel   = 0;
    imageViewInfo.subresourceRange.levelCount     = 1;
    imageViewInfo.subresourceRange.baseArrayLayer = 0;
    imageViewInfo.subresourceRange.layerCount     = 1;

    VkImageView imageView;
    result = vkCreateImageView(drvkGetDevice(pVulkan, 0), &imageViewInfo, NULL, &imageView);
    if (result != VK_SUCCESS) {
        return NULL;
    }


    
    VkCommandBuffer setupCmdBuffer[2];
    bufferAllocInfo.commandBufferCount = 2;
    bufferAllocInfo.commandPool        = pWorld->pGraphicsQueue->commandPool;
    bufferAllocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    result = vkAllocateCommandBuffers(drvkGetDevice(pVulkan, 0), &bufferAllocInfo, setupCmdBuffer);
    if (result != VK_SUCCESS) {
        return NULL;
    }

    VkCommandBufferBeginInfo beginInfo;
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pNext = NULL;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    beginInfo.pInheritanceInfo = NULL;
    result = vkBeginCommandBuffer(setupCmdBuffer[0], &beginInfo);
    if (result != VK_SUCCESS) {
        return NULL;
    }

    {
        VkImageMemoryBarrier barrier;
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.pNext = NULL;
        barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        vkCmdPipelineBarrier(setupCmdBuffer[0], VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);

        VkImageSubresourceRange subresource;
        subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subresource.baseMipLevel = 0;
        subresource.levelCount = 1;
        subresource.baseArrayLayer = 0;
        subresource.layerCount = 1;

        VkClearColorValue color;
        color.float32[0] = 0.5f;
        color.float32[1] = 0.0f;
        color.float32[2] = 0.0f;
        color.float32[3] = 1.0f;
        vkCmdClearColorImage(setupCmdBuffer[0], image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &color, 1, &subresource);

        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        vkCmdPipelineBarrier(setupCmdBuffer[0], VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);
    }

    result = vkEndCommandBuffer(setupCmdBuffer[0]);
    if (result != VK_SUCCESS) {
        return NULL;
    }


    VkSemaphore imageWrittenSemaphore;
    result = drvkCreateSemaphore(drvkGetDevice(pVulkan, 0), &imageWrittenSemaphore);
    if (result != VK_SUCCESS) {
        return NULL;
    }

    VkSubmitInfo submitInfo;
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = NULL;
    submitInfo.waitSemaphoreCount = 0;
    submitInfo.pWaitSemaphores = NULL;
    submitInfo.pWaitDstStageMask = NULL;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &setupCmdBuffer[0];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &imageWrittenSemaphore;
    result = vkQueueSubmit(pWorld->pGraphicsQueue->vkQueue, 1, &submitInfo, VK_NULL_HANDLE);
    if (result != VK_SUCCESS) {
        return NULL;
    }


    // At this point the render target should have some content inside it so we now want to read it back to the host. To do this we
    // use a proxy image that uses linear tiling.
    imageInfo.tiling = VK_IMAGE_TILING_LINEAR;
    imageInfo.usage  = VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    VkImage proxyImage;
    result = vkCreateImage(drvkGetDevice(pVulkan, 0), &imageInfo, NULL, &proxyImage);
    if (result != VK_SUCCESS) {
        return NULL;
    }

    VkDeviceMemory proxyImageMemory;
    result = drvkAllocateImageMemory(pVulkan, 0, proxyImage, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &proxyImageMemory);
    if (result != VK_SUCCESS) {
        return NULL;
    }

    result = vkBindImageMemory(drvkGetDevice(pVulkan, 0), proxyImage, proxyImageMemory, 0);
    if (result != VK_SUCCESS) {
        return NULL;
    }


    result = vkBeginCommandBuffer(setupCmdBuffer[1], &beginInfo);
    if (result != VK_SUCCESS) {
        return NULL;
    }

    {
        VkImageMemoryBarrier barrier;
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.pNext = NULL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.image = proxyImage;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        vkCmdPipelineBarrier(setupCmdBuffer[1], VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);

        VkImageCopy copyRegion;
        copyRegion.srcSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        copyRegion.srcSubresource.mipLevel       = 0;
        copyRegion.srcSubresource.baseArrayLayer = 0;
        copyRegion.srcSubresource.layerCount     = 1;
        copyRegion.srcOffset.x = 0;
        copyRegion.srcOffset.y = 0;
        copyRegion.srcOffset.z = 0;
        copyRegion.dstSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        copyRegion.dstSubresource.mipLevel       = 0;
        copyRegion.dstSubresource.baseArrayLayer = 0;
        copyRegion.dstSubresource.layerCount     = 1;
        copyRegion.dstOffset.x = 0;
        copyRegion.dstOffset.y = 0;
        copyRegion.dstOffset.z = 0;
        copyRegion.extent.width  = 1280;
        copyRegion.extent.height = 720;
        copyRegion.extent.depth  = 1;
        vkCmdCopyImage(setupCmdBuffer[1], image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, proxyImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

        barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
        vkCmdPipelineBarrier(setupCmdBuffer[1], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);
    }

    result = vkEndCommandBuffer(setupCmdBuffer[1]);
    if (result != VK_SUCCESS) {
        return NULL;
    }


    VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &imageWrittenSemaphore;
    submitInfo.pWaitDstStageMask = &waitDstStageMask;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &setupCmdBuffer[1];
    submitInfo.signalSemaphoreCount = 0;
    submitInfo.pSignalSemaphores = NULL;
    result = vkQueueSubmit(pWorld->pGraphicsQueue->vkQueue, 1, &submitInfo, VK_NULL_HANDLE);
    if (result != VK_SUCCESS) {
        return NULL;
    }

    result = vkQueueWaitIdle(pWorld->pGraphicsQueue->vkQueue);
    if (result != VK_SUCCESS) {
        return NULL;
    }


    VkMappedMemoryRange memoryRange;
    memoryRange.sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    memoryRange.pNext  = NULL;
    memoryRange.memory = proxyImageMemory;
    memoryRange.offset = 0;
    memoryRange.size   = VK_WHOLE_SIZE;
    result = vkInvalidateMappedMemoryRanges(drvkGetDevice(pVulkan, 0), 1, &memoryRange);
    if (result != VK_SUCCESS) {
        return NULL;
    }


    // Test if everything worked...
    VkImageSubresource imageMipmap;
    imageMipmap.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageMipmap.mipLevel   = 0;
    imageMipmap.arrayLayer = 0;

    VkSubresourceLayout imageMipmapLayout;
    vkGetImageSubresourceLayout(drvkGetDevice(pVulkan, 0), proxyImage, &imageMipmap, &imageMipmapLayout);

    void* pImageData = NULL;
    result = vkMapMemory(drvkGetDevice(pVulkan, 0), proxyImageMemory, 0, VK_WHOLE_SIZE, 0, &pImageData);
    if (result != VK_SUCCESS) {
        return NULL;
    }

    vkUnmapMemory(drvkGetDevice(pVulkan, 0), proxyImageMemory);


    return pWorld;
}

void drge_delete_graphics_world(drge_graphics_world* pWorld)
{
    free(pWorld);
}



//// Resources ////

drge_graphics_resource* drge_graphics_world_create_texture_resource(drge_graphics_world* pWorld, const drge_graphics_texture_info* pInfo)
{
    if (pWorld == NULL || pInfo == NULL) {
        return NULL;
    }

    VkDevice device = drvkGetDevice(pWorld->pVulkan, 0);
    assert(device != NULL);

    VkImage image = NULL;
    VkDeviceMemory imageMemory = NULL;
    VkImageView imageView = NULL;

    static VkImageType imageViewType2ImageTypeMap[] = {
        VK_IMAGE_TYPE_1D,   // VK_IMAGE_VIEW_TYPE_1D
        VK_IMAGE_TYPE_2D,   // VK_IMAGE_VIEW_TYPE_2D
        VK_IMAGE_TYPE_3D,   // VK_IMAGE_VIEW_TYPE_3D
        VK_IMAGE_TYPE_2D,   // VK_IMAGE_VIEW_TYPE_CUBE
        VK_IMAGE_TYPE_1D,   // VK_IMAGE_VIEW_TYPE_1D_ARRAY
        VK_IMAGE_TYPE_2D,   // VK_IMAGE_VIEW_TYPE_2D_ARRAY
        VK_IMAGE_TYPE_2D,   // VK_IMAGE_VIEW_TYPE_CUBE_ARRAY
    };

    VkImageCreateInfo imageInfo;
    imageInfo.sType                 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.pNext                 = NULL;
    imageInfo.flags                 = 0;
    imageInfo.imageType             = imageViewType2ImageTypeMap[pInfo->type];
    imageInfo.format                = pInfo->format;
    imageInfo.extent.width          = pInfo->width;
    imageInfo.extent.height         = pInfo->height;
    imageInfo.extent.depth          = pInfo->depth;
    imageInfo.mipLevels             = 1;
    imageInfo.arrayLayers           = 1;
    imageInfo.samples               = pInfo->samplesMSAA;
    imageInfo.tiling                = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage                 = pInfo->usage;
    imageInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.queueFamilyIndexCount = 0;
    imageInfo.pQueueFamilyIndices   = NULL;
    imageInfo.initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED;

    if (pInfo->useMipmapping) {
        imageInfo.mipLevels = (uint32_t)floor(log2(dr_max(dr_max(pInfo->width, pInfo->height), pInfo->depth))) + 1;
    }

    if (pInfo->pData != NULL) {
        imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    }
    if (pInfo->samplesMSAA > 1) {
        imageInfo.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
    }

    if (pInfo->type == VK_IMAGE_VIEW_TYPE_CUBE) {
        imageInfo.arrayLayers = 6;
    }


    VkResult result = vkCreateImage(device, &imageInfo, NULL, &image);
    if (result != VK_SUCCESS) {
        goto on_error;
    }

    result = drvkAllocateImageMemory(pWorld->pVulkan, pWorld->primaryDeviceIndex, image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &imageMemory);
    if (result != VK_SUCCESS) {
        goto on_error;
    }

    result = vkBindImageMemory(pWorld->primaryDevice, image, imageMemory, 0);
    if (result != VK_SUCCESS) {
        goto on_error;
    }


    // Copy image data if applicable.
    if (pInfo->pData != NULL)
    {
        // TODO: Implement Me.
    }


    VkImageViewCreateInfo imageViewInfo;
    imageViewInfo.sType      = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewInfo.pNext      = NULL;
    imageViewInfo.flags      = 0;
    imageViewInfo.image      = image;
    imageViewInfo.viewType   = pInfo->type;
    imageViewInfo.format     = pInfo->format;
    imageViewInfo.components = (VkComponentMapping){ VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
    imageViewInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    imageViewInfo.subresourceRange.baseMipLevel   = 0;
    imageViewInfo.subresourceRange.levelCount     = imageInfo.mipLevels;
    imageViewInfo.subresourceRange.baseArrayLayer = 0;
    imageViewInfo.subresourceRange.layerCount     = 1;

    result = vkCreateImageView(pWorld->primaryDevice, &imageViewInfo, NULL, &imageView);
    if (result != VK_SUCCESS) {
        goto on_error;
    }


    // Everything should be good at this point.
    drge_graphics_texture_resource* pTextureResource = malloc(sizeof(*pTextureResource));
    if (pTextureResource == NULL) {
        goto on_error;
    }

    pTextureResource->type        = drge_graphics_resource_type_texture;
    pTextureResource->pWorld      = pWorld;
    pTextureResource->image       = image;
    pTextureResource->imageMemory = imageMemory;
    pTextureResource->imageView   = imageView;
    pTextureResource->info        = *pInfo;
    pTextureResource->info.pData  = NULL;    // <-- Set this to null for safety since it's likely the caller will free the memory later.

    return (drge_graphics_resource*)pTextureResource;

on_error:
    if (imageView) {
        vkDestroyImageView(pWorld->primaryDevice, imageView, NULL);
    }
    if (image) {
        vkDestroyImage(pWorld->primaryDevice, image, NULL);
    }
    if (imageMemory) {
        vkFreeMemory(pWorld->primaryDevice, imageMemory, NULL);
    }

    return NULL;
}

void drge_graphics_world_delete_texture_resource(drge_graphics_resource* pResource)
{
    drge_graphics_texture_resource* pTextureResource = drge_to_graphics_texture_resource(pResource);
    if (pTextureResource == NULL) {
        return;
    }


    vkDestroyImageView(pResource->pWorld->primaryDevice, pTextureResource->imageView, NULL);
    vkDestroyImage(pResource->pWorld->primaryDevice, pTextureResource->image, NULL);
    vkFreeMemory(pResource->pWorld->primaryDevice, pTextureResource->imageMemory, NULL);

    free(pTextureResource);
}

size_t drge_graphics_world_get_texture_data(drge_graphics_resource* pResource, void* pDataOut)
{
    drge_graphics_texture_resource* pTextureResource = drge_to_graphics_texture_resource(pResource);
    if (pTextureResource == NULL) {
        return 0;
    }

    // The texture must have been created with VK_IMAGE_USAGE_TRANSFER_SRC_BIT.
    if ((pTextureResource->info.usage & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) == 0) {
        return 0;
    }

    VkMemoryRequirements memReqs;
    vkGetImageMemoryRequirements(drvkGetDevice(pResource->pWorld->pVulkan, 0), pTextureResource->image, &memReqs);

    if (pDataOut)
    {
        
    }

    return memReqs.size;
}


drge_graphics_resource* drge_graphics_world_create_material_resource(drge_graphics_world* pWorld, const drge_graphics_material_info* pInfo)
{
    if (pWorld == NULL || pInfo == NULL) {
        return NULL;
    }

    // TODO: Implement Me.
    return NULL;
}

void drge_graphics_world_delete_material_resource(drge_graphics_resource* pResource)
{
    drge_graphics_material_resource* pMAterialResource = drge_to_graphics_material_resource(pResource);
    if (pMAterialResource == NULL) {
        return;
    }

    // TODO: Implement Me.
}


drge_graphics_resource* drge_graphics_world_create_mesh_resource(drge_graphics_world* pWorld, const drge_graphics_mesh_info* pInfo)
{
    if (pWorld == NULL || pInfo == NULL) {
        return NULL;
    }

    // TODO: Implement Me.
    return NULL;
}

void drge_graphics_world_delete_mesh_resource(drge_graphics_resource* pResource)
{
    drge_graphics_mesh_resource* pMeshResource = drge_to_graphics_mesh_resource(pResource);
    if (pMeshResource == NULL) {
        return;
    }

    // TODO: Implement Me.
}


void drge_graphics_world_delete_resource(drge_graphics_resource* pResource)
{
    switch (pResource->type)
    {
        case drge_graphics_resource_type_texture:
        {
            drge_graphics_world_delete_texture_resource(pResource);
        } break;

        case drge_graphics_resource_type_material:
        {
            drge_graphics_world_delete_material_resource(pResource);
        } break;
        
        case drge_graphics_resource_type_mesh:
        {
            drge_graphics_world_delete_mesh_resource(pResource);
        } break;

        default: break;
    }
}


drge_graphics_texture_resource* drge_to_graphics_texture_resource(drge_graphics_resource* pResource)
{
    if (pResource == NULL || pResource->type != drge_graphics_resource_type_texture) {
        return NULL;
    }

    return (drge_graphics_texture_resource*)pResource;
}

drge_graphics_material_resource* drge_to_graphics_material_resource(drge_graphics_resource* pResource)
{
    if (pResource == NULL || pResource->type != drge_graphics_resource_type_material) {
        return NULL;
    }

    return (drge_graphics_material_resource*)pResource;
}

drge_graphics_mesh_resource* drge_to_graphics_mesh_resource(drge_graphics_resource* pResource)
{
    if (pResource == NULL || pResource->type != drge_graphics_resource_type_mesh) {
        return NULL;
    }

    return (drge_graphics_mesh_resource*)pResource;
}
