#include <array>
#include "VulkanSplit.h"
VkResult VulkanSplit::PrepareOffscreenRenderpass(VkDevice device){
    std::array<VkAttachmentDescription, 1>attachmentDescription{};
    attachmentDescription[0].format = COLOR_FORMAT;
    attachmentDescription[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescription[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachmentDescription[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;			// We will read from depth, so it's important to store the depth attachment results
    attachmentDescription[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescription[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachmentDescription[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;			// We don't care about initial layout of the attachment
    attachmentDescription[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;// Attachment will be transitioned to shader read at render pass end

    VkAttachmentReference attachmentReference = {};
    attachmentReference.attachment = 0;
    attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.colorAttachmentCount = 1;							
    subpass.pColorAttachments = &attachmentReference;
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    // Use subpass dependencies for layout transitions
    std::array<VkSubpassDependency, 2> dependencies;
    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    VkRenderPassCreateInfo renderPassCreateInfo = {};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpass;
    renderPassCreateInfo.pDependencies = dependencies.data();
    renderPassCreateInfo.pAttachments = attachmentDescription.data();
    renderPassCreateInfo.attachmentCount = attachmentDescription.size();
    renderPassCreateInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());

    return vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &offscreenPass.renderPass);
}
void VulkanSplit::DrawGraphics(VkCommandBuffer command, const BaseGraphic *graphic){
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
void VulkanSplit::DrawImage(VkCommandBuffer command, uint32_t windowWidth, uint32_t windowHeight){
    uint32_t dynamicOffset;
    const glm::mat4 projection = glm::ortho(0.0f, (float)windowWidth, 0.0f, (float)windowHeight, -1.0f, 1.0f);
    pipelines.pipeline.BindPipeline(command);
    pipelines.pipeline.PushPushConstant(command, VK_SHADER_STAGE_VERTEX_BIT, sizeof(glm::mat4), &projection);
    for (uint32_t i = 0; i < mImageCount; ++i){
        dynamicOffset = i * mMinUniformBufferOffset;
        pipelines.pipeline.BindDescriptorSet(command, descriptorset.set, 1, &dynamicOffset);
        DrawGraphics(command, &mRect);
    }
}
void VulkanSplit::DrawDebug(VkCommandBuffer command, uint32_t windowWidth, uint32_t windowHeight){
    // uint32_t dynamicOffset = 0;
    // const glm::mat4 projection = glm::ortho(0.0f, (float)windowWidth * .25f, 0.0f, (float)windowHeight * .25f, -1.0f, 1.0f);
    // pipelines.debug.BindPipeline(command);
    // pipelines.debug.BindDescriptorSet(command, descriptorset.debug, 1, &dynamicOffset);
    // pipelines.debug.PushPushConstant(command, VK_SHADER_STAGE_VERTEX_BIT, sizeof(glm::mat4), &projection);
    // DrawGraphics(command, &mRect);
}
VulkanSplit::VulkanSplit(/* args */)
{
}

VulkanSplit::~VulkanSplit(){
}

void VulkanSplit::Cleanup(VkDevice device){
    vkDestroySampler(device, mTextureSampler, nullptr);
    vkDestroyDescriptorSetLayout(device, mSetLayout, nullptr);
    vkDestroySemaphore(device, offscreenPass.semaphore, nullptr);
    vkDestroyRenderPass(device, offscreenPass.renderPass, nullptr);
    vkDestroyFramebuffer(device, offscreenPass.frameBuffer, nullptr);

    mTexture.Destroy(device);

    mRect.index.Destroy(device);
    mRect.vertex.Destroy(device);

    uniform.position.Destroy(device);
    uniform.background.Destroy(device);

    DestroyGraphicsPipeline(device);
}

void VulkanSplit::RecreateImageUniform(VkDevice device, uint32_t imageCount){
    if(uniform.position.buffer != VK_NULL_HANDLE){
        uniform.position.Destroy(device);
    }
    uniform.position.CreateBuffer(device, mMinUniformBufferOffset * imageCount, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    uniform.position.AllocateAndBindMemory(device, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    uniform.position.size = mMinUniformBufferOffset;
}
VkResult VulkanSplit::ReprepareOffscreenFramebuffer(VkDevice device, const glm::vec3&backgroundSize){
    //应该是背景图片大小而不是窗口大小
    if(offscreenPass.width != backgroundSize.x || offscreenPass.height != backgroundSize.y){
        offscreenPass.color.Destroy(device);
        // vkDestroyRenderPass(device, offscreenPass.renderPass, nullptr);
        vkDestroyFramebuffer(device, offscreenPass.frameBuffer, nullptr);
    }
    offscreenPass.width = backgroundSize.x;
    offscreenPass.height = backgroundSize.y;

    offscreenPass.color.size.depth = 1;
    offscreenPass.color.size.width = offscreenPass.width;
    offscreenPass.color.size.height = offscreenPass.height;
    offscreenPass.color.CreateImage(device, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, COLOR_FORMAT);// We will sample directly from the color attachment for the shadow mapping
    offscreenPass.color.AllocateAndBindMemory(device, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    offscreenPass.color.CreateImageView(device, COLOR_FORMAT);

    if(offscreenPass.renderPass == VK_NULL_HANDLE)PrepareOffscreenRenderpass(device);

    // Create frame buffer
    return vkf::CreateFrameBuffer(device, offscreenPass.renderPass, { offscreenPass.width, offscreenPass.height }, { offscreenPass.color.view }, offscreenPass.frameBuffer);
}
void VulkanSplit::Setup(VkPhysicalDevice physicalDevice, VkDevice device, VkQueue graphics, const VulkanPool &pool, uint32_t imageCount){
    mTexture.image = VK_NULL_HANDLE;
    uniform.position.buffer = VK_NULL_HANDLE;
    offscreenPass.renderPass = VK_NULL_HANDLE;

    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
    mMinUniformBufferOffset = ALIGN(sizeof(Uniform), physicalDeviceProperties.limits.minUniformBufferOffsetAlignment);

    vkf::CreateTextureSampler(device, mTextureSampler);

    SetupDescriptorSetLayout(device);

    CreateRectResource(device, graphics, pool.commandPool);

    RecreateImageUniform(device, imageCount);

    uniform.background.CreateBuffer(device, sizeof(glm::mat4), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    uniform.background.AllocateAndBindMemory(device, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    // uniform.debug.CreateBuffer(device, sizeof(glm::mat4), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    // uniform.debug.AllocateAndBindMemory(device, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    // const glm::mat4 model = glm::scale(glm::mat4(1), glm::vec3(800 * .25, 800 * .25, 1));
    // uniform.debug.UpdateData(device, &model);

    uniform.offscreenBackground.CreateBuffer(device, sizeof(glm::mat4), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    uniform.offscreenBackground.AllocateAndBindMemory(device, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    uniform.offscreenPosition.CreateBuffer(device, mMinUniformBufferOffset * imageCount, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    uniform.offscreenPosition.AllocateAndBindMemory(device, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    uniform.offscreenPosition.size = mMinUniformBufferOffset;

    vkf::tool::AllocateDescriptorSets(device, pool.descriptorPool, &mSetLayout, 1, &descriptorset.set);
    vkf::tool::AllocateDescriptorSets(device, pool.descriptorPool, &mSetLayout, 1, &descriptorset.background);

    vkf::tool::AllocateDescriptorSets(device, pool.descriptorPool, &mSetLayout, 1, &descriptorset.offscreenPosition);
    vkf::tool::AllocateDescriptorSets(device, pool.descriptorPool, &mSetLayout, 1, &descriptorset.offscreenBackground);
    // vkf::tool::AllocateDescriptorSets(device, pool.descriptorPool, &mSetLayout, 1, &descriptorset.debug);

    vkf::tool::AllocateCommandBuffers(device, pool.commandPool, 1, &offscreenPass.command);

    VkSemaphoreCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vkCreateSemaphore(device, &info, nullptr, &offscreenPass.semaphore);

    ReprepareOffscreenFramebuffer(device, glm::vec3(100, 100, 1));

    UpdateDescriptorSet(device);
    // ResetImageIndex(imageCount);
}

void VulkanSplit::Draw(VkCommandBuffer command, uint32_t windowWidth, uint32_t windowHeight, const glm::vec3&color){
    PushConstant pc;
    uint32_t dynamicOffset = 0;
    pc.color = color;
    pc.projection = glm::ortho(0.0f, (float)windowWidth, 0.0f, (float)windowHeight, -1.0f, 1.0f);
    pipelines.background.BindPipeline(command);
    pipelines.background.BindDescriptorSet(command, descriptorset.background, 1, &dynamicOffset);
    pipelines.background.PushPushConstant(command, VK_SHADER_STAGE_VERTEX_BIT, sizeof(PushConstant), &pc);
    //绘制背景
    DrawGraphics(command, &mRect);
    if(mTexture.image != VK_NULL_HANDLE){
        //绘制九宫格图片
        DrawImage(command, windowWidth, windowHeight);
    }
}

void VulkanSplit::RerodOffscreenCommand(const glm::vec3 &color){
    // vkf::tool::BeginCommands(offscreenPass.command, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
    vkf::tool::BeginCommands(offscreenPass.command, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    vkf::tool::BeginRenderPassGeneral(offscreenPass.command, offscreenPass.frameBuffer, offscreenPass.renderPass, offscreenPass.width, offscreenPass.height);
    VkRect2D rect = {};
    VkViewport viewport = {};
    viewport.maxDepth = 1;
    viewport.width = offscreenPass.width;
    viewport.height = offscreenPass.height;
    rect.extent.width = offscreenPass.width;
    rect.extent.height = offscreenPass.height;
    vkCmdSetScissor(offscreenPass.command, 0, 1, &rect);
    vkCmdSetViewport(offscreenPass.command, 0, 1, &viewport);
    PushConstant pc;
    uint32_t dynamicOffset = 0;
    pc.color = color;
    pc.projection = glm::ortho(0.0f, (float)offscreenPass.width, 0.0f, (float)offscreenPass.height, -1.0f, 1.0f);
    pipelines.offscreenBackground.BindPipeline(offscreenPass.command);
    pipelines.offscreenBackground.BindDescriptorSet(offscreenPass.command, descriptorset.offscreenBackground, 1, &dynamicOffset);
    pipelines.offscreenBackground.PushPushConstant(offscreenPass.command, VK_SHADER_STAGE_VERTEX_BIT, sizeof(PushConstant), &pc);
    //绘制背景
    DrawGraphics(offscreenPass.command, &mRect);
    if(mTexture.image != VK_NULL_HANDLE){
        //绘制九宫格图片
        pipelines.offscreen.BindPipeline(offscreenPass.command);
        pipelines.offscreen.PushPushConstant(offscreenPass.command, VK_SHADER_STAGE_VERTEX_BIT, sizeof(glm::mat4), &pc.projection);
        for (uint32_t i = 0; i < mImageCount; ++i){
            dynamicOffset = i * mMinUniformBufferOffset;
            pipelines.offscreen.BindDescriptorSet(offscreenPass.command, descriptorset.offscreenPosition, 1, &dynamicOffset);
            DrawGraphics(offscreenPass.command, &mRect);
        }
    }
    vkCmdEndRenderPass(offscreenPass.command);
    vkEndCommandBuffer(offscreenPass.command);
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

    pipelines.offscreen.DestroyCache(device, cacheData);
    vkf::tool::WriteFileContent("OffscreenPipelineCache", cacheData.data(), cacheData.size() * sizeof(uint32_t)); 
    pipelines.offscreen.DestroyLayout(device);
    pipelines.offscreen.DestroyPipeline(device);

    pipelines.offscreenBackground.DestroyCache(device, cacheData);
    vkf::tool::WriteFileContent("OffscreenColorPipelineCache", cacheData.data(), cacheData.size() * sizeof(uint32_t)); 
    pipelines.offscreenBackground.DestroyLayout(device);
    pipelines.offscreenBackground.DestroyPipeline(device);
}

void VulkanSplit::CreateGraphicsPipeline(VkDevice device, VkRenderPass renderpass, uint32_t scissorWidth, uint32_t scissorHeight){
    std::vector<uint32_t>cacheData;
    // vkf::tool::GetFileContent("OffscreenPipelineCache", cacheData);
    // VkRect2D rect = {};
    // rect.extent.width = scissorWidth * .25;
    // rect.extent.height = scissorHeight * .25;
    // rect.offset.x = scissorWidth - scissorWidth * .25;
    // pipelines.debug.PushScissor(&rect);
    // pipelines.debug.PushViewport(scissorWidth * .25, scissorHeight * .25, scissorWidth - scissorWidth * .25);
    // pipelines.debug.PushShader(device, VK_SHADER_STAGE_VERTEX_BIT, "shaders/baseVert.spv");
    // pipelines.debug.PushShader(device, VK_SHADER_STAGE_FRAGMENT_BIT, "shaders/debugFrag.spv");

    // pipelines.debug.PushVertexInputBindingDescription(sizeof(Vertex));
    // pipelines.debug.PushPushConstant(sizeof(glm::mat4), VK_SHADER_STAGE_VERTEX_BIT);
    // pipelines.debug.PushVertexInputAttributeDescription(0, 0, VK_FORMAT_R32G32B32_SFLOAT);
    // pipelines.debug.PushVertexInputAttributeDescription(1, offsetof(Vertex, Vertex::mUv), VK_FORMAT_R32G32_SFLOAT);

    // pipelines.debug.CreateCache(device, cacheData);
    // pipelines.debug.CreateLayout(device, { mSetLayout });
    // pipelines.debug.CreatePipeline(device, renderpass);

    vkf::tool::GetFileContent("OffscreenPipelineCache", cacheData);
    // pipelines.offscreen.PushScissor(offscreenPass.width, offscreenPass.height);
    // pipelines.offscreen.PushViewport(offscreenPass.width, offscreenPass.height);
    pipelines.offscreen.PushShader(device, VK_SHADER_STAGE_VERTEX_BIT, "shaders/baseVert.spv");
    pipelines.offscreen.PushShader(device, VK_SHADER_STAGE_FRAGMENT_BIT, "shaders/baseFrag.spv");

    pipelines.offscreen.PushVertexInputBindingDescription(sizeof(Vertex));
    pipelines.offscreen.PushPushConstant(sizeof(glm::mat4), VK_SHADER_STAGE_VERTEX_BIT);
    pipelines.offscreen.PushVertexInputAttributeDescription(0, 0, VK_FORMAT_R32G32B32_SFLOAT);
    pipelines.offscreen.PushVertexInputAttributeDescription(1, offsetof(Vertex, Vertex::mUv), VK_FORMAT_R32G32_SFLOAT);

    pipelines.offscreen.CreateCache(device, cacheData);
    pipelines.offscreen.CreateLayout(device, { mSetLayout });
    pipelines.offscreen.CreatePipeline(device, offscreenPass.renderPass);

    vkf::tool::GetFileContent("OffscreenColorPipelineCache", cacheData);
    // pipelines.offscreenColor.PushScissor(offscreenPass.width, offscreenPass.height);
    // pipelines.offscreenColor.PushViewport(offscreenPass.width, offscreenPass.height);
    pipelines.offscreenBackground.PushShader(device, VK_SHADER_STAGE_VERTEX_BIT, "shaders/baseColorVert.spv");
    pipelines.offscreenBackground.PushShader(device, VK_SHADER_STAGE_FRAGMENT_BIT, "shaders/baseColorFrag.spv");

    pipelines.offscreenBackground.PushVertexInputBindingDescription(sizeof(Vertex));
    pipelines.offscreenBackground.PushPushConstant(sizeof(PushConstant), VK_SHADER_STAGE_VERTEX_BIT);
    pipelines.offscreenBackground.PushVertexInputAttributeDescription(0, 0, VK_FORMAT_R32G32B32_SFLOAT);
    pipelines.offscreenBackground.PushVertexInputAttributeDescription(1, offsetof(Vertex, Vertex::mUv), VK_FORMAT_R32G32_SFLOAT);

    pipelines.offscreenBackground.CreateCache(device, cacheData);
    pipelines.offscreenBackground.CreateLayout(device, { mSetLayout });
    pipelines.offscreenBackground.CreatePipeline(device, offscreenPass.renderPass);
    //---------------------------------
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
void VulkanSplit::UpdateOffscreenBackground(VkDevice device, const glm::vec3&size){
    const glm::mat4 model = glm::scale(glm::mat4(1), size);
    uniform.offscreenBackground.UpdateData(device, &model);
}
void VulkanSplit::UpdateBackground(VkDevice device, const glm::vec3&pos, const glm::vec3&size){
    const glm::mat4 model = glm::scale(glm::translate(glm::mat4(1), pos), size);
    uniform.background.UpdateData(device, &model);
}
void VulkanSplit::UpdateOffscreen(VkDevice device, uint32_t index, const glm::vec3&pos, const glm::vec2&size){
    // const uint32_t index = ROW_COLUMN_INDEX(row, column, column);
    Uniform ubo;
    const glm::mat4 modelScale = glm::scale(glm::mat4(1), glm::vec3(size, 1));
    // const glm::mat4 modelScale = glm::scale(glm::mat4(1), glm::vec3(mImageSize.width, mImageSize.height, 1));
    ubo.imageIndex = index;
    ubo.model = glm::translate(glm::mat4(1.0f), pos) * modelScale;
    uniform.offscreenPosition.UpdateData(device, mMinUniformBufferOffset, &ubo, mMinUniformBufferOffset * index);
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
    if(mTexture.image != VK_NULL_HANDLE){
        vkf::tool::UpdateDescriptorSets(device, 2, setlayoutBindings, { uniform.position }, { mTexture }, descriptorset.set, mTextureSampler);
    }
    setlayoutBindings[0].descriptorCount = 1;
    setlayoutBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    setlayoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    setlayoutBindings[1].binding = 1;
    setlayoutBindings[1].descriptorCount = 1;
    setlayoutBindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    setlayoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    vkf::tool::UpdateDescriptorSets(device, 1, setlayoutBindings, { uniform.offscreenBackground }, {}, descriptorset.offscreenBackground);
    
    // vkf::tool::UpdateDescriptorSets(device, 2, setlayoutBindings, { uniform.debug }, { offscreenPass.color }, descriptorset.debug, mTextureSampler);
    if(mTexture.image != VK_NULL_HANDLE){
        vkf::tool::UpdateDescriptorSets(device, 2, setlayoutBindings, { uniform.offscreenPosition }, { mTexture }, descriptorset.offscreenPosition, mTextureSampler);
    }
}
void VulkanSplit::ChangeTextureImage(VkDevice device, const void **datas, uint32_t imageCount, uint32_t width, uint32_t height, VkQueue graphics, VkCommandPool pool){
    vkDeviceWaitIdle(device);
    if(mTexture.image != VK_NULL_HANDLE){
        mTexture.Destroy(device);
    }
    mImageCount = imageCount;
    vkf::CreateImageArray(device, datas, imageCount, width, height, mTexture, pool, graphics);
}

void VulkanSplit::DrawFrame(VkDevice device, uint32_t currentFrame, const VkCommandBuffer &commandbuffers, VkSwapchainKHR swapchain, const VulkanQueue &vulkanQueue, const VulkanSynchronize &vulkanSynchronize, void (*recreateSwapchain)(void *userData), void *userData){
    vkWaitForFences(device, 1, &vulkanSynchronize.fences, VK_TRUE, UINT16_MAX);
    vkResetFences(device, 1, &vulkanSynchronize.fences);
    uint32_t imageIndex = 0;
    vkf::PrepareFrame(device, swapchain, vulkanSynchronize.imageAcquired[currentFrame], imageIndex);
    vkf::RenderFrame(device, offscreenPass.command, vulkanQueue.graphics, vulkanSynchronize.imageAcquired[currentFrame], offscreenPass.semaphore, VK_NULL_HANDLE);
    vkf::RenderFrame(device, commandbuffers, vulkanQueue.graphics, offscreenPass.semaphore, vulkanSynchronize.renderComplete[currentFrame], vulkanSynchronize.fences);
    vkf::SubmitFrame(device, imageIndex, swapchain, vulkanQueue.present, vulkanSynchronize.renderComplete[currentFrame], nullptr, nullptr);
     //VK_CHECK(vkQueueWaitIdle(vulkanQueue.graphics));
}
