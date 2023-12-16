#ifndef SPLIT_INCLUDE_H
#define SPLIT_INCLUDE_H
#include "VulkanSplit.h"
enum SPLIT_TYPE{
    JIGSAW = 0,
    AAIGNED
};
class SplitImage:public VulkanSplit{
    struct{
        glm::vec3 pos;
        glm::vec3 size;
        glm::vec3 color = glm::vec3(1, 1, 1);
    }background;
    struct{
        VkExtent2D size;
        std::string type;
        VkExtent2D realSize;
        std::vector<unsigned char *>datas;
    }images;
    uint32_t mRow, mColumn;
    SPLIT_TYPE mSplitType = AAIGNED;
    std::vector<glm::vec3>mImagePos;
    std::vector<glm::vec3>mRealImagePos;
    void UpdateImage(VkDevice device);
    void InitImagePos(uint32_t windowWidth, uint32_t windowHeight);
public:
    SplitImage(/* args */);
    ~SplitImage();
    inline void SetRow(uint32_t row){
        mRow = row;
    }
    inline void SetColumn(uint32_t column){
        mColumn = column;
    }
    inline void ChangeBackgroundColor(const glm::vec3&color){
        background.color = color;
    }
    inline glm::vec3 GetImagePos(uint32_t index){
        return mImagePos[index];
    }
    inline void SetSplitType(SPLIT_TYPE type){
        mSplitType = type;
    }
    inline glm::vec3 GetImageSize(){
        glm::vec3 size;
        size.z = 1;
        size.x = images.size.width;
        size.y = images.size.height;
        return size;
    }
    inline bool IsLoadTexture(){
        return !images.datas.empty();
    }
    // inline void ResetImageIndex(){
    //     VulkanSplit::ResetImageIndex(mRow * mColumn);
    // }
    // inline void *GetImageData(uint32_t index){
    //     return images.datas[index];
    // }
    void Cleanup(VkDevice device);

    void WriteImageToFolder(const std::string&file);
    void WriteImageToFile(VkPhysicalDevice physicalDevice, VkDevice device, VkQueue graphics, VkCommandPool pool, const std::string&file);
    
    void Draw(VkCommandBuffer command, uint32_t windowWidth, uint32_t windowHeight);
    void UpdateUniform(VkDevice device, uint32_t windowWidth, uint32_t windowHeight);
    void Setup(VkPhysicalDevice physicalDevice, VkDevice device, VkQueue graphics, const VulkanPool &pool);
    // void CreateTextureImage(VkDevice device, uint32_t width, uint32_t height, VkQueue graphics, VkCommandPool pool);
    // void *LoadTextureImage(VkDevice device, const std::string&image, VkExtent2D&size, VkQueue graphics, VkCommandPool pool);
    // void SplitTextureImage(VkDevice device, const void *data, const VkExtent2D&size, VkQueue graphics, VkCommandPool pool);
    void ChangeTextureImage(VkDevice device, const std::string &image, uint32_t windowWidth, uint32_t windowHeight, VkQueue graphics, VkCommandPool pool);

    // void FreeTextureImage(void *data);
    // void SwapImage(uint32_t sourceIndex, uint32_t destIndex);
    bool mousecursor(double xpos, double ypos, uint32_t&index);
    void SwapImage(VkDevice device, VkQueue graphics, VkCommandPool pool, uint32_t sourceIndex, uint32_t destIndex);
};
#endif