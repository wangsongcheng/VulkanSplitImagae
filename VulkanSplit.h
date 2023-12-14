#ifndef VULKAN_SPLIT_INCLUDE_H
#define VULKAN_SPLIT_INCLUDE_H
#include "Pipeline.h"
#include "vulkanFrame.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
//如果val比alignment小，则返回alignment，否则如果val大于alignment但小于alignment*2则返回alignment*2以此类推
#define ALIGN(val, alignment)((val + alignment - 1) & ~(alignment - 1))
#define ROW_COLUMN_INDEX(ROW_INDEX, COLUMN_INDEX, COLUMN)((ROW_INDEX) * (COLUMN) + (COLUMN_INDEX))
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
struct PushConstant{
    glm::mat4 projection;
    //结构体对齐, 对齐, 对齐
    glm::vec3 color;
};
struct Uniform{
    glm::mat4 model;
    float imageIndex;
};
class VulkanSplit{
    struct{
        GraphicsPipeline pipeline;
        GraphicsPipeline background;
    }pipelines;
    struct{
        VulkanBuffer position;
        VulkanBuffer background;
    }uniform;
    struct{
        VkDescriptorSet set;
        VkDescriptorSet background;
    }descriptorset;
    BaseGraphic mRect;
    VulkanImage mTexture;
    uint32_t mImageCount;
    // VulkanImage mBackground;
    VkSampler mTextureSampler;
    VkDescriptorSetLayout mSetLayout;
    void SetupDescriptorSetLayout(VkDevice device);
    void Draw(VkCommandBuffer command, const BaseGraphic *graphic);
    // void SetupDescriptorSet(VkDevice device, VkDescriptorPool pool);
    void CreateRectResource(VkDevice device, VkQueue graphics, VkCommandPool pool);
protected:
    VkExtent2D mImageSize;
    VkExtent2D mImageRealSize;
    int32_t mMinUniformBufferOffset;
    // std::vector<uint32_t>mImageIndex;
public:
    VulkanSplit(/* args */);
    ~VulkanSplit();
    inline void ResetImageIndex(uint32_t imageCount){
        // mImageIndex.resize(imageCount);
        // for (size_t i = 0; i < imageCount; ++i){
        //     mImageIndex[i] = i;
        // } 
    }
    void Cleanup(VkDevice device);
    void Setup(VkPhysicalDevice physicalDevice, VkDevice device, VkQueue graphics, const VulkanPool&pool, uint32_t imageCount);

    void Draw(VkCommandBuffer command, uint32_t windowWidth, uint32_t windowHeight, const glm::vec3&backgroundColor);

    void DestroyGraphicsPipeline(VkDevice device);
    void CreateGraphicsPipeline(VkDevice device, VkRenderPass renderpass, uint32_t scissorWidth, uint32_t scissorHeight);

    void UpdateTexture(VkDevice device, uint32_t index, const glm::vec3&pos, const glm::vec2&size);
    void UpdateBackground(VkDevice device, uint32_t row, uint32_t column, uint32_t windowWidth, uint32_t windowHeight);

    void UpdateDescriptorSet(VkDevice device);

    void ChangeTextureImage(VkDevice device, const void **datas, uint32_t imageCount, uint32_t width, uint32_t height, VkQueue graphics, VkCommandPool pool);
};
#endif