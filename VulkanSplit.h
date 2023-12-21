#ifndef VULKAN_SPLIT_INCLUDE_H
#define VULKAN_SPLIT_INCLUDE_H
#include "Pipeline.h"
#include "vulkanFrame.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#define COLOR_FORMAT VK_FORMAT_R32G32B32A32_SFLOAT
//如果val比alignment小，则返回alignment，否则如果val大于alignment但小于alignment*2则返回alignment*2以此类推
#define ALIGN(val, alignment)((val + alignment - 1) & ~(alignment - 1))
#define ROW_COLUMN_INDEX(ROW_INDEX, COLUMN_INDEX, COLUMN)((ROW_INDEX) * (COLUMN) + (COLUMN_INDEX))
// #define OFFSCREEN_DEBUG
struct BaseGraphic{
    VulkanBuffer index;
    VulkanBuffer vertex;
    uint32_t indexCount;
    uint32_t vertexCount;
};
struct Vertex {
    glm::vec3 mPos;
    glm::vec2 mUv;
    Vertex(const glm::vec3&pos, const glm::vec2&uv){
        mPos = pos;
        mUv = uv;
    }
};
struct CARTOONS{
	glm::vec3 degree;
	float cartoons;
};
struct PushConstant{
    glm::mat4 projection;
    //结构体对齐, 对齐, 对齐
    glm::vec3 color;
};
struct Uniform{
    glm::mat4 model;
    //结构体对齐, 对齐, 对齐
	CARTOONS cartoons;
    float imageIndex;
    float useImageArray;
};
class VulkanSplit{
    struct{
#ifdef OFFSCREEN_DEBUG
        GraphicsPipeline debug;
#endif
        GraphicsPipeline texture;
        GraphicsPipeline background;
        GraphicsPipeline offscreenTexture;
        GraphicsPipeline offscreenBackground;
    }pipelines;
    struct{
#ifdef OFFSCREEN_DEBUG
        VulkanBuffer debug;
#endif
        VulkanBuffer position;
        VulkanBuffer background;
        VulkanBuffer offscreenPosition;
        VulkanBuffer offscreenBackground;
    }uniform;
    struct{
        VkDescriptorSet set;
#ifdef OFFSCREEN_DEBUG
        VkDescriptorSet debug;
#endif
        VkDescriptorSet background;
        VkDescriptorSet offscreenPosition;
        VkDescriptorSet offscreenBackground;
    }descriptorset;
    struct{
        VulkanImage color;
        VkSemaphore semaphore;
        uint32_t width, height;
        VkRenderPass renderPass;
        VkCommandBuffer command;
        VkFramebuffer frameBuffer;
    }offscreenPass;
    BaseGraphic mRect;
    VulkanImage mTexture;
    VulkanImage mIncrease;
    VkSampler mTextureSampler;
    VkPipelineCache mPipelineCache;
    VkDescriptorSetLayout mSetLayout;
    void SetupDescriptorSetLayout(VkDevice device);
    VkResult PrepareOffscreenRenderpass(VkDevice device);
    void DrawGraphics(VkCommandBuffer command, const BaseGraphic *graphic);
    void CreateRectResource(VkDevice device, VkQueue graphics, VkCommandPool pool);
    void DrawImage(VkCommandBuffer command, const glm::mat4&projection, VkDescriptorSet set, const GraphicsPipeline &texture);
    void DrawBackground(VkCommandBuffer command, const glm::mat4&projection, VkDescriptorSet set, const glm::vec3&color, const GraphicsPipeline&background);
protected:
    uint32_t mImageCount;
    int32_t mMinUniformBufferOffset;
    // std::vector<uint32_t>mImageIndex;
public:
    VulkanSplit(/* args */);
    ~VulkanSplit();
    // inline void ResetImageIndex(uint32_t imageCount){
    //     // mImageIndex.resize(imageCount);
    //     // for (size_t i = 0; i < imageCount; ++i){
    //     //     mImageIndex[i] = i;
    //     // } 
    // }
    inline VkImage GetOffscreenImage(){
        return offscreenPass.color.image;
    }
    void Cleanup(VkDevice device);
    void RecreateImageUniform(VkDevice device, uint32_t imageCount);
    VkResult ReprepareOffscreenFramebuffer(VkDevice device, const VkExtent2D&backgroundSize);
    void Setup(VkPhysicalDevice physicalDevice, VkDevice device, VkQueue graphics, const VulkanPool&pool, uint32_t imageCount);

    void Draw(VkCommandBuffer command, uint32_t windowWidth, uint32_t windowHeight, const glm::vec3&backgroundColor);
    
    void RerodOffscreenCommand(const glm::vec3&backgroundColor);

    void DestroyGraphicsPipeline(VkDevice device);
    void CreateGraphicsPipeline(VkDevice device, VkRenderPass renderpass, uint32_t scissorWidth, uint32_t scissorHeight);

    void UpdateTexture(VkDevice device, uint32_t index, const Uniform&ubo);
    void UpdateOffscreenTexture(VkDevice device, uint32_t index, const Uniform&ubo);

    void UpdateOffscreenBackground(VkDevice device);
    void UpdateBackground(VkDevice device, const glm::vec2&pos, const VkExtent2D&size);
    // void UpdateTexture(VkDevice device, uint32_t index, const glm::vec2&pos, const VkExtent2D&grid);
    // void UpdateOffscreenTexture(VkDevice device, uint32_t index, const glm::vec2&pos, const VkExtent2D&grid);

    void UpdateDescriptorSet(VkDevice device);

    void ChangeIncreaseImage(VkDevice device, const void *datas, uint32_t width, uint32_t height, VkQueue graphics, VkCommandPool pool);
    void ChangeTextureImage(VkDevice device, const void **datas, uint32_t imageCount, uint32_t width, uint32_t height, VkQueue graphics, VkCommandPool pool);
#ifdef OFFSCREEN_DEBUG
    void DrawDebug(VkCommandBuffer command, uint32_t windowWidth, uint32_t windowHeight);
#endif
    void DrawFrame(VkDevice device, uint32_t currentFrame, const VkCommandBuffer& commandbuffers, VkSwapchainKHR swapchain, const VulkanQueue&vulkanQueue, const VulkanSynchronize&vulkanSynchronize, void(*recreateSwapchain)(void* userData) = nullptr, void* userData = nullptr);
};
#endif