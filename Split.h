#ifndef SPLIT_INCLUDE_H
#define SPLIT_INCLUDE_H
#include "VulkanSplit.h"
// enum SPLIT_TYPE{
//     SPLIT_TYPE_JIGSAW = 0,
//     SPLIT_TYPE_JIU_GONG_GE,
//     SPLIT_TYPE_UNKNOW = 0xffffffffff
// };
struct GRID_IMAGE_INFO{
    glm::vec2 pos;
    uint32_t row, column;
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
            glm::vec3 degree;
        }cartoon;
        struct{
            // uint32_t row;
            bool increase;
            uint32_t index;
            // uint32_t column;
            uint32_t rowAndcolumn;//所占的行列数
        }increase;
        //需求是图片内容旋转而不是整个正方形旋转
        // std::vector<float>angle;
    }effects;
    // //如果不是JIU_GONG_GE忽略
    // struct{
    //     //第一张所占的行列
    //     uint32_t row;
    //     uint32_t column;
    // }firstImage;
    uint32_t mRow, mColumn;
    const uint32_t mOffset = 10;
    unsigned char *mSourceImageData;
    unsigned char *mIncreaseImageDatas;
    std::vector<GRID_IMAGE_INFO>mGridImage;
    std::vector<GRID_IMAGE_INFO>mOffscreenGridImage;
    glm::vec2 mIncreaseGridPos, mOffscreenIncreaseGridPos;
    VkExtent2D mGrid, mIncreaseGrid, mOffscreenGrid, mOffscreenIncreaseGrid;
    // SPLIT_TYPE mSplitType = SPLIT_TYPE_JIU_GONG_GE;
    void InitGridImageInfo();
    void InitBackgroundPos(uint32_t windowWidth, uint32_t windowHeight);
    uint32_t ResetImage(VkDevice device, const void *data, uint32_t width, uint32_t height, bool increase);
    //重置图片, 调用前请先读出图片数据
    // void InitJigsawPos(const glm::vec3&backgroundPos, const VkExtent2D&backgroundSize);
    // void InitJiuGongGePos(const glm::vec3&backgroundPos, const VkExtent2D&backgroundSize);
public:
    SplitImage(/* args */);
    ~SplitImage();
    inline VkExtent2D GetBackgroundSize(const VkExtent2D&imageSize){
        VkExtent2D size;
        size.height = (mRow + 1) * mOffset + imageSize.height;
        size.width = (mColumn + 1) * mOffset + imageSize.width;
        return size;
    }
    inline uint32_t GetOffset(){
        return mOffset;
    }
    inline uint32_t GetRow(){
        return mRow;
    }
    inline uint32_t GetColumn(){
        return mColumn;
    }
    inline uint32_t GetIncreaseIndex(){
        return effects.increase.index;
    }
    inline uint32_t GetIncreaseRowAndColumn(){
        return effects.increase.rowAndcolumn;
    }
    // inline uint32_t GetIncreaseColumn(){
    //     return effects.increase.column;
    // }
    // inline float GetAngle(uint32_t index){
    //     return effects.angle[index];
    // }
    inline float GetCartoons(){
        return effects.cartoon.cartoons;
    }
    inline glm::vec3 GetDegree(){
        return effects.cartoon.degree;
    }
    inline void SetRow(uint32_t row){
        mRow = row;
    }
    inline void SetColumn(uint32_t column){
        mColumn = column;
    }
    // inline void SetFirstRow(uint32_t row){
    //     if(mSplitType != SPLIT_TYPE_JIGSAW || row < mRow - 1)effects.increase.row = row;
    // }
    // inline void SetFirstColumn(uint32_t column){
    //     if(mSplitType != SPLIT_TYPE_JIGSAW || column < mColumn - 1)effects.increase.column = column;
    // }
    inline void ChangeBackgroundColor(const glm::vec3&color){
        background.color = color;
    }
    inline glm::vec2 GetImagePos(uint32_t index){
        return mGridImage[index].pos;
    }
    // inline SPLIT_TYPE GetSplitType(){
    //     return mSplitType;
    // }
    inline VkExtent2D GetGridSize(){
        return mGrid;
    }
    inline bool IsLoadTexture(){
        return !images.datas.empty();
    }
    inline void SetDegree(const glm::vec3&degree){
        effects.cartoon.degree = degree;
    }
    inline void SetCartoons(float cartoons){
        effects.cartoon.cartoons = cartoons;
    }
    inline void IncreaseImage(uint32_t index, uint32_t rowAndcolumn){
        if(index >= mRow * mColumn || rowAndcolumn == 0 || rowAndcolumn >= mRow || rowAndcolumn >= mColumn)return;
        const uint32_t uiSelectRow = effects.increase.index?effects.increase.index / mRow:0, uiSelectColumn = effects.increase.index?effects.increase.index % mColumn:0;
        if(uiSelectRow + rowAndcolumn > mRow || uiSelectColumn + rowAndcolumn > mColumn)return;
        // effects.increase.row = row;
        effects.increase.index = index;
        effects.increase.rowAndcolumn = rowAndcolumn;
        // effects.increase.column = column;
    }
    // inline void SetAngle(uint32_t index, float angle){
    //     effects.angle[index] = angle;
    // }
    // inline void ResetImageIndex(){
    //     VulkanSplit::ResetImageIndex(mRow * mColumn);
    // }
    // inline void *GetImageData(uint32_t index){
    //     return images.datas[index];
    // }
    void Cleanup(VkDevice device);

    void UpdateImageUniform(VkDevice device);
    void UpdateImage(VkDevice device, VkQueue graphics, VkCommandPool pool);
    
    void WriteImageToFolder(const std::string&file);
    void WriteImageToFile(VkPhysicalDevice physicalDevice, VkDevice device, VkQueue graphics, VkCommandPool pool, const std::string&file);
    
    void UpdateTexture(VkDevice device, uint32_t index, const VkExtent2D&grid);
    void Draw(VkCommandBuffer command, uint32_t windowWidth, uint32_t windowHeight);
    void UpdateBackground(VkDevice device, uint32_t windowWidth, uint32_t windowHeight);
    void Setup(VkPhysicalDevice physicalDevice, VkDevice device, VkQueue graphics, const VulkanPool &pool);
    // void CreateTextureImage(VkDevice device, uint32_t width, uint32_t height, VkQueue graphics, VkCommandPool pool);
    // void *LoadTextureImage(VkDevice device, const std::string&image, VkExtent2D&size, VkQueue graphics, VkCommandPool pool);
    // void SplitTextureImage(VkDevice device, const void *data, const VkExtent2D&size, VkQueue graphics, VkCommandPool pool);
    // void InertTextureImage(VkDevice device, const std::string&image, uint32_t index, VkQueue graphics, VkCommandPool pool);
    //调用前请先读出图片数据
    void ChangeImage(VkDevice device, const void *data, uint32_t width, uint32_t height, VkQueue graphics, VkCommandPool pool);
    void ChangeImage(VkDevice device, const std::string &image, uint32_t windowWidth, uint32_t windowHeight, VkQueue graphics, VkCommandPool pool);
    
    void ResetImage(VkDevice device, VkQueue graphics, VkCommandPool pool);
    // void SetSplitType(VkDevice device, const std::string &image, SPLIT_TYPE type, uint32_t windowWidth, uint32_t windowHeight, VkQueue graphics, VkCommandPool pool);

    // void FreeTextureImage(void *data);
    // void SwapImage(uint32_t sourceIndex, uint32_t destIndex);
    bool MouseCursor(double xpos, double ypos, int32_t&index);
    void SwapImage(VkDevice device, uint32_t sourceIndex, uint32_t destIndex, VkQueue graphics, VkCommandPool pool);
    void SwapImage(VkDevice device, void *datsa, uint32_t width, uint32_t height, uint32_t destIndex, VkQueue graphics, VkCommandPool pool);
};
#endif