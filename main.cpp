#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#ifdef __linux
#include <unistd.h>
#include <dirent.h>
#endif
#ifdef WIN32
#include <Windows.h>
#endif // WIN32
#include <iostream>
#include <algorithm>

#include "Split.h"

#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
enum DIRECTION{
    UP_DIRECTION = 0,
    DOWN_DIRECTION,
    LEFT_DIRECTION,
    RIGHT_DIRECTION
};
struct IMGUI_WINDOW_INFO{
    ImVec2 pos;
    ImVec2 size;
};
VulkanPool g_VulkanPool;
VulkanQueue g_VulkanQueue;
VulkanDevice g_VulkanDevice;
VulkanWindows g_VulkanWindows;
VulkanSynchronize g_VulkanSynchronize;
VkDebugUtilsMessengerEXT g_VulkanMessenger;

const uint32_t g_WindowWidth = 800, g_WindowHeight = g_WindowWidth;

VkCommandBuffer g_CommandBuffer;

bool g_ShowOpenFileUI, g_ShowOpenFolderUI;
void (*g_OpenFileFunCall)(const std::string&file);
std::vector<const char *>g_FileTypeItem;

struct{
    bool reset;
    bool change;
    bool update;
    bool complete;
    bool screenhot;
    // SPLIT_TYPE splitType = SPLIT_TYPE_UNKNOW;
}g_ImageOperate;
SplitImage g_Split;
bool g_ShowMessageBox;
int32_t g_SelectImage = -1;
IMGUI_WINDOW_INFO g_ImguiInfo;
// static std::vector<int32_t>g_SelectImage;
std::string g_ImageName, g_SaveImageName, g_WindowName = "九宫格";
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
#ifdef WIN32
    HANDLE hListFile;
    CHAR szFilePath[MAX_PATH];
    WIN32_FIND_DATA FindFileData;

    lstrcpyA(szFilePath, path.c_str());

    lstrcatA(szFilePath, "\\*");
    hListFile = FindFirstFile(szFilePath, &FindFileData);
    //判断句柄
    if (hListFile == INVALID_HANDLE_VALUE){
        printf("错误： %d\n", GetLastError());
        return;
    }
    else{
        do{
            if(lstrcmp(FindFileData.cFileName,TEXT("."))==0 || lstrcmp(FindFileData.cFileName,TEXT(".."))==0){
                continue;
            }
            //打印文件名、目录名
            //printf("%s\t\t", FindFileData.cFileName);
            //判断文件属性，是否为目录
            if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
                folder.push_back(FindFileData.cFileName);
            }
            else {
                file.push_back(FindFileData.cFileName);
            }
        } while (FindNextFile(hListFile, &FindFileData));
    }
#endif // WIN32
}
bool ShowHomeDirectory(std::string&currentDir){
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
        std::string homePaht = getenv("USERPROFILE");
        std::vector<std::string>recentPath = { homePaht, homePaht + "\\Desktop", homePaht + "\\Documents", homePaht + "\\Downloads", homePaht + "\\Music", homePaht + "\\Pictures", homePaht + "\\Videos" };
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
    return false;
}
void ShowFolderDirectory(const std::string&currentDir, std::vector<std::string>&subDir){
    getSubdirectory(currentDir, subDir);
    if(ImGui::BeginTable("各个文件路径名", subDir.size())){
        for (size_t i = 0; i < subDir.size(); ++i){
            ImGui::TableSetupColumn(subDir[i].c_str());
        }
        ImGui::TableHeadersRow();
        ImGui::EndTable();
    }
}
bool ShowFolderAndFile(const char *const *items, int fileTypeIndex, std::string&currentDir, std::vector<std::string>&subDir, std::string&selectedFile){
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
#ifdef __linux
                if(ImGui::Selectable((folder[i] + '/').c_str())){
#endif
#ifdef WIN32
                if (ImGui::Selectable((folder[i] + '\\').c_str())) {
#endif // WIN32

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
#ifdef __linux                        
                        selectedFile = currentDir + '/' + file[i];
#endif
#ifdef WIN32
                        selectedFile = currentDir + '\\' + file[i];
#endif
                    }
                }
            }
            ImGui::EndTable();
        }
        ImGui::EndChild();
    }
    return false;
}
bool ShowOpenFolderUI(const char *const *items, int items_count, std::string&result){
    // bool continueShow = true;
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
    char directory[MAX_PATH] = { 0 };
    GetCurrentDirectory(sizeof(directory), directory);
    static std::string currentDir = directory;
#endif
    if(ImGui::BeginTable("表_对齐", 2)){
        ImGui::TableNextColumn();
        if(ShowHomeDirectory(currentDir)){
            return true;
        }
        ImGui::TableNextColumn();
        ShowFolderDirectory(currentDir, subDir);
        if(ShowFolderAndFile(items, fileTypeIndex, currentDir, subDir, selectedFile)){
            return true;
        }
        ImGui::EndTable();
    }
    ImVec2 windowSize = ImGui::GetWindowSize();    
    static char fileName[1000] = "/";
    if((selectedFile == "" || selectedFile.c_str() != currentDir) && !strchr(selectedFile.c_str(), '.'))
    // if(selectedFile == "" || strcmp(selectedFile.c_str(), fileName))
        selectedFile = currentDir;
    memset(fileName, 0, sizeof(fileName));
    memcpy(fileName, selectedFile.c_str(), sizeof(char) * selectedFile.size());
    if(ImGui::InputText("选择保存的文件或文件夹", fileName, 999)){
        selectedFile = fileName;
    }
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
        memset(fileName, 0, sizeof(fileName));
        selectedFile = "";
        ImGui::End();
        return false;
    }
    ImGui::SetCursorPos(ImVec2(40, windowSize.y - 40));
    if(ImGui::Button("保存")){
        if(selectedFile != ""){
            result = selectedFile;
            memset(fileName, 0, sizeof(fileName));
            selectedFile = "";
            ImGui::End();
            return false;
        }
        else{
            result = currentDir;
            memset(fileName, 0, sizeof(fileName));
            selectedFile = "";
            ImGui::End();
            return false;
        }
    }
    ImGui::End();
    return true;
}
//只有点击了取消或打开按钮才会返回false
bool ShowOpenFileUI(const char *const *items, int items_count, std::string&result){
    // bool continueShow = true;
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
    char directory[MAX_PATH] = { 0 };
    GetCurrentDirectory(sizeof(directory), directory);
    static std::string currentDir = directory;
#endif
    if(ImGui::BeginTable("表_对齐", 2)){
        ImGui::TableNextColumn();
        if(ShowHomeDirectory(currentDir)){
            return true;
        }
        ImGui::TableNextColumn();
        ShowFolderDirectory(currentDir, subDir);
        if(ShowFolderAndFile(items, fileTypeIndex, currentDir, subDir, selectedFile)){
            return true;
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
    g_ImageOperate.change = true;
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
void SaveImage(const std::string &file){
    if(g_ImageOperate.complete){
        g_SaveImageName = file;
        g_ImageOperate.screenhot = true;
    }
    else{
        g_Split.WriteImageToFolder(file);
    }
}
bool MessageBox(const std::string&message, const std::string&title){
    if(ImGui::Begin(title.c_str())){
        ImGui::Text(message.c_str());
        if(ImGui::Button("确定")){
            ImGui::End();
            return false;
        }
        ImGui::End();
    }
    return true;
}
void DeselectedImage(uint32_t imageIndex){
    g_SelectImage = -1;
    // g_SelectImage[imageIndex] = -1;
    VkExtent2D grid = g_Split.GetGridSize();
    const uint32_t offset = g_Split.GetOffset();
    const uint32_t increaseRowAColumn = g_Split.GetIncreaseRowAndColumn();
    if(increaseRowAColumn > 1 && imageIndex == g_Split.GetIncreaseIndex()){
        grid.height = grid.height * increaseRowAColumn + (increaseRowAColumn - 1) * offset;
        grid.width = grid.width * increaseRowAColumn + (increaseRowAColumn - 1) * offset;
    }
    g_Split.UpdateTexture(g_VulkanDevice.device, imageIndex, grid);
}
//交换图片后，合并结果不匹配
// void SwapImage(uint32_t sourceIndex, uint32_t destIndex){
//     g_SelectImage = -1;
//     const uint32_t index = g_Split.GetIncreaseIndex();
//     if(sourceIndex == index || destIndex == index){
//         DeselectedImage(destIndex);
//         DeselectedImage(sourceIndex);
//         return;
//     }
//     g_Split.SwapImage(g_VulkanDevice.device, sourceIndex, destIndex, g_VulkanQueue.graphics, g_VulkanPool.commandPool);
//     // g_Split.UpdateBackground(g_VulkanDevice.device, g_WindowWidth, g_WindowHeight);
//     g_Split.UpdateImage(g_VulkanDevice.device);
//     g_Split.UpdateDescriptorSet(g_VulkanDevice.device);
// }
void SelectedImage(uint32_t imageIndex){
    VkExtent2D grid = g_Split.GetGridSize();
    const uint32_t offset = g_Split.GetOffset();
    const uint32_t increaseRowAColumn = g_Split.GetIncreaseRowAndColumn();
    if(increaseRowAColumn > 1 && imageIndex == g_Split.GetIncreaseIndex()){
        grid.height = grid.height * increaseRowAColumn + (increaseRowAColumn - 1) * offset;
        grid.width = grid.width * increaseRowAColumn + (increaseRowAColumn - 1) * offset;
    }
    grid.width *= 1.05f;
    grid.height *= 1.05f;
    // g_Split.UpdateBackground(g_VulkanDevice.device, g_WindowWidth, g_WindowHeight);
    g_Split.UpdateTexture(g_VulkanDevice.device, imageIndex, grid);
}
// void SelectAll(){
//     g_SelectImage.resize(g_Split.GetRow() * g_Split.GetColumn());
//     for (size_t i = 0; i < g_SelectImage.size(); ++i){
//         SelectedImage(i);
//         g_SelectImage[i] = i;
//     }
// }
void DeselectedAll(){
    g_SelectImage = -1;
    // g_SelectImage.clear();
    // g_Split.UpdateBackground(g_VulkanDevice.device, g_WindowWidth, g_WindowHeight);
    g_Split.UpdateImageUniform(g_VulkanDevice.device);
}
// void ReverseSelectAll(){
//     const std::vector<int32_t>index = g_SelectImage;
//     SelectAll();
//     for (size_t i = 0; i < index.size(); ++i){
//         if(index[i] != -1){
//             DeselectedImage(index[i]);
//         }
//     }
// }
/*{{{*/
void updateImguiWidget(){
    // static bool checkbuttonstatu;//检查框的状态。这个值传给imgui会影响到检查框
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    static int32_t increaseRowAndColumn = g_Split.GetIncreaseRowAndColumn();
    static int32_t rowAndcolumn[2] = { (int32_t)g_Split.GetRow(), (int32_t)g_Split.GetColumn() };
    static std::string messageboxTitle = "提示", messageboxMessage;
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
            if(ImGui::BeginMenu("保存")){
                if(ImGui::MenuItem("散图")){
                    g_ImageOperate.complete = false;
                    g_ShowOpenFolderUI = true;
                    g_OpenFileFunCall = SaveImage;
                    g_FileTypeItem.push_back("*.*");
                }
                if(ImGui::MenuItem("完整")){
                    g_ImageOperate.complete = true;
                    g_ShowOpenFolderUI = true;
                    g_OpenFileFunCall = SaveImage;
                    g_FileTypeItem.push_back("*.*");
                    g_FileTypeItem.push_back("*.gif");
                    g_FileTypeItem.push_back("*.bmp");
                    g_FileTypeItem.push_back("*.jpg");
                    g_FileTypeItem.push_back("*.png");
                }
                ImGui::EndMenu();
            }
            // if(ImGui::BeginMenu("另存为")){
            //     if(ImGui::MenuItem("散图")){
            //         g_ImageOperate.complete = false;
            //         g_ShowOpenFolderUI = true;
            //         g_OpenFileFunCall = SaveImage;
            //         g_FileTypeItem.push_back("*.*");
            //     }
            //     if(ImGui::MenuItem("完整")){
            //         g_ImageOperate.complete = true;
            //         g_ShowOpenFolderUI = true;
            //         g_OpenFileFunCall = SaveImage;
            //         g_FileTypeItem.push_back("*.*");
            //         g_FileTypeItem.push_back("*.gif");
            //         g_FileTypeItem.push_back("*.bmp");
            //         g_FileTypeItem.push_back("*.jpg");
            //         g_FileTypeItem.push_back("*.png");
                    
            //     }
            //     ImGui::EndMenu();
            // }
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
        // if(ImGui::BeginMenu("编辑")){
        //     //不能交换图片的话，这个功能就没意义了
        //     // if(ImGui::MenuItem("重置")){
        //     //     g_ImageOperate.reset = true;
        //     // }
        //     if(ImGui::MenuItem("导入")){
        //         //导入的肯定是小图。因为要导入大图的话，应该从文件->打开那里导入
        //     }
        //     ImGui::EndMenu();
        // }
        ImGui::EndMainMenuBar();
    }
    static float backgroundColor[3] = { 1, 1, 1 };
    if(ImGui::Begin(g_WindowName.c_str())){
        g_ImguiInfo.pos = ImGui::GetWindowPos();
        g_ImguiInfo.size = ImGui::GetWindowSize();
        //最多放大一张图片
        if(g_Split.IsLoadTexture()){
            if(ImGui::InputInt("扩大图片", &increaseRowAndColumn)){
                //输入完后，仍然会一直执行, 直到点一下其他地方
                if(g_SelectImage != -1){
                    g_Split.IncreaseImage(g_SelectImage, increaseRowAndColumn);
                    OpenImage(g_ImageName);
                    //g_ImageOperate.update = true;
                }
                else{
                    increaseRowAndColumn = 1;
                }
            }
            if(ImGui::InputInt2("图片行列", rowAndcolumn)){
                if(rowAndcolumn[0] != 0 && rowAndcolumn[1] != 0){
                    g_Split.SetRow(rowAndcolumn[0]);
                    g_Split.SetColumn(rowAndcolumn[1]);
                    OpenImage(g_ImageName);
                    //g_ImageOperate.update = true;
                }
                else{
                    rowAndcolumn[0] = 1;
                    rowAndcolumn[1] = 1;
                }
            }
        }
        if(ImGui::ColorEdit3("背景颜色", backgroundColor)){
            g_Split.ChangeBackgroundColor(glm::vec3(backgroundColor[0], backgroundColor[1], backgroundColor[2]));
            g_Split.UpdateBackground(g_VulkanDevice.device, g_WindowWidth, g_WindowHeight);
        }
        //虽然散图无法不带特效。但可以先输出完整图片，再从完整图片截图....
        //完整图片是按真实图片大小输出的
        if(g_Split.IsLoadTexture()){
            static float cartoons = g_Split.GetCartoons();
            static float degree[] = { g_Split.GetDegree().x, g_Split.GetDegree().y, g_Split.GetDegree().z };
            if(ImGui::SliderFloat("卡通化因子", &cartoons, 0, 20)){
                g_Split.SetCartoons(cartoons);
                g_Split.UpdateImageUniform(g_VulkanDevice.device);
            }
            if(ImGui::InputFloat3("卡通化程度", degree)){
                g_Split.SetDegree(glm::vec3(degree[0], degree[1], degree[2]));
                g_Split.UpdateImageUniform(g_VulkanDevice.device);
            }
        }
        if(ImGui::Button("注意事项")){
            messageboxMessage = "扩图:扩大的行列必须一样\n交换:扩大后的图片无法和小图交换";
            g_ShowMessageBox = true;
        }
        // for (size_t i = 0; i < g_SelectImage.size(); ++i){
        //     if(g_SelectImage[i] != -1){
        //         static float angle = g_Split.GetAngle(g_SelectImage[i]);
        //         char lable[MAX_BYTE] = { 0 };
        //         sprintf(lable, "第%d张图片的旋转角度", g_SelectImage[i] + 1);
        //         if(ImGui::SliderFloat(lable, &angle, 0, 360)){
        //             g_Split.SetAngle(g_SelectImage[i], angle);
        //             g_Split.UpdateTexture(g_VulkanDevice.device, g_SelectImage[i], g_Split.GetGridSize());
        //         }
        //     }   
        // }
        //因为特效是直接往所有图片加, 所以选择图片用处不大
        // if(ImGui::BeginTable("表_对齐", 3)){
        //     ImGui::TableNextColumn();
        //     if(ImGui::Button("全选")){
        //         SelectAll();
        //     }
        //     ImGui::TableNextColumn();
        //     if(ImGui::Button("取消全选")){
        //         DeselectedAll();
        //     }
        //     ImGui::TableNextColumn();
        //     if(ImGui::Button("反向全选")){
        //         ReverseSelectAll();
        //     }
        //     ImGui::EndTable();
        // }
        ImGui::End();
    }
    if(g_ShowOpenFileUI){
        std::string file = "";
        g_ShowOpenFileUI = ShowOpenFileUI(g_FileTypeItem.data(), g_FileTypeItem.size(), file);
        if(file != "" && g_OpenFileFunCall){
            g_OpenFileFunCall(file);
            g_FileTypeItem.clear();
        }
    }
    if(g_ShowOpenFolderUI){
        std::string file = "";
        g_ShowOpenFolderUI = ShowOpenFolderUI(g_FileTypeItem.data(), g_FileTypeItem.size(), file);
        if(file != "" && g_OpenFileFunCall){
            g_OpenFileFunCall(file);
            g_FileTypeItem.clear();
        }
    }
    if(g_ShowMessageBox){
        g_ShowMessageBox = MessageBox(messageboxMessage, messageboxTitle);
        if(!g_ShowMessageBox)messageboxMessage = "";
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
#ifdef WIN32
    g_VulkanDevice.physicalDevice = vkf::GetPhysicalDevices(g_VulkanDevice.instance);
#else
    g_VulkanDevice.physicalDevice = vkf::GetPhysicalDevices(g_VulkanDevice.instance, VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU);
#endif // WIN32

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
    vkf::CreateDescriptorPool(g_VulkanDevice.device, 11, g_VulkanPool.descriptorPool);
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
    for (size_t i = 0; i < g_VulkanSynchronize.imageAcquired.size(); ++i){
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
bool g_PressS, g_PressControl;
//action `GLFW_PRESS`, `GLFW_RELEASE` or `GLFW_REPEAT`.  Future
void keyboard(GLFWwindow *window, int key, int scancode, int action, int mods){
    if(action == GLFW_PRESS && g_Split.IsLoadTexture()){
        if(key == GLFW_KEY_S){
            g_PressS = true;
        }
        if(key == GLFW_KEY_LEFT_CONTROL || key == GLFW_KEY_RIGHT_CONTROL){
            g_PressControl = true;
        }
        //保存的是完整图片。保存散图必须通过:打开->保存->散图
        if(g_PressControl && g_PressS){
            g_ImageOperate.complete = true;
            g_ShowOpenFolderUI = true;
            g_OpenFileFunCall = SaveImage;
            g_FileTypeItem.push_back("*.*");
            g_FileTypeItem.push_back("*.gif");
            g_FileTypeItem.push_back("*.bmp");
            g_FileTypeItem.push_back("*.jpg");
            g_FileTypeItem.push_back("*.png");
            g_PressS = false;
            g_PressControl = false;
        }
    }
}

// bool g_LeftDown;
// uint32_t g_SelectImageIndex;
// void mousecursorpos(GLFWwindow *window, double xpos, double ypos){
//     if(g_ShowOpenFileUI || g_ShowOpenFolderUI)return;
//     // uint32_t index;
//     // VkExtent2D grid = g_Split.GetGridSize();
//     // grid.width *= 1.05f;
//     // grid.height *= 1.05f;
//     // if(g_LeftDown){
//     //     if(g_SelectImageIndex != -1){
//     //         g_Split.UpdateTexture(g_VulkanDevice.device, g_SelectImageIndex, glm::vec2(xpos - grid.width * .5, ypos - grid.height * .5), grid);
//     //     }
//     // }
//     // else{
//     //     if(g_Split.MouseCursor(xpos, ypos, index)){
//     //         const glm::vec2 pos = g_Split.GetImagePos(index);
//     //         if(g_SelectImageIndex != -1 && g_SelectImageIndex != index){
//     //             // g_Split.SwapImage(g_SelectImageIndex, index);
//     //             g_Split.SwapImage(g_VulkanDevice.device, g_VulkanQueue.graphics, g_VulkanPool.commandPool, g_SelectImageIndex, index);
//     //             g_Split.UpdateDescriptorSet(g_VulkanDevice.device);
//     //             // g_Split.RerodOffscreenCommand();
//     //             g_Split.UpdateUniform(g_VulkanDevice.device, g_WindowWidth, g_WindowHeight);
//     //             g_SelectImageIndex = -1;
//     //         }
//     //         else{
//     //             g_SelectImageIndex = -1;
//     //             g_Split.UpdateUniform(g_VulkanDevice.device, g_WindowWidth, g_WindowHeight);
//     //             g_Split.UpdateTexture(g_VulkanDevice.device, index, pos, grid);
//     //         }
//     //     }
//     //     else{
//     //         g_SelectImageIndex = -1;
//     //         g_Split.UpdateUniform(g_VulkanDevice.device, g_WindowWidth, g_WindowHeight);
//     //     }
//     // }
// }
// bool CanSwapImage(const std::vector<int32_t>&index){
//     uint32_t count = 0;
//     for (size_t i = 0; i < index.size(); ++i){
//         if(index[i] != -1){
//             ++count;
//         }
//     }
//     return count == 1;
// }
// int32_t IsSelected(const std::vector<int32_t>&index, int32_t imageIndex){
//     int32_t iSelected = -1;
//     auto it = std::find(index.begin(), index.end(), imageIndex);
//     if(it != index.end()){
//         iSelected = *it;
//     }
//     // for (size_t i = 0; i < index.size(); ++i){
//     //     if(index[i] == imageIndex){
//     //         iSelected = i;
//     //         break;
//     //     }
//     // }
//     return iSelected;
// }
void mousebutton(GLFWwindow *window, int button, int action, int mods) {
    if(g_ShowOpenFileUI || g_ShowOpenFolderUI || g_ShowMessageBox)return;
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    if(xpos > g_ImguiInfo.pos.x && xpos < g_ImguiInfo.pos.x + g_ImguiInfo.size.x && ypos > g_ImguiInfo.pos.y && ypos < g_ImguiInfo.pos.y + g_ImguiInfo.size.y)return;
    if(GLFW_MOUSE_BUTTON_LEFT == button && GLFW_PRESS == action){
        if(g_SelectImage == -1){
            if(g_Split.MouseCursor(xpos, ypos, g_SelectImage)){
                SelectedImage(g_SelectImage);
            }
        }
        else{
            int32_t index;
            if(g_Split.MouseCursor(xpos, ypos, index)){
                if(g_SelectImage == index){
                    DeselectedImage(g_SelectImage);
                }
                // else{
                //     SwapImage(g_SelectImage, index);
                //     g_SelectImage = -1;
                // }
            }
        }
    }
    // // g_LeftDown = action;
    // if(GLFW_MOUSE_BUTTON_LEFT == button && GLFW_PRESS == action){
    //     int32_t index;
    //     const bool bPressControl = GLFW_PRESS == glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) || GLFW_PRESS == glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL);
    //     if(g_SelectImage.empty() || bPressControl){
    //         if(g_Split.mousecursor(xpos, ypos, index)){
    //             const int32_t selectedIndex = IsSelected(g_SelectImage, index);
    //             if(g_SelectImage.empty() || selectedIndex == -1){
    //                 g_SelectImage.push_back(index);
    //             }
    //             else{
    //                 DeselectedImage(index);
    //             }
    //             for (size_t i = 0; i < g_SelectImage.size(); ++i){
    //                 if(g_SelectImage[i] != -1){
    //                     SelectedImage(g_SelectImage[i]);
    //                 }
    //             }
    //         }
    //     }
    //     else{
    //         if(CanSwapImage(g_SelectImage)){
    //             if(g_Split.mousecursor(xpos, ypos, index)){
    //                 if(index != g_SelectImage[0]){
    //                     SwapImage(g_SelectImage[0], index);
    //                 }
    //             }
    //             DeselectedAll();
    //             g_SelectImage.clear();
    //         }
    //     }
    // }
}
void RecordCommand(uint32_t currentFrame){
    vkf::tool::BeginCommands(g_CommandBuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    vkf::tool::BeginRenderPassGeneral(g_CommandBuffer, g_VulkanWindows.framebuffers[currentFrame], g_VulkanWindows.renderpass, g_WindowWidth, g_WindowHeight);
    g_Split.Draw(g_CommandBuffer, g_WindowWidth, g_WindowHeight);
    updateImguiWidget();
    vkCmdEndRenderPass(g_CommandBuffer);
#ifdef OFFSCREEN_DEBUG
    VkClearValue clearValues = {};
    clearValues.color = { 0, 0, 0, 1.0f };
    vkf::tool::BeginRenderPass(g_CommandBuffer, g_VulkanWindows.framebuffers[currentFrame], g_VulkanWindows.renderpass, g_WindowWidth * .25, g_WindowHeight * .25, 1, &clearValues, g_WindowWidth - g_WindowWidth * .25);
    g_Split.DrawDebug(g_CommandBuffer, g_WindowWidth, g_WindowHeight);
    vkCmdEndRenderPass(g_CommandBuffer);
#endif
    VK_CHECK(vkEndCommandBuffer(g_CommandBuffer));
}
void setup(GLFWwindow *windows){
    glfwSetKeyCallback(windows, keyboard);
    glfwSetMouseButtonCallback(windows, mousebutton);
    // glfwSetCursorPosCallback(windows, mousecursorpos);

    // vkf::CreateTextureSampler(g_VulkanDevice.device, g_TextureSampler);

    g_Split.Setup(g_VulkanDevice.physicalDevice, g_VulkanDevice.device, g_VulkanQueue.graphics, g_VulkanPool);
    g_Split.CreateGraphicsPipeline(g_VulkanDevice.device, g_VulkanWindows.renderpass, g_WindowWidth, g_WindowHeight);
    g_Split.UpdateBackground(g_VulkanDevice.device, g_WindowWidth, g_WindowHeight);

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

    ImGui_ImplVulkan_CreateFontsTexture(g_VulkanPool.commandPool, g_VulkanQueue.graphics);

    vkf::tool::AllocateCommandBuffers(g_VulkanDevice.device, g_VulkanPool.commandPool, 1, &g_CommandBuffer);

    g_Split.RerodOffscreenCommand(glm::vec3(1, 1, 1));
}
void cleanup(){
    vkDeviceWaitIdle(g_VulkanDevice.device);

    g_Split.DestroyGraphicsPipeline(g_VulkanDevice.device);
    g_Split.Cleanup(g_VulkanDevice.device);

    ImGui_ImplGlfw_Shutdown();
    ImGui_ImplVulkan_Shutdown();
}
void display(GLFWwindow* window){
    static size_t currentFrame;
    vkDeviceWaitIdle(g_VulkanDevice.device);
    if(g_ImageOperate.screenhot){
        g_ImageOperate.screenhot = false;
        g_Split.WriteImageToFile(g_VulkanDevice.physicalDevice, g_VulkanDevice.device, g_VulkanQueue.graphics, g_VulkanPool.commandPool, g_SaveImageName);
    }
    // if(g_ImageOperate.splitType != SPLIT_TYPE_UNKNOW){
    //     g_Split.SetSplitType(g_VulkanDevice.device, g_ImageName, g_ImageOperate.splitType, g_WindowWidth, g_WindowHeight, g_VulkanQueue.graphics, g_VulkanPool.commandPool);
    //     g_ImageOperate.splitType = SPLIT_TYPE_UNKNOW;
    // }
    if(g_ImageOperate.update){
        g_ImageOperate.update = false;
        DeselectedAll();
        g_Split.UpdateImage(g_VulkanDevice.device, g_VulkanQueue.graphics, g_VulkanPool.commandPool);
        g_Split.InitBackgroundPos(g_WindowWidth, g_WindowHeight);
        g_Split.InitGridImageInfo();
        g_Split.UpdateBackground(g_VulkanDevice.device, g_WindowWidth, g_WindowHeight);
        g_Split.UpdateDescriptorSet(g_VulkanDevice.device);
        g_Split.UpdateImageUniform(g_VulkanDevice.device);
    }
    // if(g_ImageOperate.reset){
    //     g_ImageOperate.reset = false;
    //     DeselectedAll();
    //     g_Split.ResetImage(g_VulkanDevice.device, g_VulkanQueue.graphics, g_VulkanPool.commandPool);
    //     g_Split.UpdateDescriptorSet(g_VulkanDevice.device);
    //     g_Split.UpdateBackground(g_VulkanDevice.device, g_WindowWidth, g_WindowHeight);
    //     g_Split.UpdateImage(g_VulkanDevice.device);
    // }
    if(g_ImageOperate.change){
        g_ImageOperate.change = false;
        if(g_SelectImage != -1)DeselectedImage(g_SelectImage);
        g_Split.ChangeImage(g_VulkanDevice.device, g_ImageName, g_WindowWidth, g_WindowHeight, g_VulkanQueue.graphics, g_VulkanPool.commandPool);

        g_Split.UpdateDescriptorSet(g_VulkanDevice.device);
        g_Split.UpdateBackground(g_VulkanDevice.device, g_WindowWidth, g_WindowHeight);
        g_Split.UpdateImageUniform(g_VulkanDevice.device);
        g_SelectImage = -1;
        // g_Split.IncreaseImage(0, 1, 1);
    }
    RecordCommand(currentFrame);
    // vkf::DrawFrame(g_VulkanDevice.device, currentFrame, g_CommandBuffer, g_VulkanWindows.swapchain, g_VulkanQueue, g_VulkanSynchronize);
    g_Split.DrawFrame(g_VulkanDevice.device, currentFrame, g_CommandBuffer, g_VulkanWindows.swapchain, g_VulkanQueue, g_VulkanSynchronize);
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
