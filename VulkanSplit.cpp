#include "VulkanSplit.h"
void VulkanSplit::Draw(VkCommandBuffer command, const BaseGraphic *graphic){
    VkDeviceSize offset = 0;
    if(graphic){
        vkCmdBindVertexBuffers(command, 0, 1, &graphic->vertex.buffer, &offset);
        if(graphic->indexCount){
            vkCmdBindIndexBuffer(command, graphic->index.buffer, 0, VK_INDEX_TYPE_UINT16);
            vkCmdDrawIndexed(command, graphic->indexCount, 1, 0, 0, 0);
        }
        else{
            vkCmdDrawIndexed(command, graphic->vertexCount, 1, 0, 0, 0);
        }
    }
}
void VulkanSplit::SetupDescriptorSetLayout(VkDevice device){
    // Shared pipeline layout for all pipelines used in this sample
    VkDescriptorSetLayoutBinding setlayoutBindings[2] = {};
    // setlayoutBinding.binding = 0;
    setlayoutBindings[0].descriptorCount = 1;
    setlayoutBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    setlayoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    setlayoutBindings[1].binding = 1;
    setlayoutBindings[1].descriptorCount = 1;
    setlayoutBindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    setlayoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    vkf::CreateDescriptorSetLayout(device, 2, setlayoutBindings, &mSetLayout);
}
// void VulkanSplit::SetupDescriptorSet(VkDevice device, VkDescriptorPool pool){
//     vkf::tool::AllocateDescriptorSets(device, pool, &mSetLayout, 1, &mSet);

// }
void VulkanSplit::CreateRectResource(VkDevice device, VkQueue graphics, VkCommandPool pool){
    const uint16_t indices[] = { 0, 1, 2, 0, 3, 1 };
    const Vertex vertices[] = {
        // 位置     // 纹理
        Vertex(glm::vec3(.0f, 1.0f, .0f), glm::vec2(0.0f, 1.0f)),//左下
        Vertex(glm::vec3(1.0f, .0f, .0f), glm::vec2(1.0f, 0.0f)),//右上
        Vertex(glm::vec3(.0f, .0f, .0f), glm::vec2(0.0f, 0.0f)), //左上

        Vertex(glm::vec3(1.0f, 1.0f, .0f), glm::vec2(1.0f, 1.0f))//右下
    };
    mRect.indexCount = 6;
    mRect.vertexCount = 0;
    mRect.index.CreateBuffer(device, sizeof(indices), VK_BUFFER_USAGE_INDEX_BUFFER_BIT|VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    mRect.vertex.CreateBuffer(device, sizeof(vertices), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT|VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    mRect.index.AllocateAndBindMemory(device, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    mRect.vertex.AllocateAndBindMemory(device, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vkf::tool::CopyBuffer(device, sizeof(indices), indices, graphics, pool, mRect.index);
    vkf::tool::CopyBuffer(device, sizeof(vertices), vertices, graphics, pool, mRect.vertex);
}

VulkanSplit::VulkanSplit(/* args */){
}

VulkanSplit::~VulkanSplit(){
}

void VulkanSplit::Cleanup(VkDevice device){
    vkDestroySampler(device, mTextureSampler, nullptr);
    vkDestroyDescriptorSetLayout(device, mSetLayout, nullptr);

    mTexture.Destroy(device);

    mRect.index.Destroy(device);
    mRect.vertex.Destroy(device);

    uniform.position.Destroy(device);
    uniform.background.Destroy(device);

    DestroyGraphicsPipeline(device);
}

void VulkanSplit::Setup(VkPhysicalDevice physicalDevice, VkDevice device, VkQueue graphics, const VulkanPool &pool, uint32_t imageCount){
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
    mMinUniformBufferOffset = ALIGN(sizeof(Uniform), physicalDeviceProperties.limits.minUniformBufferOffsetAlignment);
    // int32_t mubo = ALIGN(sizeof(glm::mat4), physicalDeviceProperties.limits.minUniformBufferOffsetAlignment);

    vkf::CreateTextureSampler(device, mTextureSampler);

    SetupDescriptorSetLayout(device);

    CreateRectResource(device, graphics, pool.commandPool);
    uniform.position.CreateBuffer(device, mMinUniformBufferOffset * imageCount, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    uniform.position.AllocateAndBindMemory(device, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    uniform.background.CreateBuffer(device, sizeof(glm::mat4(1)), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    uniform.background.AllocateAndBindMemory(device, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    uniform.position.size = mMinUniformBufferOffset;

    vkf::tool::AllocateDescriptorSets(device, pool.descriptorPool, &mSetLayout, 1, &descriptorset.set);
    vkf::tool::AllocateDescriptorSets(device, pool.descriptorPool, &mSetLayout, 1, &descriptorset.background);

    mImageSize.width = 0;
    mImageSize.height = 0;
    mTexture.image = VK_NULL_HANDLE;

    UpdateDescriptorSet(device);
    // ResetImageIndex(imageCount);
}

void VulkanSplit::Draw(VkCommandBuffer command, uint32_t windowWidth, uint32_t windowHeight, const glm::vec3&backgroundColor){
    PushConstant pc;
    uint32_t dynamicOffset = 0;
    pc.color = backgroundColor;
    pc.projection = glm::ortho(0.0f, (float)windowWidth, 0.0f, (float)windowHeight, -1.0f, 1.0f);
    pipelines.background.BindPipeline(command);
    pipelines.background.BindDescriptorSet(command, descriptorset.background, 1, &dynamicOffset);
    pipelines.background.PushPushConstant(command, VK_SHADER_STAGE_VERTEX_BIT, sizeof(PushConstant), &pc);
    //绘制背景
    Draw(command, &mRect);
    if(mTexture.image != VK_NULL_HANDLE){
        //绘制九宫格图片
        pipelines.pipeline.BindPipeline(command);
        pipelines.pipeline.PushPushConstant(command, VK_SHADER_STAGE_VERTEX_BIT, sizeof(glm::mat4), &pc.projection);
        for (uint32_t i = 0; i < mImageCount; ++i){
            dynamicOffset = i * mMinUniformBufferOffset;
            pipelines.pipeline.BindDescriptorSet(command, descriptorset.set, 1, &dynamicOffset);
            Draw(command, &mRect);
        }
    }
}

void VulkanSplit::DestroyGraphicsPipeline(VkDevice device){
    std::vector<uint32_t>cacheData;
    pipelines.background.DestroyCache(device, cacheData);
    vkf::tool::WriteFileContent("BackgroundPipelineCache", cacheData.data(), cacheData.size() * sizeof(uint32_t)); 
    pipelines.background.DestroyLayout(device);
    pipelines.background.DestroyPipeline(device);

    pipelines.pipeline.DestroyCache(device, cacheData);
    vkf::tool::WriteFileContent("GraphicsPipelineCache", cacheData.data(), cacheData.size() * sizeof(uint32_t)); 
    pipelines.pipeline.DestroyLayout(device);
    pipelines.pipeline.DestroyPipeline(device);
}

void VulkanSplit::CreateGraphicsPipeline(VkDevice device, VkRenderPass renderpass, uint32_t scissorWidth, uint32_t scissorHeight){
    std::vector<uint32_t>cacheData;
    vkf::tool::GetFileContent("GraphicsPipelineCache", cacheData);

    pipelines.pipeline.PushShader(device, VK_SHADER_STAGE_VERTEX_BIT, "shaders/baseVert.spv");
    pipelines.pipeline.PushShader(device, VK_SHADER_STAGE_FRAGMENT_BIT, "shaders/baseFrag.spv");

    pipelines.pipeline.PushScissor(scissorWidth, scissorHeight);
    pipelines.pipeline.PushViewport(scissorWidth, scissorHeight);

    pipelines.pipeline.PushVertexInputBindingDescription(sizeof(Vertex));
    pipelines.pipeline.PushPushConstant(sizeof(glm::mat4), VK_SHADER_STAGE_VERTEX_BIT);
    pipelines.pipeline.PushVertexInputAttributeDescription(0, 0, VK_FORMAT_R32G32B32_SFLOAT);
    pipelines.pipeline.PushVertexInputAttributeDescription(1, offsetof(Vertex, Vertex::mUv), VK_FORMAT_R32G32_SFLOAT);

    pipelines.pipeline.CreateCache(device, cacheData);
    pipelines.pipeline.CreateLayout(device, {mSetLayout});
    pipelines.pipeline.CreatePipeline(device, renderpass);

    vkf::tool::GetFileContent("BackgroundPipelineCache", cacheData);

    pipelines.background.PushShader(device, VK_SHADER_STAGE_VERTEX_BIT, "shaders/baseColorVert.spv");
    pipelines.background.PushShader(device, VK_SHADER_STAGE_FRAGMENT_BIT, "shaders/baseColorFrag.spv");

    pipelines.background.PushScissor(scissorWidth, scissorHeight);
    pipelines.background.PushViewport(scissorWidth, scissorHeight);

    pipelines.background.PushVertexInputBindingDescription(sizeof(Vertex));
    pipelines.background.PushPushConstant(sizeof(PushConstant), VK_SHADER_STAGE_VERTEX_BIT);
    pipelines.background.PushVertexInputAttributeDescription(0, 0, VK_FORMAT_R32G32B32_SFLOAT);
    pipelines.background.PushVertexInputAttributeDescription(1, offsetof(Vertex, Vertex::mUv), VK_FORMAT_R32G32_SFLOAT);

    pipelines.background.CreateCache(device, cacheData);
    pipelines.background.CreateLayout(device, {mSetLayout});
    pipelines.background.CreatePipeline(device, renderpass);
}

void VulkanSplit::UpdateTexture(VkDevice device, uint32_t index, const glm::vec3&pos, const glm::vec2&size){
    // const uint32_t index = ROW_COLUMN_INDEX(row, column, column);
    Uniform ubo;
    const glm::mat4 modelScale = glm::scale(glm::mat4(1), glm::vec3(size, 1));
    // const glm::mat4 modelScale = glm::scale(glm::mat4(1), glm::vec3(mImageSize.width, mImageSize.height, 1));
    ubo.imageIndex = index;
    ubo.model = glm::translate(glm::mat4(1.0f), pos) * modelScale;
    uniform.position.UpdateData(device, mMinUniformBufferOffset, &ubo, mMinUniformBufferOffset * index);
}
void VulkanSplit::UpdateBackground(VkDevice device, uint32_t row, uint32_t column, uint32_t windowWidth, uint32_t windowHeight){
    Uniform ubo;
    const uint32_t offset = 10;
    // const glm::mat4 modelScale = glm::scale(glm::mat4(1), glm::vec3(mImageSize.width, mImageSize.height, 1));
    const glm::vec3 backgroundSize = glm::vec3((column + 1) * offset + column * mImageSize.width, (row + 1) * offset + row * mImageSize.height, 1);
    const glm::vec3 backgroundPos = glm::vec3(windowWidth * .5 - backgroundSize.x * .5, windowHeight * .5 - backgroundSize.y * .5, 0);
    if(mImageSize.width != 0 && mImageSize.height != 0)
        ubo.model = glm::scale(glm::translate(glm::mat4(1.0f), backgroundPos), backgroundSize);
    else
        ubo.model = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(windowWidth - (windowWidth - 3 * offset), windowHeight - (windowHeight - 5 * offset), 0)), glm::vec3(windowWidth - 6 * offset, windowHeight - 7 * offset, 1));
    uniform.background.UpdateData(device, &ubo.model);
}

void VulkanSplit::UpdateDescriptorSet(VkDevice device){
    VkDescriptorSetLayoutBinding setlayoutBindings[2] = {};
    setlayoutBindings[0].descriptorCount = 1;
    setlayoutBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    setlayoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    setlayoutBindings[1].binding = 1;
    setlayoutBindings[1].descriptorCount = 1;
    setlayoutBindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    setlayoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    vkf::tool::UpdateDescriptorSets(device, 1, setlayoutBindings, { uniform.background }, {}, descriptorset.background);
    if(mTexture.image != VK_NULL_HANDLE)
        vkf::tool::UpdateDescriptorSets(device, 2, setlayoutBindings, { uniform.position }, { mTexture }, descriptorset.set, mTextureSampler);
}
void VulkanSplit::ChangeTextureImage(VkDevice device, const void **datas, uint32_t imageCount, uint32_t width, uint32_t height, VkQueue graphics, VkCommandPool pool){
    vkDeviceWaitIdle(device);
    if(mTexture.image != VK_NULL_HANDLE){
        mTexture.Destroy(device);
        mTexture.image = VK_NULL_HANDLE;
    }
    mImageCount = imageCount;
    mImageSize.width = width;
    mImageSize.height = height;
    vkf::CreateImageArray(device, datas, imageCount, width, height, mTexture, pool, graphics);
}