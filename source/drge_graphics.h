// Public domain. See "unlicense" statement at the end of dr_ge.h.

typedef enum
{
    drge_graphics_resource_type_texture,
    drge_graphics_resource_type_material,
    drge_graphics_resource_type_mesh
} drge_graphics_resource_type;

typedef enum
{
    drge_graphics_object_type_mesh
} drge_graphics_object_type;

typedef struct drge_graphics_world drge_graphics_world;
typedef struct drge_graphics_resource drge_graphics_resource;
typedef struct drge_graphics_texture_resource drge_graphics_texture_resource;
typedef struct drge_graphics_material_resource drge_graphics_material_resource;
typedef struct drge_graphics_mesh_resource drge_graphics_mesh_resource;

typedef struct
{
    // The width of the texture.
    uint32_t width;
    
    // The height of the texture. Set to 1 for 1D textures.
    uint32_t height;

    // The depth of the texture. Set to 1 for 1D or 2D textures.
    uint32_t depth;

    // The format of the image data.
    //
    // For standard R8B8G8A8 formats, you'll typically want to use VK_FORMAT_R8G8B8A8_UNORM.
    VkFormat format;
    
    // The number of samples to use with this texture for multisampled anti-aliasing. Set to 0 or 1 to not use multisampling. If multisampling is requested,
    // but the underlying rendering API does not support it, texture creation will fail.
    VkSampleCountFlags samplesMSAA;

    // The texture usage flags.
    //   - Use VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT if the texture will be used as a render target.
    //   - Use VK_IMAGE_USAGE_TRANSFER_SRC_BIT if the contents of the texture will need to be retrieved by the host.
    //   - Use VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT if the image will be used in a material or shader.
    //
    // VK_IMAGE_USAGE_TRANSFER_DST_BIT and VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT is set implicitly if pData is not null. VK_IMAGE_USAGE_SAMPLED_BIT is set
    // implicitly if samplesMSAA not 0 or 1.
    VkImageUsageFlags usage;

    // The image type (1D, 2D, Cube, etc.)
    //
    // When this is set to VK_IMAGE_VIEW_TYPE_CUBE and pData is not null, pData should contain the image data for each face one after the other as one big chunk.
    VkImageViewType type;

    // Whether or not mipmapping should be used with the texture.
    bool useMipmapping;

    // A pointer to the raw texture data. This must be set to NULL if the texture is being used as a render target (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT is set).
    const void* pData;

} drge_graphics_texture_info;

typedef struct
{
    int unused;
} drge_graphics_material_info;

typedef struct
{
    int unused;
} drge_graphics_mesh_info;


// Base structure representing an asset.
struct drge_graphics_resource
{
#define DRGE_BASE_GRAPHICS_RESOURCE_ATTRIBS \
    drge_graphics_resource_type type; \
    drge_graphics_world* pWorld;

    DRGE_BASE_GRAPHICS_RESOURCE_ATTRIBS
};

struct drge_graphics_texture_resource
{
    DRGE_BASE_GRAPHICS_RESOURCE_ATTRIBS

    // The Vulkan image object.
    VkImage image;

    // The backing memory of the image object.
    VkDeviceMemory imageMemory;

    // The image view.
    VkImageView imageView;


    // The information that was used to create the texture. This is mainly for informational purposes.
    drge_graphics_texture_info info;


    // TEMP. This section contains the proxy image that's used to pass the image data back to the host. This is temporary and
    // will be replaced with a much more memory-efficient implementation.
    VkImage proxyImage;
    VkDeviceMemory proxyImageMemory;
};

struct drge_graphics_material_resource
{
    DRGE_BASE_GRAPHICS_RESOURCE_ATTRIBS
};

struct drge_graphics_mesh_resource
{
    DRGE_BASE_GRAPHICS_RESOURCE_ATTRIBS
};


struct drge_graphics_world
{
    // The Vulkan context.
    drvk_context* pVulkan;

    // The index of the primary device.
    uint32_t primaryDeviceIndex;

    // The primary device that most graphics operations will go through.
    VkDevice primaryDevice;

    // The queue we'll use for all of our resource management. Any time we create an image, copy image data, etc., it'll go through here.
    drvk_queue* pResourceQueue;

    // The command buffer we'll use for resource management.
    VkCommandBuffer resourceCmdBuffer;



    // TEMP. Everything below is just for testing and getting the basics working.
    drvk_queue* pGraphicsQueue;

};



// Creates a graphics world.
drge_graphics_world* drge_create_graphics_world(drvk_context* pVulkan);

// Deletes a graphics world.
void drge_delete_graphics_world(drge_graphics_world* pWorld);


//// Resources ////

// Creates a texture resource.
drge_graphics_resource* drge_graphics_world_create_texture_resource(drge_graphics_world* pWorld, const drge_graphics_texture_info* pInfo);

// Deletes a texture resource.
void drge_graphics_world_delete_texture_resource(drge_graphics_resource* pResource);

// Retrieves the data of the given texture resource.
//
// This is not a simple accessor - it will retrieve a copy of the image data from the GPU. Avoid calling this while in the middle
// of rendering a frame.
//
// Returns the size in bytes of the copied data. A return value of zero indicates an error.
size_t drge_graphics_world_get_texture_data(drge_graphics_resource* pResource, void* pDataOut);


// Creates a material resource.
drge_graphics_resource* drge_graphics_world_create_material_resource(drge_graphics_world* pWorld, const drge_graphics_material_info* pInfo);

// Deletes a material resource.
void drge_graphics_world_delete_material_resource(drge_graphics_resource* pResource);


// Creates a mesh resource.
drge_graphics_resource* drge_graphics_world_create_mesh_resource(drge_graphics_world* pWorld, const drge_graphics_mesh_info* pInfo);

// Deletes a mesh resource.
void drge_graphics_world_delete_mesh_resource(drge_graphics_resource* pResource);


// Deletes the given resource.
//
// The resource can be of any type.
void drge_graphics_world_delete_resource(drge_graphics_resource* pResource);



// Dynamically casts the given graphics resource to a texture resource.
//
// Returns NULL if the resource is not a texture resource.
drge_graphics_texture_resource* drge_to_graphics_texture_resource(drge_graphics_resource* pResource);

// Dynamically casts the given graphics resource to a material resource.
//
// Returns NULL if the resource is not a material resource.
drge_graphics_material_resource* drge_to_graphics_material_resource(drge_graphics_resource* pResource);

// Dynamically casts the given graphics resource to a mesh resource.
//
// Returns NULL if the resource is not a mesh resource.
drge_graphics_mesh_resource* drge_to_graphics_mesh_resource(drge_graphics_resource* pResource);



//// Objects ////




//// Render Targets ////

