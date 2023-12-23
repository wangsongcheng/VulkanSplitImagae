#include "Pipeline.h"
// #include "imgui.h"

// void Pipeline::DestroyDescriptorSet(VkDevice device, DescriptorSet&descriptorSet){
//     switch (descriptorSet.binding.descriptorType){
//     case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
//     case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
//         destroyBuffer(device, descriptorSet.uniform);
//         break;
//     case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
//     case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
//         destroyBuffer(device, descriptorSet.storage);
//         break;
//     case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
//         destroyImage(device, descriptorSet.sampledImage);
//         break;
//     default:
//         break;
//     }
// }
//void Pipeline::GetDescriptorSetBinding(const spirv_cross::CompilerGLSL&glsl, const spirv_cross::SmallVector<spirv_cross::Resource>&resource, VkDescriptorType type, std::vector<DescriptorSet>&out){
//    if(resource.empty())return;
//    DescriptorSet ds{};
//    ds.binding.descriptorCount = 1;
//    ds.binding.descriptorType = type;
//    for (size_t i = 0; i < resource.size(); ++i){
//        const spirv_cross::Resource descriptorSet = resource[i];
//        ds.uiSet = glsl.get_decoration(descriptorSet.id, spv::DecorationDescriptorSet);
//        if(type != VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER){
//            ds.uiSize = glsl.get_declared_struct_size(glsl.get_type(descriptorSet.type_id));
//        }
//        // if(uiSet > descriptorSet.size())descriptorSet.resize(uiSet + 1);
//        ds.binding.binding = glsl.get_decoration(descriptorSet.id, spv::DecorationBinding);
//        ds.mName = descriptorSet.name;
//        ds.binding.stageFlags = GetShaderStageFlags(glsl.get_entry_points_and_stages()[0].execution_model);
//        out.push_back(ds);
//    }
//}
//std::vector<Shader>::iterator Pipeline::GetShaders(VkShaderStageFlags stage){
//    auto it = mShaders.begin();
//    for (; it != mShaders.end(); ++it){
//        if(it->mStage == stage)break;
//    }
//    return it;
//}
//VkShaderStageFlags Pipeline::GetShaderStageFlags(const spv::ExecutionModel&executionModel){
//    VkShaderStageFlags stage;
//    switch (executionModel){
//    case spv::ExecutionModelVertex:
//        stage = VK_SHADER_STAGE_VERTEX_BIT;
//        break;
//    case spv::ExecutionModelTessellationControl:
//        stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
//        break;
//    case spv::ExecutionModelTessellationEvaluation:
//        stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
//        break;
//    case spv::ExecutionModelGeometry:
//        stage = VK_SHADER_STAGE_GEOMETRY_BIT;
//        break;
//    case spv::ExecutionModelFragment:
//        stage = VK_SHADER_STAGE_FRAGMENT_BIT;
//        break;
//    case spv::ExecutionModelGLCompute:
//        stage = VK_SHADER_STAGE_COMPUTE_BIT;
//        break;
//    default:
//        break;
//    }
//    return stage;
//}
//std::string Pipeline::GetShaderStageName(VkShaderStageFlags stage){
//#define IS_STAGE(strStage, result) case strStage: result = #strStage;break;
//    std::string result;
//    switch (stage){
//    IS_STAGE(VK_SHADER_STAGE_VERTEX_BIT, result);
//    IS_STAGE(VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, result);
//    IS_STAGE(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, result);
//    IS_STAGE(VK_SHADER_STAGE_GEOMETRY_BIT, result);
//    IS_STAGE(VK_SHADER_STAGE_FRAGMENT_BIT, result);
//    IS_STAGE(VK_SHADER_STAGE_COMPUTE_BIT, result);
//    default:
//        result = "未知类型";
//        break;
//    }
//    result = result.c_str() + strlen("VK_SHADER_STAGE_");
//    std::size_t pos = result.find("_BIT");
//    if(pos != std::string::npos){
//        result[pos] = 0;
//    }
//    pos = result.find('_');
//    while (pos != std::string::npos){
//        result[pos] = ' ';
//        pos = result.find('_');
//    }
//    return result;
//#undef IS_STAGE
//}
Pipeline::Pipeline(){

}
Pipeline::~Pipeline(){
    cleanup();
}
/*
    着色器各种信息，一般用在描述符集布局上
    着色器顶点属性信息
*/
//auto Pipeline::AddShader(VkDevice device, const std::vector<uint32_t>&code){
//    spirv_cross::CompilerGLSL glsl(code);
//    spirv_cross::SmallVector<spirv_cross::EntryPoint> entry = glsl.get_entry_points_and_stages();
//    VkShaderStageFlags stage = GetShaderStageFlags(entry[0].execution_model);
//    auto it = GetShaders(stage);
//    if(it != mShaders.end()){
//        DeleteShader(device, it->mStage);
//    }
//    mShaders.push_back(Shader(GetShaderStageName(stage), (VkShaderStageFlagBits)stage));
//    it = GetShaders(stage);
//    VkShaderModuleCreateInfo info = {};
//    info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
//    info.pCode = code.data();
//    info.codeSize = code.size() * sizeof(uint32_t);
//    vkCreateShaderModule(device, &info, nullptr, &it->mModule);
//
//    spirv_cross::ShaderResources resource = glsl.get_shader_resources();
//    spirv_cross::SmallVector<spirv_cross::Resource>pcResource = resource.push_constant_buffers;
//    if(!pcResource.empty()){
//        it->mPushConstant.resize(pcResource.size());
//        PushConstantInfo&pc = it->mPushConstant[0];
//        pc.mName = pcResource[0].name;
//        pc.mPushConstantRange.offset = 0;
//        pc.mPushConstantRange.stageFlags = it->mStage;
//        pc.mPushConstantRange.size = glsl.get_declared_struct_size(glsl.get_type(pcResource[0].type_id));
//        // updatePushConstant(pc, bt_m);
//    }
//
//    //描述符集相关
//    GetDescriptorSetBinding(glsl, resource.uniform_buffers, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, it->mDescriptorSet);
//    GetDescriptorSetBinding(glsl, resource.storage_buffers, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, it->mDescriptorSet);
//    GetDescriptorSetBinding(glsl, resource.sampled_images, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, it->mDescriptorSet);
//    return it;
//}
void Pipeline::BindDescriptorSet(VkCommandBuffer cmd, VkDescriptorSet&set, uint32_t firstSet, uint32_t descriptorSetCount)const{
    BindDescriptorSet(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, set, firstSet, 0, nullptr, descriptorSetCount);
}
void Pipeline::BindDescriptorSet(VkCommandBuffer cmd, VkDescriptorSet&set, uint32_t dynamicOffsetCount, const uint32_t *pDynamicOffsets, uint32_t descriptorSetCount)const{
    BindDescriptorSet(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, set, 0, dynamicOffsetCount, pDynamicOffsets, descriptorSetCount);
}
void Pipeline::BindDescriptorSet(VkCommandBuffer cmd, VkPipelineBindPoint pipelineBindPoint, VkDescriptorSet&set, uint32_t firstSet, uint32_t dynamicOffsetCount, const uint32_t *pDynamicOffsets, uint32_t descriptorSetCount)const{
    vkCmdBindDescriptorSets(cmd, pipelineBindPoint, mLayout, firstSet, descriptorSetCount, &set, dynamicOffsetCount, pDynamicOffsets);    
}
// VkResult Pipeline::CreateCache(VkDevice device, const std::vector<uint32_t>&cacheData){
//     VkPipelineCacheCreateInfo cacheInfo = {};
//     cacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
//     cacheInfo.initialDataSize = cacheData.size() * sizeof(uint32_t);
//     cacheInfo.pInitialData = cacheData.data();
//     return vkCreatePipelineCache(device, &cacheInfo, nullptr, &mCache);
// }
// VkResult Pipeline::CreateDescriptorSetLayout(VkDevice device){
//     if(mSetLayoutBindings.empty()){
//         printf("请先调用PushDescriptorSetLayoutBinding函数\n");
//         return VK_NOT_READY;
//     }
//     VkResult result;
//     VkDescriptorSetLayoutCreateInfo descriptorSetLayout{};
//     const VkDescriptorSetLayoutBinding *pBindings = mSetLayoutBindings.data();

//     mSetLayouts.resize(mSetLayoutBindingIndex.size());
//     descriptorSetLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
//     descriptorSetLayout.pBindings = pBindings;
//     descriptorSetLayout.bindingCount = mSetLayoutBindingIndex[0];
//     result = vkCreateDescriptorSetLayout(device, &descriptorSetLayout, nullptr, &mSetLayouts[0]);//如果i从0开始不能直接i-1,所以先这么写
//     if(result == VK_SUCCESS){
//         for (size_t i = 1; i < mSetLayouts.size(); ++i){
//             descriptorSetLayout.bindingCount = mSetLayoutBindingIndex[i];
//             descriptorSetLayout.pBindings = pBindings + i * mSetLayoutBindingIndex[i - 1];
//             result = vkCreateDescriptorSetLayout(device, &descriptorSetLayout, nullptr, &mSetLayouts[i]);
//         }
//     }
//    return result;
// }
VkResult Pipeline::CreateLayout(VkDevice device, const std::vector<VkDescriptorSetLayout>&setlayouts){
    // if(mSetLayouts.empty() || !mSetLayoutBindings.empty()){
    //     printf("请先调用CreateDescriptorSetLayout\n");
    //     return VK_NOT_READY;
    // }
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.pSetLayouts = setlayouts.data();
    pipelineLayoutInfo.setLayoutCount = setlayouts.size();
    pipelineLayoutInfo.pPushConstantRanges = mPushConstants.data();
    pipelineLayoutInfo.pushConstantRangeCount = mPushConstants.size();
    return vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &mLayout);
}
//void Pipeline::DeleteShader(VkDevice device, VkShaderStageFlags stage){
//    auto it = GetShaders(stage);
//    if(it == mShaders.end())return;
//    if(it->mModule != VK_NULL_HANDLE)vkDestroyShaderModule(device, it->mModule, nullptr);
//    // for (size_t uiDescriptorSet = 0; uiDescriptorSet < it->mDescriptorSet.size(); ++uiDescriptorSet){
//    //     DestroyDescriptorSet(device, it->mDescriptorSet[uiDescriptorSet]);
//    // }
//}
// void Pipeline::DestroyCache(VkDevice device, std::vector<uint32_t>&cacheData){
//     GetCacheData(device, cacheData);
//     vkDestroyPipelineCache(device, mCache, nullptr);
// }
void Pipeline::DestroyLayout(VkDevice device){
    vkDestroyPipelineLayout(device, mLayout, nullptr);
}
void Pipeline::DestroyPipeline(VkDevice device){
    vkDestroyPipeline(device, mPipeline, nullptr);
}
// void Pipeline::DestroySetLayout(VkDevice device){
//     for (size_t i = 0; i < mSetLayouts.size(); ++i){
//         vkDestroyDescriptorSetLayout(device, mSetLayouts[i], nullptr);
//     }
//     mSetLayouts.clear();
// }
//创建管线后就销毁了着色器模块
// void Pipeline::DestrotyShader(VkDevice device){
//    for (auto it = mShaders.begin(); it != mShaders.end(); ++it){
//        vkDestroyShaderModule(device, it->mModule, nullptr);
//    }
// }
// void Pipeline::GetCacheData(VkDevice device, std::vector<uint32_t>&cacheData){
//     size_t cacheDataSize;
//     vkGetPipelineCacheData(device, mCache, &cacheDataSize, nullptr);
//     cacheData.resize(cacheDataSize / sizeof(char));
//     vkGetPipelineCacheData(device, mCache, &cacheDataSize, cacheData.data());
// }
// void Pipeline::PushPushConstant(const VkPushConstantRange&pc) {
// }
void Pipeline::PushPushConstant(uint32_t size, VkShaderStageFlags stage, uint32_t offset){
    VkPushConstantRange pc{};
    pc.size = size;
    pc.offset = offset;
    pc.stageFlags = stage;
    mPushConstants.push_back(pc);
}
void Pipeline::PushConstant(VkCommandBuffer cmd, VkShaderStageFlags stage, uint32_t size, const void *pData, uint32_t offset)const{
    vkCmdPushConstants(cmd, mLayout, stage, offset, size, pData);
}
// void Pipeline::PushDescriptorSetLayoutBinding(uint32_t binding, VkShaderStageFlags stage, VkDescriptorType descriptorType, uint32_t uiSetIndex){
//     VkDescriptorSetLayoutBinding bind = {};
//     bind.binding = binding;
//     bind.stageFlags = stage;
//     bind.descriptorCount = 1;
//     bind.descriptorType = descriptorType;
//     mSetLayoutBindings.push_back(bind);

//     if(uiSetIndex + 1 > mSetLayoutBindingIndex.size())mSetLayoutBindingIndex.push_back(0);
//     ++mSetLayoutBindingIndex[uiSetIndex];//记录多少个bind属于这个描述符布局
// }
//分离原因:不同管线，但相同描述符布局，可以共同更新
// VkResult Pipeline::AllocateDescriptorSets(VkDevice device, VkDescriptorPool pool, uint32_t setCount, VkDescriptorSet *pDescriptorSet, uint32_t uiSetIndex){
//     VkDescriptorSetAllocateInfo info = {};
//     info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
//     info.descriptorPool = pool;
//     info.descriptorSetCount = setCount;
//     info.pSetLayouts = &mSetLayouts[uiSetIndex];
//     return vkAllocateDescriptorSets(device, &info, pDescriptorSet);
// }
// void Pipeline::UpdateDescriptorSets(VkDevice device, const std::vector<VulkanBuffer>&descriptorBuffer, const std::vector<VulkanImage>&descriptorImage, VkDescriptorSet&destSet, const VkSampler&textureSampler){
//     std::vector<uint32_t>index(2);//一个uniform一个图片采样器。如果需要其他则个数必须增加
//     std::vector<VkDescriptorImageInfo>descriptorImageInfo(descriptorImage.size());
//     std::vector<VkWriteDescriptorSet>writeDescriptorSet(mSetLayoutBindings.size());
//     std::vector<VkDescriptorBufferInfo> descriptorBufferInfo(descriptorBuffer.size());
//     for (size_t i = 0; i < writeDescriptorSet.size(); ++i){
//         writeDescriptorSet[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//         writeDescriptorSet[i].dstSet = destSet;
//         writeDescriptorSet[i].descriptorCount = 1;
//         writeDescriptorSet[i].dstBinding = mSetLayoutBindings[i].binding;//直接分离的话，该值一般用i。这样的话，着色器的bind索引就必须按顺序写
//         writeDescriptorSet[i].descriptorType = mSetLayoutBindings[i].descriptorType;
//         switch (writeDescriptorSet[i].descriptorType){
//         case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
//             descriptorImageInfo[index[1]].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//             descriptorImageInfo[index[1]].imageView = descriptorImage[index[1]].view;
//             descriptorImageInfo[index[1]].sampler = textureSampler;
//             writeDescriptorSet[i].pImageInfo = &descriptorImageInfo[index[1]++];
//             break;
//         case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
//         case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
//         case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
// //            descriptorBufferInfo[i].offset = 0;
//             descriptorBufferInfo[index[0]].range = descriptorBuffer[index[0]].size;
//             descriptorBufferInfo[index[0]].buffer = descriptorBuffer[index[0]].buffer;
//             writeDescriptorSet[i].pBufferInfo = &descriptorBufferInfo[index[0]++];
//             break;
//         default:
//             break;
//         }
//     }
//     vkUpdateDescriptorSets(device, writeDescriptorSet.size(), writeDescriptorSet.data(), 0, nullptr);
// }
// void Pipeline::PushShader(const Shader&shader){
//     mShaders.push_back(shader);
// }
// void Pipeline::PushShader(VkShaderStageFlags stage, const VkShaderModule& Module) {
//     Shader shader{};
//     shader.mStage = stage;
//     shader.mModule = Module;
//     shader.mEntryName = "main";
//     mShaders.push_back(shader);
// }
void Pipeline::PushShader(VkDevice device, VkShaderStageFlags stage, const std::string&file){
    std::vector<uint32_t>data;
    vkf::tool::GetFileContent(file, data);
    PushShader(device, stage, data);
}

void Pipeline::PushShader(VkDevice device, VkShaderStageFlags stage, const std::vector<uint32_t>&code) {
    PushShader(device, stage, code.size() * sizeof(uint32_t), code.data());
}

void Pipeline::PushShader(VkDevice device, VkShaderStageFlags stage, uint32_t size, const uint32_t *code){
    VkShaderModule sModule;
    VkShaderModuleCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    info.pCode = code;
    info.codeSize = size;
    VK_CHECK(vkCreateShaderModule(device, &info, nullptr, &sModule));
    Shader shader{};
    shader.mStage = stage;
    shader.mModule = sModule;
    shader.mEntryName = "main";
    mShaders.push_back(shader);
}

GraphicsPipeline::GraphicsPipeline(){
    // mState.mPolygonMode = VK_POLYGON_MODE_FILL;
}
GraphicsPipeline::GraphicsPipeline(const GraphicsPipelineStateInfo&stateInfo){
    mState = stateInfo;
}
GraphicsPipeline::~GraphicsPipeline(){
    cleanup();
}
// VkResult GraphicsPipeline::CreatePipeline(){

// }
// //如果传进来的是其他子类或父类。将直接崩溃
// bool GraphicsPipeline::operator==(const Pipeline&pipeline){
//     const GraphicsPipeline *gp = (GraphicsPipeline *)&pipeline;

//     // if(*this == pipeline){

//     // }
// }
VkResult GraphicsPipeline::CreatePipeline(VkDevice device, VkRenderPass renderPass, VkPipelineCache cache){
    uint32_t specializationIndex = 0;
    VkSpecializationInfo specialization = {};
    std::vector<VkPipelineShaderStageCreateInfo>ShaderStageCreateInfo(mShaders.size());
    for (uint32_t i = 0; i < mShaders.size(); ++i){
    // for (auto it = ShaderBegin(); it != ShaderEnd(); ++it){
        ShaderStageCreateInfo[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        ShaderStageCreateInfo[i].pName = mShaders[i].mEntryName.c_str();
        ShaderStageCreateInfo[i].stage = (VkShaderStageFlagBits)mShaders[i].mStage;
        ShaderStageCreateInfo[i].module = mShaders[i].mModule;
        if(specializationIndex < mSpecializationInfo.size()){
            if(ShaderStageCreateInfo[i].stage & mSpecializationInfo[specializationIndex].stage){
                specialization.pData = mSpecializationInfo[specializationIndex].pData;
                specialization.dataSize = mSpecializationInfo[specializationIndex].specializationMapEntrys[0].size;//这里默认所有大小都相同
                specialization.pMapEntries = mSpecializationInfo[specializationIndex].specializationMapEntrys.data();
                specialization.mapEntryCount = mSpecializationInfo[specializationIndex].specializationMapEntrys.size();
                ShaderStageCreateInfo[i].pSpecializationInfo = &specialization;
                ++specializationIndex;
            }
        }
    }
    VkPipelineVertexInputStateCreateInfo VertexInputStateCreateInfo{};// = vkt::Pipline::VertexInputStateCreateInfo(mVertexInput.mBindingDescription, mVertexInput.mAttributeDescriptions);
    VertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    VertexInputStateCreateInfo.pVertexBindingDescriptions = mBindingDescriptions.data();
    VertexInputStateCreateInfo.vertexBindingDescriptionCount = mBindingDescriptions.size();
    VertexInputStateCreateInfo.pVertexAttributeDescriptions = mVertexInputAttributeDescription.data();
    VertexInputStateCreateInfo.vertexAttributeDescriptionCount = mVertexInputAttributeDescription.size();
    // VkPipelineInputAssemblyStateCreateInfo InputAssemblyStateCreateInfo{};// = vkt::Pipline::InputAssemblyStateCreateInfo(pPipeline->mTopology, pPipeline->mPrimitiveRestart); 
    // InputAssemblyStateCreateInfo.topology = mState.mTopology;
    // InputAssemblyStateCreateInfo.primitiveRestartEnable = mState.mPrimitiveRestart;
    // VkPipelineMultisampleStateCreateInfo multisampling{};// = vkt::Pipline::MultisampleStateCreateInfo();
    // multisampling.sType = ;
    // multisampling.minSampleShading = 1.0f;
    // multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    VkPipelineColorBlendStateCreateInfo ColorBlendStateCreateInfo{};
    ColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    ColorBlendStateCreateInfo.attachmentCount = 1;
    ColorBlendStateCreateInfo.pAttachments = &mState.mColorBlend;
    //if(blendConstants)memcpy(info.blendConstants, blendConstants, sizeof(float) * 4);
    ColorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY;
    // ColorBlendStateCreateInfo.logicOpEnable = logicOpEnable;false
    VkPipelineViewportStateCreateInfo ViewportStateCreateInfo;
    ViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    ViewportStateCreateInfo.flags = 0;
    ViewportStateCreateInfo.pNext = nullptr;
    if(mScissor.empty() || mViewport.empty()){//不支持多个
        ViewportStateCreateInfo.scissorCount = 1;
        ViewportStateCreateInfo.viewportCount = 1;
    }
    else{
        ViewportStateCreateInfo.scissorCount = mScissor.size();
        ViewportStateCreateInfo.viewportCount = mViewport.size();
    }
    // ViewportStateCreateInfo.scissorCount = mScissorCount;
    // ViewportStateCreateInfo.viewportCount = mViewportCount;
    ViewportStateCreateInfo.pScissors = mScissor.data();
    ViewportStateCreateInfo.pViewports = mViewport.data();
    VkPipelineDynamicStateCreateInfo *pDynamicState = nullptr;
    if(!ViewportStateCreateInfo.pScissors || !ViewportStateCreateInfo.pViewports){//即使启用动态视口,viewportCount和scissorCount也不能为0
        mDynamics.push_back(VK_DYNAMIC_STATE_SCISSOR);
        mDynamics.push_back(VK_DYNAMIC_STATE_VIEWPORT);
    }
    if(!mDynamics.empty()) {
        pDynamicState = new VkPipelineDynamicStateCreateInfo;
        VkPipelineDynamicStateCreateInfo dynamicState;
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.flags = 0;
        dynamicState.pNext = nullptr;
        dynamicState.pDynamicStates = mDynamics.data();
        dynamicState.dynamicStateCount = mDynamics.size();
        memcpy(pDynamicState, &dynamicState, sizeof(VkPipelineDynamicStateCreateInfo));
    }
    // RasterizationStateCreateInfo.lineWidth = 1.0f;
    // RasterizationStateCreateInfo.cullMode = mState.mCullMode;
    // RasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    // RasterizationStateCreateInfo.polygonMode = mState.mPolygonMode;
    // RasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
    // RasterizationStateCreateInfo.depthClampEnable = mState.mDepthClampEnable;//如果设置为true,则会将超出近平面和远平面的片段截断为近平面和远平面上，而不是直接丢弃。阴影贴图可能需要用到
    // RasterizationStateCreateInfo.rasterizerDiscardEnable =  VK_FALSE;//如果设置为true，则不会输出任何图形
    VkGraphicsPipelineCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    info.pStages = ShaderStageCreateInfo.data();
    info.stageCount = ShaderStageCreateInfo.size();
    info.pVertexInputState = &VertexInputStateCreateInfo;
    info.pInputAssemblyState = &mState.mInputAssembly;
    info.pViewportState = &ViewportStateCreateInfo;
    info.pDynamicState = pDynamicState;
    if(mState.mTessellation.patchControlPoints > 0)info.pTessellationState = &mState.mTessellation;
    info.layout = mLayout;
    info.renderPass = renderPass;
    info.pColorBlendState = &ColorBlendStateCreateInfo;
    info.pDepthStencilState = &mState.mDepthStencil;
    info.pMultisampleState = &mState.mMultisample;
    info.pRasterizationState = &mState.mRasterization;
    VkResult ret = vkCreateGraphicsPipelines(device, cache, 1, &info, nullptr, &mPipeline);
    if(pDynamicState)delete pDynamicState;
    for (auto it = ShaderBegin(); it != ShaderEnd(); ++it) {
        vkDestroyShaderModule(device, it->mModule, nullptr);
    }
    return ret;
    // if(pDynamicState)delete pDynamicState;
}
//void GraphicsPipeline::IncreaseShader(VkDevice device, const std::vector<uint32_t>&code){
//    auto it = AddShader(device, code);
//    if(it->mStage == VK_SHADER_STAGE_VERTEX_BIT){//测试得知。stage input确实不是顶点着色器独有
//        spirv_cross::CompilerGLSL glsl(code);
//        spirv_cross::ShaderResources resource = glsl.get_shader_resources();
//        spirv_cross::SmallVector<spirv_cross::Resource>stageInput = resource.stage_inputs;
//        VkFormat vertexFormat[] = { VK_FORMAT_UNDEFINED, VK_FORMAT_R32_SFLOAT, VK_FORMAT_R32G32_SFLOAT, VK_FORMAT_R32G32B32_SFLOAT, VK_FORMAT_R32G32B32A32_SFLOAT };
//        for (int i = stageInput.size() - 1; i >= 0; --i){
//            uint32_t size = GetVecSize(glsl, stageInput[i]);
//            uint32_t uiLocation = GetLocation(glsl, stageInput[i]);
//            VkVertexInputAttributeDescription vertexInput;
//            vertexInput.binding = 0;
//            vertexInput.format = vertexFormat[size];
//            vertexInput.location = uiLocation;
//            vertexInput.offset = stride;
//            VertexInputAttributeDescription vertexInputAttributeDescription;
//            vertexInputAttributeDescription.mName = stageInput[i].name;
//            vertexInputAttributeDescription.mAttributeDescriptions = vertexInput;
//            mVertexInput.mVertexInputAttributeDescription.push_back(vertexInputAttributeDescription);
//        }
//        mVertexInput.mBindingDescription.binding = 0;
//        mVertexInput.mBindingDescription.stride = stride;
//        mVertexInput.mBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
//    }
//}
// void GraphicsPipeline::BindDescriptorSets(VkCommandBuffer cmd, const VkDescriptorSet *pDescriptorSets, uint32_t descriptorSetCount){
//     BindDescriptorSets(cmd, 0, pDescriptorSets, descriptorSetCount);
// }
// void GraphicsPipeline::BindDescriptorSets(VkCommandBuffer cmd, uint32_t uiSet, const VkDescriptorSet *pDescriptorSets, uint32_t descriptorSetCount){
//     vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, mLayout, uiSet, descriptorSetCount, pDescriptorSets, 0, nullptr);
// }
// void GraphicsPipeline::PushConstant(VkCommandBuffer cmd){
//     for (size_t uiShader = 0; uiShader < mShaders.size(); ++uiShader){
        
//     }
// }
// void GraphicsPipeline::PushConstant(VkCommandBuffer cmd, const std::vector<glm::mat4>&data){

// }