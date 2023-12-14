#include "Split.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
void copy(const stbi_uc *source, stbi_uc *destination, uint32_t row, uint32_t column, const VkExtent2D& sourceImage, const VkExtent2D& destinationImage) {
    //如果要截取第2行的图片，需要跳过 row * 目标图片的高度 * 原图宽度
    uint32_t uiLineSize = destinationImage.width * 4;
    uint32_t uiSourceLineSize = sourceImage.width * 4;
    uint32_t imageOffsetStart = row * destinationImage.height * uiSourceLineSize + column * uiLineSize;
    for (size_t dataOffset = 0; dataOffset < destinationImage.height; ++dataOffset) {
        memcpy(destination + dataOffset * uiLineSize, source + imageOffsetStart + dataOffset * uiSourceLineSize, uiLineSize);
    }
}

void Split::UpdateImage(VkDevice device, uint32_t windowWidth, uint32_t windowHeight){
    const uint32_t offset = 10;
    const glm::vec3 backgroundSize = glm::vec3((mColumn + 1) * offset + mColumn * mImageSize.width, (mRow + 1) * offset + mRow * mImageSize.height, 1);
    const glm::vec3 backgroundPos = glm::vec3(windowWidth * .5 - backgroundSize.x * .5, windowHeight * .5 - backgroundSize.y * .5, 0);
    mImagePos.resize(mRow * mColumn);
    for (uint32_t uiRow = 0; uiRow < mRow; ++uiRow){
        for (uint32_t uiColumn = 0; uiColumn < mColumn; ++uiColumn){
            uint32_t index = ROW_COLUMN_INDEX(uiRow, uiColumn, mColumn);
            mImagePos[index] = glm::vec3(backgroundPos.x + uiColumn * mImageSize.width + (uiColumn + 1) * offset, backgroundPos.y + uiRow * mImageSize.height + (uiRow + 1) * offset, 0);
            UpdateTexture(device, index, mImagePos[index], glm::vec2(mImageSize.width, mImageSize.height));
            // mImagePos[mImageIndex[index]] = glm::vec3(backgroundPos.x + uiColumn * mImageSize.width + (uiColumn + 1) * offset, backgroundPos.y + uiRow * mImageSize.height + (uiRow + 1) * offset, 0);
            // UpdateTexture(device, mImageIndex[index], mImagePos[mImageIndex[index]], glm::vec2(mImageSize.width, mImageSize.height));
        }
    }
}

Split::Split(/* args */){
}

Split::~Split(){

}

void Split::Cleanup(VkDevice device){
    VulkanSplit::Cleanup(device);
    for (size_t i = 0; i < mImageDatas.size(); ++i){
        delete[]mImageDatas[i];
    }
}

void Split::Draw(VkCommandBuffer command, uint32_t windowWidth, uint32_t windowHeight){
    VulkanSplit::Draw(command, windowWidth, windowHeight, mBackgroundColor);
}

void Split::UpdateUniform(VkDevice device, uint32_t windowWidth, uint32_t windowHeight){
    UpdateBackground(device, mRow, mColumn, windowWidth, windowHeight);
    UpdateImage(device, windowWidth, windowHeight);
}

void Split::Setup(VkPhysicalDevice physicalDevice, VkDevice device, VkQueue graphics, const VulkanPool &pool){
    // ChangeTextureImage(device, "1.jpg", graphics, pool.commandPool);
    VulkanSplit::Setup(physicalDevice, device, graphics, pool, mRow * mColumn);
}
// void *Split::LoadTextureImage(VkDevice device, const std::string &image, VkExtent2D &size, VkQueue graphics, VkCommandPool pool){
//     int nrComponents;
//     stbi_uc* data = stbi_load(image.c_str(), (int *)&size.width, (int *)&size.height, &nrComponents, STBI_rgb_alpha);
//     if(data == nullptr){
//         printf("load picture error:%s\n", image.c_str());
//         return nullptr;
//     }
//     return data;
// }

// void Split::SplitTextureImage(VkDevice device, const void *data, const VkExtent2D &size, VkQueue graphics, VkCommandPool pool){
//     VkExtent2D destination;
//     mImageRealSize = size;
//     destination.height = size.height / mRow;
//     destination.width = size.width / mColumn;
//     for (size_t i = 0; i < mImageDatas.size(); ++i){
//         delete[]mImageDatas[i];
//     }
//     mImageDatas.resize(mRow * mColumn);
//     const uint32_t imageSize = destination.width * destination.height * 4;
//     for (uint32_t uiRow = 0; uiRow < mRow; ++uiRow){
//         for (uint32_t uiColumn = 0; uiColumn < mColumn; ++uiColumn){
//             const uint32_t index = ROW_COLUMN_INDEX(uiRow, uiColumn, mColumn);
//             mImageDatas[index] = new stbi_uc[imageSize];
//             copy((const stbi_uc *)data, mImageDatas[index], uiRow, uiColumn, size, destination);
//         }
//     }
// }

// void Split::CreateTextureImage(VkDevice device, uint32_t width, uint32_t height, VkQueue graphics, VkCommandPool pool){
//     VkExtent2D destination;
//     // stbi_uc * data = (stbi_uc *)LoadTextureImage(device, image, source, graphics, pool);
//     destination.height = height / mRow;
//     destination.width = width / mColumn;
//     // SplitTextureImage(device, data, source, graphics, pool);
//     // FreeTextureImage((void *)data);
//     ChangeTextureImage(device, (const void **)mImageDatas.data(), mImageDatas.size(), destination.width, destination.height, graphics, pool);
// }

bool Split::mousecursor(double xpos, double ypos, uint32_t &index){
    for (size_t i = 0; i < mImagePos.size(); ++i){
        // if(xpos > mImagePos[mImageIndex[i]].x && ypos > mImagePos[mImageIndex[i]].y && xpos < mImagePos[mImageIndex[i]].x + mImageSize.width && ypos < mImagePos[mImageIndex[i]].y + mImageSize.height){
        if(xpos > mImagePos[i].x && ypos > mImagePos[i].y && xpos < mImagePos[i].x + mImageSize.width && ypos < mImagePos[i].y + mImageSize.height){
            index = i;
            // index = mImageIndex[i];
            return true;
        }
    }
    return false;
}
void Split::ChangeTextureImage(VkDevice device, const std::string &image, uint32_t windowWidth, uint32_t windowHeight, VkQueue graphics, VkCommandPool pool){
    int nrComponents;
    VkExtent2D source, destination;
    stbi_uc* data = stbi_load(image.c_str(), (int *)&source.width, (int *)&source.height, &nrComponents, STBI_rgb_alpha);
    if(data == nullptr){
        printf("load picture error:%s\n", image.c_str());
        return;
    }
    mImageRealSize = source;
    destination.height = source.height / mRow;
    destination.width = source.width / mColumn;
    for (size_t i = 0; i < mImageDatas.size(); ++i){
        delete[]mImageDatas[i];
    }
    mImageDatas.resize(mRow * mColumn);
    const uint32_t imageSize = destination.width * destination.height * 4;
    for (uint32_t uiRow = 0; uiRow < mRow; ++uiRow){
        for (uint32_t uiColumn = 0; uiColumn < mColumn; ++uiColumn){
            const uint32_t index = ROW_COLUMN_INDEX(uiRow, uiColumn, mColumn);
            mImageDatas[index] = new stbi_uc[imageSize];
            copy(data, mImageDatas[index], uiRow, uiColumn, source, destination);
        }
    }
    stbi_image_free(data);
    VulkanSplit::ChangeTextureImage(device, (const void **)mImageDatas.data(), mImageDatas.size(), destination.width, destination.height, graphics, pool);
    const float zoom = .8f;
    while (source.width > windowWidth || source.height > windowHeight){
        source.width *= zoom;
        source.height *= zoom;
    }
    mImageSize.height = source.height / mRow;
    mImageSize.width = source.width / mColumn;
}
// void Split::FreeTextureImage(void *data){
//     stbi_image_free(data);
// }
// void Split::SwapImage(uint32_t sourceIndex, uint32_t destIndex)
// {
//     // uint32_t index = mImageIndex[sourceIndex];
//     // mImageIndex[sourceIndex] = mImageIndex[destIndex];
//     // mImageIndex[destIndex] = index;
// }
void Split::SwapImage(VkDevice device, VkQueue graphics, VkCommandPool pool, uint32_t sourceIndex, uint32_t destIndex){
    const uint32_t imageSize = mImageSize.width * mImageSize.height * 4;
    stbi_uc *data = new stbi_uc[imageSize];
    memcpy(data, mImageDatas[sourceIndex], imageSize);
    memcpy(mImageDatas[sourceIndex], mImageDatas[destIndex], imageSize);
    memcpy(mImageDatas[destIndex], data, imageSize);
    delete[]data;
    VulkanSplit::ChangeTextureImage(device, (const void **)mImageDatas.data(), mImageDatas.size(), mImageSize.width, mImageSize.height, graphics, pool);
}