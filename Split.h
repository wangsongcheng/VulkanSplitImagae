#ifndef SPLIT_INCLUDE_H
#define SPLIT_INCLUDE_H
#include "VulkanSplit.h"
enum SPLIT_TYPE{
    JIGSAW = 0,
    JIU_GONG_GE
};
class SplitImage:public VulkanSplit{
    struct{
        glm::vec3 pos;
        VkExtent2D size;
        glm::vec3 color = glm::vec3(1, 1, 1);
    }background;
    struct{
        VkExtent2D size;
        std::string type;
        std::vector<unsigned char *>datas;
    }images;
    struct{
        struct{
            float cartoons;
            glm::vec3  degree = glm::vec3(.5, .5, .5);
        }cartoon;
        //需求是图片内容旋转而不是整个正方形旋转
        // std::vector<float>angle;
    }specialEffects;
    VkExtent2D mGrid;
    uint32_t mRow, mColumn;
    SPLIT_TYPE mSplitType = JIU_GONG_GE;
    std::vector<glm::vec2>mImagePos;
    std::vector<glm::vec2>mOffscreenImagePos;
    void InitImagePos(uint32_t windowWidth, uint32_t windowHeight);
    void InitBackgroundPos(uint32_t windowWidth, uint32_t windowHeight);
    void InitJigsawPos(const glm::vec3&backgroundPos, const VkExtent2D&backgroundSize);
    void InitJiuGongGePos(const glm::vec3&backgroundPos, const VkExtent2D&backgroundSize);
public:
    SplitImage(/* args */);
    ~SplitImage();
    inline VkExtent2D GetBackgroundSize(const VkExtent2D&imageSize){
        VkExtent2D size;
        const uint32_t offset = 10;
        size.height = (mRow + 1) * offset + imageSize.height;
        size.width = (mColumn + 1) * offset + imageSize.width;
        return size;
    }
    inline uint32_t GetRow(){
        return mRow;
    }
    inline uint32_t GetColumn(){
        return mColumn;
    }
    // inline float GetAngle(uint32_t index){
    //     return specialEffects.angle[index];
    // }
    inline float GetCartoons(){
        return specialEffects.cartoon.cartoons;
    }
    inline glm::vec3 GetDegree(){
        return specialEffects.cartoon.degree;
    }
    inline void SetRow(uint32_t row){
        mRow = row;
    }
    inline void SetColumn(uint32_t column){
        mColumn = column;
    }
    inline void ChangeBackgroundColor(const glm::vec3&color){
        background.color = color;
    }
    inline glm::vec2 GetImagePos(uint32_t index){
        return mImagePos[index];
    }
    inline void SetSplitType(SPLIT_TYPE type){
        mSplitType = type;
    }
    inline VkExtent2D GetGridSize(){
        return mGrid;
    }
    inline bool IsLoadTexture(){
        return !images.datas.empty();
    }
    inline void SetDegree(const glm::vec3&degree){
        specialEffects.cartoon.degree = degree;
    }
    inline void SetCartoons(float cartoons){
        specialEffects.cartoon.cartoons = cartoons;
    }
    // inline void SetAngle(uint32_t index, float angle){
    //     specialEffects.angle[index] = angle;
    // }
    // inline void ResetImageIndex(){
    //     VulkanSplit::ResetImageIndex(mRow * mColumn);
    // }
    // inline void *GetImageData(uint32_t index){
    //     return images.datas[index];
    // }
    void Cleanup(VkDevice device);

    void UpdateImage(VkDevice device);
    
    void WriteImageToFolder(const std::string&file);
    void WriteImageToFile(VkPhysicalDevice physicalDevice, VkDevice device, VkQueue graphics, VkCommandPool pool, const std::string&file);
    
    void UpdateTexture(VkDevice device, uint32_t index, const VkExtent2D&grid);
    void Draw(VkCommandBuffer command, uint32_t windowWidth, uint32_t windowHeight);
    void UpdateBackground(VkDevice device, uint32_t windowWidth, uint32_t windowHeight);
    void Setup(VkPhysicalDevice physicalDevice, VkDevice device, VkQueue graphics, const VulkanPool &pool);
    // void CreateTextureImage(VkDevice device, uint32_t width, uint32_t height, VkQueue graphics, VkCommandPool pool);
    // void *LoadTextureImage(VkDevice device, const std::string&image, VkExtent2D&size, VkQueue graphics, VkCommandPool pool);
    // void SplitTextureImage(VkDevice device, const void *data, const VkExtent2D&size, VkQueue graphics, VkCommandPool pool);
    void InertTextureImage(VkDevice device, const std::string&image, uint32_t index, VkQueue graphics, VkCommandPool pool);
    void ChangeTextureImage(VkDevice device, const std::string &image, uint32_t windowWidth, uint32_t windowHeight, VkQueue graphics, VkCommandPool pool);

    // void FreeTextureImage(void *data);
    // void SwapImage(uint32_t sourceIndex, uint32_t destIndex);
    bool mousecursor(double xpos, double ypos, int32_t&index);
    void SwapImage(VkDevice device, uint32_t sourceIndex, uint32_t destIndex, VkQueue graphics, VkCommandPool pool);
    void SwapImage(VkDevice device, void *datsa, uint32_t width, uint32_t height, uint32_t destIndex, VkQueue graphics, VkCommandPool pool);
};
#endif