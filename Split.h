#ifndef SPLIT_INCLUDE_H
#define SPLIT_INCLUDE_H
#include "VulkanSplit.h"
class Split:public VulkanSplit{
    uint32_t mRow = 3, mColumn = 3;
    std::vector<glm::vec3>mImagePos;
    std::vector<unsigned char *>mImageDatas;
    glm::vec3 mBackgroundColor = glm::vec3(1, 1, 1);
    void UpdateImage(VkDevice device, uint32_t windowWidth, uint32_t windowHeight);
public:
    Split(/* args */);
    ~Split();
    inline void SetRow(uint32_t row){
        mRow = row;
    }
    inline void SetColumn(uint32_t column){
        mColumn = column;
    }
    inline void ChangeBackgroundColor(const glm::vec3&color){
        mBackgroundColor = color;
    }
    inline glm::vec3 GetImagePos(uint32_t index){
        return mImagePos[index];
    }
    inline glm::vec3 GetImageSize(){
        glm::vec3 size;
        size.z = 1;
        size.x = mImageSize.width;
        size.y = mImageSize.height;
        return size;
    }
    inline void ResetImageIndex(){
        VulkanSplit::ResetImageIndex(mRow * mColumn);
    }
    void Cleanup(VkDevice device);
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