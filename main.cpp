#include <stdio.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#ifdef __linux
#include <unistd.h>
#include <dirent.h>
#endif

#include "Split.h"

#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
enum DIRECTION{
    UP_DIRECTION = 0,
    DOWN_DIRECTION,
    LEFT_DIRECTION,
    RIGHT_DIRECTION
};
VulkanPool g_VulkanPool;
VulkanQueue g_VulkanQueue;
VulkanDevice g_VulkanDevice;
VulkanWindows g_VulkanWindows;
VulkanSynchronize g_VulkanSynchronize;
VkDebugUtilsMessengerEXT g_VulkanMessenger;

const uint32_t g_WindowWidth = 800, g_WindowHeight = g_WindowWidth;

VkCommandBuffer g_CommandBuffer;

bool g_ShowOpenFileUI;
void (*g_OpenFileFunCall)(const std::string&file);
std::vector<const char *>g_FileTypeItem;

Split g_Split;
std::string g_ImageName;
// void createSurface(VkInstance instance, VkSurfaceKHR&surface, void* userData){
//     glfwCreateWindowSurface(instance, (GLFWwindow *)userData, nullptr, &surface);
// }
void splicingDirectory(const std::vector<std::string>&subDir, std::string&path){
    std::string newPath;
    for (size_t i = 0; i < subDir.size(); ++i){
        newPath += subDir[i];
    }
    path = newPath;
}
void getSubdirectory(const std::string&path, std::vector<std::string>&subDir){
    //得到从0到'/'的字符串。并将字符移动到‘/’之外。然后重复
    // int index = 0;
#ifdef __linux
    char cSeparator = '/';
#endif
#ifdef WIN32
    char cSeparator = '\\';
#endif
    std::string sPath = path;
    if(sPath[sPath.size() - 1] != cSeparator){
            sPath += cSeparator;
    }
    const char *ptrStart = sPath.c_str();
    const char *ptrEnd = nullptr;
    while(ptrEnd = strchr(ptrStart, cSeparator)){
            int size = ptrEnd - ptrStart;
            subDir.push_back(std::string(ptrStart, size + 1));
            ptrStart += size + 1;
    }
}
void getFileFromDirectory(const std::string&path, std::vector<std::string>&folder, std::vector<std::string>&file){
#ifdef __linux
    DIR *d;
    dirent *dFile = NULL;
    if(!(d = opendir(path.c_str()))){
        perror("opendir error");
        printf("path is %s\n", path.c_str());
        return;
    }
    while((dFile = readdir(d))){
        if(strcmp(dFile->d_name, ".") && strcmp(dFile->d_name, "..")){
            if(dFile->d_type == DT_DIR){
                folder.push_back(dFile->d_name);
            }
            else{
                file.push_back(dFile->d_name);
            }
        }
    }
    closedir(d);
#endif
}
//只有点击了取消或打开按钮才会返回false
bool ShowOpenFileUI(const char *const *items, int items_count, std::string&result){
    bool continueShow = true;
    if(!ImGui::Begin("打开"/*, nullptr, ImGuiWindowFlags_NoResize*/)){
        ImGui::End();
        return true;
    }
    static int fileTypeIndex = items_count - 1;
    std::vector<std::string> subDir;
    static std::string selectedFile;
#ifdef __linux
    static std::string currentDir = get_current_dir_name();
#endif
#ifdef WIN32
    static std::string currentDir;
#endif
    if(ImGui::BeginTable("表_对齐", 2)){
    //         char r[MAXBYTE] = { "./Show" };
        ImGui::TableNextColumn();
        if(ImGui::BeginTable("常用路径", 1)){
            ImGui::TableSetupColumn("主目录");
            ImGui::TableHeadersRow();

            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            // ImGui::TextUnformatted(r);
            // ImGui::Button(r);
#ifdef __linux
            std::string homePaht = getenv("HOME");
            std::vector<std::string>recentPath = { homePaht, homePaht + "/Desktop", homePaht + "/Documents", homePaht + "/Downloads", homePaht + "/Music", homePaht + "/Pictures", homePaht + "/Videos" };
#endif
#ifdef WIN32
            std::vector<std::string>recentPath;
#endif
            for (size_t i = 0; i < recentPath.size(); ++i){
                if(ImGui::Selectable(recentPath[i].c_str())){
                    currentDir = recentPath[i];

                    ImGui::EndTable();
                    ImGui::EndTable();
                    ImGui::End();
                    return true;
                }
            }
            ImGui::EndTable();
        }
        ImGui::TableNextColumn();
        getSubdirectory(currentDir, subDir);
        if(ImGui::BeginTable("各个文件路径名", subDir.size())){
            for (size_t i = 0; i < subDir.size(); ++i){
                ImGui::TableSetupColumn(subDir[i].c_str());
            }
            ImGui::TableHeadersRow();
            ImGui::EndTable();
        }
        if(ImGui::Begin("文件名窗口", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_ChildWindow)){
            ImGui::SetWindowSize(ImVec2(400, 200));
            if(ImGui::BeginTable("文件名", 1)){
                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                std::vector<std::string>file;
                std::vector<std::string>folder;
                getFileFromDirectory(currentDir, folder, file);
                // ImGui::TextUnformatted(r);
                for (size_t i = 0; i < folder.size(); ++i){
                    if(ImGui::Selectable((folder[i] + '/').c_str())){
                        subDir.push_back(folder[i]);
                        splicingDirectory(subDir, currentDir);

                        ImGui::EndTable();
                        ImGui::EndChild();
                        ImGui::EndTable();
                        ImGui::End();
                        return true;
                    }
                }
                if(fileTypeIndex){//指定栏类型
                    for (size_t i = 0; i < file.size(); ++i){
                        std::size_t pos = file[i].find('.');
                        std::size_t tPos = file[i].find('.', pos + 1);
                        while(tPos < file[i].length()){
                            pos = tPos;
                            tPos = file[i].find('.', tPos + 1);
                        }
                        if(pos < file[i].length()){
                            std::string s = file[i].substr(pos);
                            if(s == items[fileTypeIndex] + 1){
                                if(ImGui::Selectable(file[i].c_str())){
                                    selectedFile = currentDir + '/' + file[i];
                                }
                            }
                        }
                    }
                }
                else{
                    for (size_t i = 0; i < file.size(); ++i){
                        if(ImGui::Selectable(file[i].c_str())){
                            selectedFile = currentDir + '/' + file[i];
                        }
                    }
                }
                ImGui::EndTable();
            }
            ImGui::EndChild();
        }
        ImGui::EndTable();
    }
    ImVec2 windowSize = ImGui::GetWindowSize();
    ImGui::Text(selectedFile.c_str());
    ImGui::SetCursorPos(ImVec2(0, windowSize.y - 70));
    if(ImGui::BeginTable("返回上级_类型_表", 2)){
        ImGui::TableNextColumn();
        if(ImGui::Button("返回上级")){
            if(subDir.size() > 1){
                subDir.pop_back();
                splicingDirectory(subDir, currentDir);
            }
        }
        ImGui::TableNextColumn();ImGui::Combo("类型", &fileTypeIndex, items, items_count);
        ImGui::EndTable();
    }
    ImGui::SetCursorPos(ImVec2(0, windowSize.y - 40));
    if(ImGui::Button("取消")){
        selectedFile = "";
        ImGui::End();
        return false;
    }
    ImGui::SetCursorPos(ImVec2(40, windowSize.y - 40));
    if(ImGui::Button("打开")){
        if(selectedFile != ""){
            result = selectedFile;
            selectedFile = "";
            ImGui::End();
            return false;
        }
    }
    ImGui::End();
    return true;
}
void OpenImage(const std::string &file){
    g_ImageName = file;
    g_Split.ChangeTextureImage(g_VulkanDevice.device, file, g_WindowWidth, g_WindowHeight, g_VulkanQueue.graphics, g_VulkanPool.commandPool);
    g_Split.UpdateUniform(g_VulkanDevice.device, g_WindowWidth, g_WindowHeight);
    g_Split.UpdateDescriptorSet(g_VulkanDevice.device);
    // VkExtent2D source, size;
    // void *data = g_Split.LoadTextureImage(g_VulkanDevice.device, file, source, g_VulkanQueue.graphics, g_VulkanPool.commandPool);
    // if(data){
    //     g_Split.SplitTextureImage(g_VulkanDevice.device, data, source, g_VulkanQueue.graphics, g_VulkanPool.commandPool);
    //     g_Split.FreeTextureImage(data);
    //     size = source;
    //     const float zoom = .8f;
    //     while (size.width > g_WindowWidth || size.height > g_WindowHeight){
    //         size.width *= zoom;
    //         size.height *= zoom;
    //     }
    //     g_Split.CreateTextureImage(g_VulkanDevice.device, size.width, size.height, g_VulkanQueue.graphics, g_VulkanPool.commandPool);
    //     g_Split.UpdateUniform(g_VulkanDevice.device, g_WindowWidth, g_WindowHeight);
    //     g_Split.UpdateDescriptorSet(g_VulkanDevice.device);
    // }
}
/*{{{*/
// float g_DeltaTime = 0.0f;
// float g_LastFrame = 0.0f;
// float camX;
// float camY;
// float camZ;
// glm::vec3 rotateAxis = glm::vec3(0, -1, 0);
void updateImguiWidget(){
    // static bool checkbuttonstatu;//检查框的状态。这个值传给imgui会影响到检查框
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    
    // static double test = 0;
    // ImGui::DragInt("拖动条", &test);//确实可以拖动。但不是类似一条线中间有个圆的样子
    if(ImGui::BeginMainMenuBar()){
        if(ImGui::BeginMenu("文件")){
            if(ImGui::MenuItem("打开")){
                // bool g_ShowOpenFileUI;
                // void (*g_OpenFileFunCall)(const std::string&file);
                // std::vector<const char *>g_FileTypeItem;
                g_ShowOpenFileUI = true;
                g_OpenFileFunCall = OpenImage;
                g_FileTypeItem.push_back("*.*");
                g_FileTypeItem.push_back("*.gif");
                g_FileTypeItem.push_back("*.bmp");
                g_FileTypeItem.push_back("*.jpg");
                g_FileTypeItem.push_back("*.png");
            }
            if(ImGui::MenuItem("保存")){
            }
            if(ImGui::MenuItem("另存为")){
            }
            // if(ImGui::BeginMenu("新建")){
            //     if(ImGui::MenuItem("打开")){

            //     }
            //     ImGui::EndMenu();
            // }
            // if(ImGui::MenuItem("退出")){
            //     // glfwSetWindowShouldClose()
            // }
            ImGui::EndMenu();
        }
        if(ImGui::BeginMenu("编辑")){
            if(ImGui::MenuItem("重置")){
                // g_Split.ResetImageIndex();
                OpenImage(g_ImageName);
                g_Split.UpdateUniform(g_VulkanDevice.device, g_WindowWidth, g_WindowHeight);
            }
            ImGui::EndMenu();
        }
        // if(ImGui::BeginMenu("摄像机")){
        //     if(ImGui::BeginMenu("视角")){
        //         if(ImGui::MenuItem("重置")){
        //             g_CameraPos = g_Pos;
        //             g_Up = glm::vec3(0, 1, 0);
        //             g_RotateBase = glm::mat4(1.0f);
        //             UpdateUniform(g_VulkanDevice.device);
        //         }
        //         ImGui::EndMenu();
        //     }
        //     ImGui::EndMenu();
        // }
        ImGui::EndMainMenuBar();
    }
    static int rowAndcolumn[2] = { 3, 3 };
    static float backgroundColor[3] = { 1, 1, 1 };
    if(ImGui::Begin("九宫格")){
        if(ImGui::InputInt2("行列", rowAndcolumn)){
            g_Split.SetRow(rowAndcolumn[0]);
            g_Split.SetColumn(rowAndcolumn[1]);
            OpenImage(g_ImageName);
        }
        if(ImGui::ColorEdit3("背景色", backgroundColor)){
            g_Split.ChangeBackgroundColor(glm::vec3(backgroundColor[0], backgroundColor[1], backgroundColor[2]));
        }
        ImGui::End();
    }
    if(g_ShowOpenFileUI){
        std::string file = "";
        g_ShowOpenFileUI = ShowOpenFileUI(g_FileTypeItem.data(), g_FileTypeItem.size(), file);
        if(file != "" && g_OpenFileFunCall)g_OpenFileFunCall(file);
    }
    ImGui::Render();
    ImDrawData *draw_data = ImGui::GetDrawData();

    const bool isMinimized = (draw_data->DisplaySize.x <=.0f || draw_data->DisplaySize.y <= .0f);
    if(!isMinimized)ImGui_ImplVulkan_RenderDrawData(draw_data, g_CommandBuffer);
}
VkBool32 VKAPI_PTR debugUtilsMessenger(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData){
    const char* strMessageSeverity = nullptr;//, *strMessageTypes = nullptr;
    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
            strMessageSeverity = "VERBOSE";
    }
    else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
            strMessageSeverity = "INFO";
    }
    else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
            strMessageSeverity = "WARNING";
    }
    else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
            strMessageSeverity = "ERROR";
    }
    else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT) {
            strMessageSeverity = "FLAG";
    }
    printf("[VULKAN VALIDATION LAYER]\nSEVERITY:%s\nMESSAGE:%s\n", strMessageSeverity, pCallbackData->pMessage);
    return VK_FALSE;
}
void setupVulkan(GLFWwindow *window){
    uint32_t count;
    const char** instanceExtension = glfwGetRequiredInstanceExtensions(&count);
    std::vector<const char*> extensions(instanceExtension, instanceExtension + count);
    VK_CHECK(vkf::CreateInstance(extensions, g_VulkanDevice.instance));
    g_VulkanDevice.physicalDevice = vkf::GetPhysicalDevices(g_VulkanDevice.instance, VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU);
    vkf::CreateDebugUtilsMessenger(g_VulkanDevice.instance, g_VulkanMessenger, debugUtilsMessenger);
    glfwCreateWindowSurface(g_VulkanDevice.instance, window, nullptr, &g_VulkanWindows.surface);
    vkf::CreateDevice(g_VulkanDevice.physicalDevice, {}, g_VulkanWindows.surface, g_VulkanDevice.device);
    int queueFamilies[2];
    vkf::tool::GetGraphicAndPresentQueueFamiliesIndex(g_VulkanDevice.physicalDevice, g_VulkanWindows.surface, queueFamilies);
    vkGetDeviceQueue(g_VulkanDevice.device, queueFamilies[0], 0, &g_VulkanQueue.graphics);
    vkGetDeviceQueue(g_VulkanDevice.device, queueFamilies[1], 0, &g_VulkanQueue.present);
    vkf::CreateSwapchain(g_VulkanDevice.physicalDevice, g_VulkanDevice.device, g_VulkanWindows.surface, g_VulkanWindows.swapchain);
    vkf::CreateRenderPassForSwapchain(g_VulkanDevice.device, g_VulkanWindows.renderpass);
    vkf::CreateCommandPool(g_VulkanDevice.physicalDevice, g_VulkanDevice.device, g_VulkanPool.commandPool, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    vkf::CreateFrameBufferForSwapchain(g_VulkanDevice.device, { g_WindowWidth, g_WindowHeight }, g_VulkanWindows);
    vkf::CreateSemaphoreAndFenceForSwapchain(g_VulkanDevice.device, g_VulkanWindows.swapchainImageViews.size(), g_VulkanSynchronize);
    vkf::CreateDescriptorPool(g_VulkanDevice.device, 3, g_VulkanPool.descriptorPool);
    //显示设备信息
    const char *deviceType;
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(g_VulkanDevice.physicalDevice, &physicalDeviceProperties);
    switch (physicalDeviceProperties.deviceType){
    case VK_PHYSICAL_DEVICE_TYPE_CPU:
        deviceType = "CPU";
        break;
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
        deviceType = "DISCRETE GPU";
        break;
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
        deviceType = "INTEGRATED GPU";
        break;
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
        deviceType = "VIRTUAL GPU";
        break;
    default:
        deviceType = "OTHER";
        break;
    }
	printf("gpu name:%s, gpu type:%s\n", physicalDeviceProperties.deviceName, deviceType);
}
void cleanupVulkan(){
    for (size_t i = 0; i < g_VulkanSynchronize.fences.size(); ++i){
        vkDestroyFence(g_VulkanDevice.device, g_VulkanSynchronize.fences[i], nullptr);
        vkDestroySemaphore(g_VulkanDevice.device, g_VulkanSynchronize.imageAcquired[i], nullptr);
        vkDestroySemaphore(g_VulkanDevice.device, g_VulkanSynchronize.renderComplete[i], nullptr);
    }
    for (size_t i = 0; i < g_VulkanWindows.framebuffers.size(); ++i){
        vkDestroyImageView(g_VulkanDevice.device, g_VulkanWindows.swapchainImageViews[i], nullptr);
        vkDestroyFramebuffer(g_VulkanDevice.device, g_VulkanWindows.framebuffers[i], nullptr);
    }
    g_VulkanWindows.depthImage.Destroy(g_VulkanDevice.device);
    
    vkDestroyCommandPool(g_VulkanDevice.device, g_VulkanPool.commandPool, nullptr);
    vkDestroyDescriptorPool(g_VulkanDevice.device, g_VulkanPool.descriptorPool, nullptr);

    vkDestroyRenderPass(g_VulkanDevice.device, g_VulkanWindows.renderpass, nullptr);
    vkDestroySwapchainKHR(g_VulkanDevice.device, g_VulkanWindows.swapchain, nullptr);

    vkDestroySurfaceKHR(g_VulkanDevice.instance, g_VulkanWindows.surface, nullptr);

    vkf::DestoryDebugUtilsMessenger(g_VulkanDevice.instance, g_VulkanMessenger);

    vkDestroyDevice(g_VulkanDevice.device, nullptr);
    vkDestroyInstance(g_VulkanDevice.instance, nullptr);
}

//action `GLFW_PRESS`, `GLFW_RELEASE` or `GLFW_REPEAT`.  Future
void keyboard(GLFWwindow *window, int key, int scancode, int action, int mods){

}
bool g_LeftDown;
uint32_t g_SelectImageIndex;
void mousecursorpos(GLFWwindow *window, double xpos, double ypos){
    uint32_t index;
    const glm::vec3 size = g_Split.GetImageSize() * glm::vec3(1.05, 1.05, 1);
    if(g_LeftDown){
        if(g_SelectImageIndex != -1 && g_Split.mousecursor(xpos, ypos, index)){
            g_Split.UpdateTexture(g_VulkanDevice.device, g_SelectImageIndex, glm::vec3(xpos - size.x * .5, ypos - size.y * .5, 0), size);
        }
    }
    else{
        if(g_Split.mousecursor(xpos, ypos, index)){
            const glm::vec3 pos = g_Split.GetImagePos(index);
            if(g_SelectImageIndex != -1){
                // g_Split.SwapImage(g_SelectImageIndex, index);
                g_Split.SwapImage(g_VulkanDevice.device, g_VulkanQueue.graphics, g_VulkanPool.commandPool, g_SelectImageIndex, index);
                g_Split.UpdateUniform(g_VulkanDevice.device, g_WindowWidth, g_WindowHeight);
                g_SelectImageIndex = -1;
            }
            else{
                g_Split.UpdateUniform(g_VulkanDevice.device, g_WindowWidth, g_WindowHeight);
                g_Split.UpdateTexture(g_VulkanDevice.device, index, pos, size);
            }
        }
        else{
            g_Split.UpdateUniform(g_VulkanDevice.device, g_WindowWidth, g_WindowHeight);
        }
    }
}
void mousebutton(GLFWwindow *window, int button, int action, int mods) {
    g_LeftDown = action;
    if(GLFW_MOUSE_BUTTON_LEFT == button && GLFW_PRESS == action){
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        if(!g_Split.mousecursor(xpos, ypos, g_SelectImageIndex)){
            g_SelectImageIndex = -1;
        }
    }
}
void RecordCommand(uint32_t currentFrame){
    vkf::tool::BeginCommands(g_CommandBuffer, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
    vkf::tool::BeginRenderPassGeneral(g_CommandBuffer, g_VulkanWindows.framebuffers[currentFrame], g_VulkanWindows.renderpass, g_WindowWidth, g_WindowHeight);
    g_Split.Draw(g_CommandBuffer, g_WindowWidth, g_WindowHeight);
    updateImguiWidget();
    vkCmdEndRenderPass(g_CommandBuffer);
    VK_CHECK(vkEndCommandBuffer(g_CommandBuffer));
}
void setup(GLFWwindow *windows){
    glfwSetKeyCallback(windows, keyboard);
    glfwSetMouseButtonCallback(windows, mousebutton);
    glfwSetCursorPosCallback(windows, mousecursorpos);

    // vkf::CreateTextureSampler(g_VulkanDevice.device, g_TextureSampler);

    g_Split.Setup(g_VulkanDevice.physicalDevice, g_VulkanDevice.device, g_VulkanQueue.graphics, g_VulkanPool);
    g_Split.CreateGraphicsPipeline(g_VulkanDevice.device, g_VulkanWindows.renderpass, g_WindowWidth, g_WindowHeight);
    g_Split.UpdateUniform(g_VulkanDevice.device, g_WindowWidth, g_WindowHeight);

    //imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO&io = ImGui::GetIO();
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;//启用手柄    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForVulkan(windows, true);

    int queueFamilies;
    vkf::tool::GetGraphicAndPresentQueueFamiliesIndex(g_VulkanDevice.physicalDevice, VK_NULL_HANDLE, &queueFamilies);
    ImGui_ImplVulkan_InitInfo initInfo = {};
    initInfo.Allocator = nullptr;
    initInfo.CheckVkResultFn = nullptr;
    initInfo.QueueFamily = queueFamilies;
    initInfo.PipelineCache = VK_NULL_HANDLE;
    initInfo.Device = g_VulkanDevice.device;
    initInfo.Queue = g_VulkanQueue.graphics;
    initInfo.Instance = g_VulkanDevice.instance;
    initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    initInfo.DescriptorPool = g_VulkanPool.descriptorPool;
    initInfo.PhysicalDevice = g_VulkanDevice.physicalDevice;
    initInfo.ImageCount = g_VulkanWindows.framebuffers.size();
    initInfo.MinImageCount = g_VulkanWindows.swapchainImageViews.size();
    ImGui_ImplVulkan_Init(&initInfo, g_VulkanWindows.renderpass);

    io.Fonts->AddFontFromFileTTF("fonts/SourceHanSerifCN-Bold.otf", 20, NULL, io.Fonts->GetGlyphRangesChineseFull());

    VkCommandBuffer cmd;
    vkf::tool::BeginSingleTimeCommands(g_VulkanDevice.device, g_VulkanPool.commandPool, cmd);
    ImGui_ImplVulkan_CreateFontsTexture(cmd);
    vkf::tool::EndSingleTimeCommands(g_VulkanDevice.device, g_VulkanPool.commandPool, g_VulkanQueue.graphics, cmd);
    ImGui_ImplVulkan_DestroyFontUploadObjects();

    vkf::tool::AllocateCommandBuffers(g_VulkanDevice.device, g_VulkanPool.commandPool, 1, &g_CommandBuffer);
}
void cleanup(){
    vkDeviceWaitIdle(g_VulkanDevice.device);

    g_Split.DestroyGraphicsPipeline(g_VulkanDevice.device);
    g_Split.Cleanup(g_VulkanDevice.device);

    // vkDestroyDescriptorSetLayout(g_VulkanDevice.device, g_SetLayout, nullptr);
    // DestroyGraphicsPipeline(g_GraphicsPipeline);

    // g_Texture.Destroy(g_VulkanDevice.device);

    // g_Index.Destroy(g_VulkanDevice.device);
    // g_Vertex.Destroy(g_VulkanDevice.device);
    // g_Position.Destroy(g_VulkanDevice.device);

}
void display(GLFWwindow* window){
    static size_t currentFrame;
    vkDeviceWaitIdle(g_VulkanDevice.device);

    RecordCommand(currentFrame);
    vkf::DrawFrame(g_VulkanDevice.device, currentFrame, g_CommandBuffer, g_VulkanWindows.swapchain, g_VulkanQueue, g_VulkanSynchronize);
    // if(GLFW_PRESS == glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT)){
    //     int id = 0;
    //     double xpos, ypos;
    //     glfwGetCursorPos(window, &xpos, &ypos);
    //     if(id = ((PickingTexture *)g_Base)->ReadPixel(g_VulkanDevice.device, g_VulkanQueue.graphics, g_VulkanPool.commandPool, xpos, ypos)){
    //         if(id != 0){
    //             printf("time:%f:选中模型, id = %d\n", glfwGetTime(), id);
    //         }
    //     }
    //     else{
    //         printf("time:%f:未选中模型, id = %d\n", glfwGetTime(), id);
    //     }
    // }
    currentFrame = (currentFrame + 1) % g_VulkanWindows.framebuffers.size();
}
int main(){
    if (GLFW_FALSE == glfwInit()) {
        printf("initialize glfw error");
        return 1;
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(g_WindowWidth, g_WindowHeight, "demo", NULL, NULL);
    // Setup Vulkan
    if (!glfwVulkanSupported()){
        printf("GLFW: Vulkan Not Supported\n");
        return 1;
    }
    setupVulkan(window);
    setup(window);
    while (!glfwWindowShouldClose(window)) {
        display(window);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
	glfwTerminate();

    cleanup();
    cleanupVulkan();
    return 0;
}
