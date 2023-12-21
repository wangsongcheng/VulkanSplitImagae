// #include <algorithm>
#include "Split.h"
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"
// void copy(const void *source, void *destination, uint32_t row, uint32_t column, const VkExtent2D& sourceImage, const VkExtent2D& destinationImage) {
void copy(const void *source, void *destination, uint32_t row, uint32_t column, const VkExtent2D& sourceImage, const VkExtent2D& destinationImage, const glm::vec2&offset) {
    //如果要截取第2行的图片，需要跳过 row * 目标图片的高度 * 原图宽度
    uint32_t uiLineSize = destinationImage.width * 4;
    uint32_t uiSourceLineSize = sourceImage.width * 4;
    
    uint32_t uiOffsetlineSize = offset.x * 4;
    uint32_t imageOffsetStart = row * offset.y * uiSourceLineSize + column * uiOffsetlineSize;
    // uint32_t imageOffsetStart = row * destinationImage.height * uiSourceLineSize + column * uiLineSize;
    for (size_t dataOffset = 0; dataOffset < destinationImage.height; ++dataOffset) {
        memcpy((stbi_uc *)destination + dataOffset * uiLineSize, (stbi_uc *)source + imageOffsetStart + dataOffset * uiSourceLineSize, uiLineSize);
    }
}
void SplitImage::UpdateImageUniform(VkDevice device){
    if(effects.increase.increase){
        VkExtent2D imageSize = mGrid;
        for (uint32_t index = 0; index < mImageCount; ++index){
            if(index != effects.increase.index)
                UpdateTexture(device, index, mGrid);
            else
                UpdateTexture(device, index, mIncreaseGrid);
        }
    }
    else{
        for (uint32_t index = 0; index < mImageCount; ++index){
            UpdateTexture(device, index, mGrid);
        }
    }
}
void SplitImage::UpdateImage(VkDevice device, VkQueue graphics, VkCommandPool pool){
    ChangeImage(device, mSourceImageData, images.size.width, images.size.height, graphics, pool);
    InitGridImageInfo();
}
void SplitImage::InitGridImageInfo(){
    uint32_t index = 0;
    mOffscreenGrid.height = images.size.height / mRow;
    mOffscreenGrid.width = images.size.width / mColumn;
    mIncreaseGrid.height = mGrid.height * effects.increase.rowAndcolumn + (effects.increase.rowAndcolumn - 1) * mOffset;
    mIncreaseGrid.width = mGrid.width * effects.increase.rowAndcolumn + (effects.increase.rowAndcolumn - 1) * mOffset;
    mOffscreenIncreaseGrid.height = mOffscreenGrid.height * effects.increase.rowAndcolumn + (effects.increase.rowAndcolumn - 1) * mOffset;
    mOffscreenIncreaseGrid.width = mOffscreenGrid.width * effects.increase.rowAndcolumn + (effects.increase.rowAndcolumn - 1) * mOffset;

    const uint32_t uiSelectRow = effects.increase.index?effects.increase.index / mRow:0, uiSelectColumn = effects.increase.index?effects.increase.index % mColumn:0;
    mIncreaseGridPos.y = background.pos.y + uiSelectRow * mGrid.height + (uiSelectRow + 1) * mOffset;
    mIncreaseGridPos.x = background.pos.x + uiSelectColumn * mGrid.width + (uiSelectColumn + 1) * mOffset;
    mOffscreenIncreaseGridPos.y = uiSelectRow * mOffscreenGrid.height + (uiSelectRow + 1) * mOffset;
    mOffscreenIncreaseGridPos.x = uiSelectColumn * mOffscreenGrid.width + (uiSelectColumn + 1) * mOffset;
    for (size_t i = 0; i < mGridImage.size(); ++i){
        // const uint32_t uiRow = i?i / mRow:0, uiColumn = i?i % mColumn:0;
        mGridImage[i].pos = glm::vec2(background.pos.x + mGridImage[i].column * mGrid.width + (mGridImage[i].column + 1) * mOffset, background.pos.y + mGridImage[i].row * mGrid.height + (mGridImage[i].row + 1) * mOffset);
        mOffscreenGridImage[i].pos = glm::vec2(mOffscreenGridImage[i].column * mOffscreenGrid.width + (mOffscreenGridImage[i].column + 1) * mOffset, mOffscreenGridImage[i].row * mOffscreenGrid.height + (mOffscreenGridImage[i].row + 1) * mOffset);
        // printf("i = %d, row = %d, column = %d, grid pos:%f, %f\n", i, mGridImage[i].row, mGridImage[i].column, mGridImage[i].pos.x, mGridImage[i].pos.y);
    }
}
void SplitImage::InitBackgroundPos(uint32_t windowWidth, uint32_t windowHeight){
    VkExtent2D imageSize;
    imageSize.height = mRow * mGrid.height;
    imageSize.width = mColumn * mGrid.width;
    background.size = GetBackgroundSize(imageSize);
    background.pos = glm::vec3(windowWidth * .5 - background.size.width * .5, windowHeight * .5 - background.size.height * .5, 0);
}
// void SplitImage::InitJigsawPos(const glm::vec3&backgroundPos, const VkExtent2D&backgroundSize){
//     // uint32_t index = 0;
//     //如果可以，不要这样直接改, 而是通过修改选定图片的大小
//     // if(mSplitType == SPLIT_TYPE_JIGSAW){
//     //     mImagePos.resize(mRow * mColumn - firstImage.row * firstImage.column + 1);
//     //     mOffscreenImagePos.resize(mRow * mColumn - firstImage.row * firstImage.column + 1);
//     //     offscreenGridSize.height = images.size.height / mRow;
//     //     offscreenGridSize.width = images.size.width / mColumn;
//     //     mOffscreenImagePos[0] = glm::vec2(offset, offset);
//     //     mImagePos[0] = glm::vec2(background.pos.x + offset, background.pos.y + offset);
//     //     for (uint32_t uiRow = 0; uiRow < mRow; ++uiRow){
//     //         for (uint32_t uiColumn = uiRow < firstImage.column?mColumn - firstImage.column + 1:0; uiColumn < mColumn; ++uiColumn){
//     //             ++index;
//     //             mOffscreenImagePos[index] = glm::vec2(uiColumn * offscreenGridSize.width + (uiColumn + 1) * offset, uiRow * offscreenGridSize.height + (uiRow + 1) * offset);
//     //             mImagePos[index] = glm::vec2(background.pos.x + uiColumn * mGrid.width + (uiColumn + 1) * offset, background.pos.y + uiRow * mGrid.height + (uiRow + 1) * offset);
//     //         }
//     //     }
//     // }
//     // else if(mSplitType == SPLIT_TYPE_JIU_GONG_GE){
//         mImagePos.resize(mRow * mColumn);
//         mOffscreenImagePos.resize(mRow * mColumn);
//         offscreenGridSize.height = images.size.height / mRow;
//         offscreenGridSize.width = images.size.width / mColumn;
//         for (uint32_t uiRow = 0; uiRow < mRow; ++uiRow){
//             for (uint32_t uiColumn = 0; uiColumn < mColumn; ++uiColumn){
//                 const uint32_t index = ROW_COLUMN_INDEX(uiRow, uiColumn, mColumn);
//                 mOffscreenImagePos[index] = glm::vec2(uiColumn * offscreenGridSize.width + (uiColumn + 1) * offset, uiRow * offscreenGridSize.height + (uiRow + 1) * offset);
//                 mImagePos[index] = glm::vec2(background.pos.x + uiColumn * mGrid.width + (uiColumn + 1) * offset, background.pos.y + uiRow * mGrid.height + (uiRow + 1) * offset);
//             }
//         }
//     // }
// }
// void SplitImage::InitJiuGongGePos(const glm::vec3&backgroundPos, const VkExtent2D&backgroundSize){
//     const uint32_t offset = 10;
//     VkExtent2D offscreenGridSize;
//     mImagePos.resize(mRow * mColumn);
//     mOffscreenImagePos.resize(mRow * mColumn);
//     offscreenGridSize.height = images.size.height / mRow;
//     offscreenGridSize.width = images.size.width / mColumn;
//     for (uint32_t uiRow = 0; uiRow < mRow; ++uiRow){
//         for (uint32_t uiColumn = 0; uiColumn < mColumn; ++uiColumn){
//             const uint32_t index = ROW_COLUMN_INDEX(uiRow, uiColumn, mColumn);
//             mOffscreenImagePos[index] = glm::vec2(uiColumn * offscreenGridSize.width + (uiColumn + 1) * offset, uiRow * offscreenGridSize.height + (uiRow + 1) * offset);
//             mImagePos[index] = glm::vec2(background.pos.x + uiColumn * mGrid.width + (uiColumn + 1) * offset, background.pos.y + uiRow * mGrid.height + (uiRow + 1) * offset);
//         }
//     }
// }
SplitImage::SplitImage(/* args */){
}

SplitImage::~SplitImage(){

}

void SplitImage::Cleanup(VkDevice device){
    VulkanSplit::Cleanup(device);
    delete[]mSourceImageData;
    delete[]mIncreaseImageDatas;
    for (size_t i = 0; i < images.datas.size(); ++i){
        delete[]images.datas[i];
    }
}

void SplitImage::WriteImageToFolder(const std::string&file){
    VkExtent2D imageSize;
    char fileName[500] = {};
    const uint32_t imageCount = mRow * mColumn;
    imageSize.height = images.size.height / mRow;
    imageSize.width = images.size.width / mColumn;
    for (size_t i = 0; i < imageCount; ++i){
        sprintf(fileName, "%s/picture%d.%s", file.c_str(), i, images.type.c_str());
        if(images.type == "png")stbi_write_png(fileName, imageSize.width, imageSize.height, 4, images.datas[i], 0);
        else if(images.type == "jpg")stbi_write_jpg(fileName, imageSize.width, imageSize.height, 4, images.datas[i], 0);
        else if(images.type == "bmp")stbi_write_bmp(fileName, imageSize.width, imageSize.height, 4, images.datas[i]);
    }
}
void SplitImage::WriteImageToFile(VkPhysicalDevice physicalDevice, VkDevice device, VkQueue graphics, VkCommandPool pool, const std::string&file){
    if(!background.size.width || !background.size.height)return;
    bool supportsBlit = true;
    // Check blit support for source and destination
    VkFormatProperties formatProps;
    // Check if the device supports blitting to linear images
    vkGetPhysicalDeviceFormatProperties(physicalDevice, COLOR_FORMAT, &formatProps);
    if (!(formatProps.linearTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT)) {
        printf("Device does not support blitting to linear tiled images, using copy instead of blit!\n");
        supportsBlit = false;
    }
    const VkExtent2D realImageSize = GetBackgroundSize(images.size);
    // Source for the copy is the last rendered swapchain image
    VulkanImage dstionImage;
    dstionImage.size.depth = 1;
    dstionImage.view = VK_NULL_HANDLE;
    dstionImage.size.width = realImageSize.width;
    dstionImage.size.height = realImageSize.height;
    // Create the linear tiled destination image to copy to and to read the memory from
    dstionImage.CreateImage(device, VK_IMAGE_USAGE_TRANSFER_DST_BIT, COLOR_FORMAT, VK_IMAGE_TILING_LINEAR);
    dstionImage.AllocateAndBindMemory(device, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    VkImage srcImage = GetOffscreenImage(), dstImage = dstionImage.image;

    // Do the actual blit from the swapchain image to our host visible destination image
    VkCommandBuffer copyCmd;// = vulkanDevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
    vkf::tool::BeginSingleTimeCommands(device, pool, copyCmd);

    // Transition destination image to transfer destination layout
    vkf::tool::InsertImageMemoryBarrier(
        copyCmd,
        dstImage,
        0,
        VK_ACCESS_TRANSFER_WRITE_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

    // Transition swapchain image from present to transfer source layout
    vkf::tool::InsertImageMemoryBarrier(
        copyCmd,
        srcImage,
        VK_ACCESS_MEMORY_READ_BIT,
        VK_ACCESS_TRANSFER_READ_BIT,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

    // If source and destination support blit we'll blit as this also does automatic format conversion (e.g. from BGR to RGB)
    if (supportsBlit){
        // Define the region to blit (we will blit the whole swapchain image)
        VkOffset3D blitSize;
        blitSize.x = realImageSize.width;
        blitSize.y = realImageSize.height;
        blitSize.z = 1;
        VkImageBlit imageBlitRegion{};
        imageBlitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBlitRegion.srcSubresource.layerCount = 1;
        imageBlitRegion.srcOffsets[1] = blitSize;
        imageBlitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBlitRegion.dstSubresource.layerCount = 1;
        imageBlitRegion.dstOffsets[1] = blitSize;

        // Issue the blit command
        vkCmdBlitImage(copyCmd, srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageBlitRegion, VK_FILTER_NEAREST);
    }
    else{
        // Otherwise use image copy (requires us to manually flip components)
        VkImageCopy imageCopyRegion{};
        imageCopyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageCopyRegion.srcSubresource.layerCount = 1;
        imageCopyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageCopyRegion.dstSubresource.layerCount = 1;
        imageCopyRegion.extent.width = realImageSize.width;
        imageCopyRegion.extent.height = realImageSize.height;
        imageCopyRegion.extent.depth = 1;

        // Issue the copy command
        vkCmdCopyImage(copyCmd, srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopyRegion);
    }

    // Transition destination image to general layout, which is the required layout for mapping the image memory later on
    vkf::tool::InsertImageMemoryBarrier(
        copyCmd,
        dstImage,
        VK_ACCESS_TRANSFER_WRITE_BIT,
        VK_ACCESS_MEMORY_READ_BIT,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_GENERAL,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

    // Transition back the swap chain image after the blit is done
    vkf::tool::InsertImageMemoryBarrier(
        copyCmd,
        srcImage,
        VK_ACCESS_TRANSFER_READ_BIT,
        VK_ACCESS_MEMORY_READ_BIT,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

    vkf::tool::EndSingleTimeCommands(device, pool, graphics, copyCmd);;

    // Get layout of the image (including row pitch)
    VkImageSubresource subResource { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0 };
    VkSubresourceLayout subResourceLayout;
    vkGetImageSubresourceLayout(device, dstImage, &subResource, &subResourceLayout);

    // Map image memory so we can start copying from it
    const char* data;
    vkMapMemory(device, dstionImage.memory, 0, VK_WHOLE_SIZE, 0, (void**)&data);
    data += subResourceLayout.offset;

    const uint32_t imageSize = realImageSize.width * realImageSize.height * 4;
    // glm::float32 *imageDatas = new glm::float32[imageSize];
    stbi_uc *imageDatas = new stbi_uc[imageSize];
    uint32_t index = 0;
    for (uint32_t y = 0; y < realImageSize.height; y++){
        glm::float32 *pixel = (glm::float32 *)data;
        for (uint32_t x = 0; x < realImageSize.width; x++){
            // const uint32_t index = ROW_COLUMN_INDEX(y, x, realImageSize.width);
            // printf("%.0f%.0f%.0f%.0f ", *row, *(row + 1), *(row + 2), *(row + 3));
            imageDatas[index] = (*pixel) * 255;
            imageDatas[index + 1] = (*(pixel + 1)) * 255;
            imageDatas[index + 2] = (*(pixel + 2)) * 255;
            imageDatas[index + 3] = (*(pixel + 3)) * 255;
            // printf("%d%d%d%d ", imageDatas[index], imageDatas[index + 1], imageDatas[index + 2], imageDatas[index + 3]);
            // imageDatas[index] = data[index];
            // memcpy(imageDatas + index, row, sizeof(glm::float32) * 4);
            index += 4;
            pixel += sizeof(glm::float32);
        }
        // printf("\n");
        data += subResourceLayout.rowPitch;
    }
    // Clean up resources
    vkUnmapMemory(device, dstionImage.memory);
    dstionImage.Destroy(device);

    std::string outFile, imageType;
    const char *dot = strchr(file.c_str(), '.');
    if(dot){
        //说明给了文件名
        imageType = ++dot;
        outFile = file;
    }
    else{
        //文件夹
        imageType = images.type;
        outFile = file + "SplitImage," + images.type;
    }
    if(!strcmp(imageType.c_str(), "png"))stbi_write_png(outFile.c_str(), realImageSize.width, realImageSize.height, 4, imageDatas, 0);
    else if(!strcmp(imageType.c_str(), "jpg"))stbi_write_jpg(outFile.c_str(), realImageSize.width, realImageSize.height, 4, imageDatas, 0);
    else if (!strcmp(imageType.c_str(), "bmp"))stbi_write_bmp(outFile.c_str(), realImageSize.width, realImageSize.height, 4, imageDatas);
    delete[]imageDatas;
}
void SplitImage::Draw(VkCommandBuffer command, uint32_t windowWidth, uint32_t windowHeight){
    RerodOffscreenCommand(background.color);
    VulkanSplit::Draw(command, windowWidth, windowHeight, background.color);
}

void SplitImage::UpdateBackground(VkDevice device, uint32_t windowWidth, uint32_t windowHeight){
    if(IsLoadTexture()){
        UpdateOffscreenBackground(device);
        VulkanSplit::UpdateBackground(device, background.pos, background.size);
    }
    // else{
    //     VkExtent2D imageSize, backgroundSize;
    //     imageSize.width = windowWidth * .5f;
    //     imageSize.height = windowHeight * .5f;
    //     backgroundSize = GetBackgroundSize(imageSize);
    //     VulkanSplit::UpdateBackground(device, glm::vec3(windowWidth - backgroundSize.width * .5, windowHeight - backgroundSize.height * .5, 0), backgroundSize);
    // }
}

void SplitImage::Setup(VkPhysicalDevice physicalDevice, VkDevice device, VkQueue graphics, const VulkanPool &pool){
    mRow = 3;
    mColumn = 3;
    // effects.increase.row = 1;
    // effects.increase.column = 1;
    effects.increase.rowAndcolumn = 1;
    effects.cartoon.degree = glm::vec3(.5, .5, .5);
    // mGridImage.resize(mRow * mColumn);
    // mOffscreenGridImage.resize(mGridImage.size());
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

bool SplitImage::MouseCursor(double xpos, double ypos, int32_t &index){
    if(effects.increase.increase){
        VkExtent2D size;
        uint32_t val = 0;
        // std::vector<GRID_IMAGE_INFO>pos(mGridImage.size() + 1);
        glm::vec2 pos;
        for (size_t i = 0; i < mGridImage.size() + 1; i++){
            if(i != effects.increase.index){
                size = mGrid;
                pos = mGridImage[val++].pos;
            }
            else{
                size = mIncreaseGrid;
                pos = mIncreaseGridPos;
            }
            if(xpos > pos.x && ypos > pos.y && xpos < pos.x + size.width && ypos < pos.y + size.height){
                index = i;
                return true;
            }
        }
        // if(xpos > mIncreaseGridPos.x && ypos > mIncreaseGridPos.y && xpos < mIncreaseGridPos.x + mIncreaseGrid.width && ypos < mIncreaseGridPos.y + mIncreaseGrid.height){
        //     index = effects.increase.index;
        //     return true;
        // }
    }
    else{
        for (size_t i = 0; i < mGridImage.size(); ++i){
            if(xpos > mGridImage[i].pos.x && ypos > mGridImage[i].pos.y && xpos < mGridImage[i].pos.x + mGrid.width && ypos < mGridImage[i].pos.y + mGrid.height){
                index = i;
                return true;
            }
        }
    }
    return false;
}
// void PrintPixel(const stbi_uc *data, uint32_t width, uint32_t height){
//     for (uint32_t uiRow = 0; uiRow < height; ++uiRow){
//         for (uint32_t uiColumn = 0; uiColumn < width; ++uiColumn){
//             const uint32_t index = ROW_COLUMN_INDEX(uiRow, uiColumn, 3);
//             printf("%d%d%d%d ", data[index], data[index + 1], data[index + 2], data[index + 3]);
//         }
//         printf("\n");
//     }
// }
void SplitImage::ChangeImage(VkDevice device, const std::string &image, uint32_t windowWidth, uint32_t windowHeight, VkQueue graphics, VkCommandPool pool){
    int nrComponents;
    VkExtent2D source;
    stbi_uc* data = stbi_load(image.c_str(), (int *)&source.width, (int *)&source.height, &nrComponents, STBI_rgb_alpha);
    if(data == nullptr){
        printf("load picture error:%s\n", image.c_str());
        return;
    }

    if(mSourceImageData){
        delete[]mSourceImageData;
    }
    mSourceImageData = new stbi_uc[source.width * source.height * 4];
    memcpy(mSourceImageData, data, sizeof(stbi_uc) * source.width * source.height * 4);

    if(images.size.width != source.width || images.size.height != source.height || mRow * mColumn != mImageCount){
        //修改帧缓冲
        ReprepareOffscreenFramebuffer(device, GetBackgroundSize(source));
    }
    ChangeImage(device, data, source.width, source.height, graphics, pool);

    stbi_image_free(data);

    images.size = source;
    if(source.width > windowWidth || source.height > windowHeight){
        VkExtent2D backgroundSize;;
        const float zoom = .9f;
        do{
            source.width *= zoom;
            source.height *= zoom;
            backgroundSize = GetBackgroundSize(source);
        } while (backgroundSize.width > windowWidth || backgroundSize.height > windowHeight);
    }
    mGrid.height = source.height / mRow;
    mGrid.width = source.width / mColumn;

    const char *dot = strchr(image.c_str(), '.');
    std::string imageType[] = { ".gif", ".bmp", ".jpg", ".png" };
    for (size_t i = 0; i < sizeof(imageType) / sizeof(std::string); ++i){
        if(!strcmp(imageType[i].c_str(), dot)){
            images.type = imageType[i].c_str() + 1;
            break;
        }
    }
    InitBackgroundPos(windowWidth, windowHeight);
    InitGridImageInfo();
}
void SplitImage::ResetImage(VkDevice device, VkQueue graphics, VkCommandPool pool){
    uint32_t imageCount = ResetImage(device, mSourceImageData, images.size.width, images.size.height, effects.increase.increase);
    if(mRow * mColumn != mImageCount){
        RecreateImageUniform(device, imageCount);
    }
    if(effects.increase.increase)
        ChangeIncreaseImage(device, mIncreaseImageDatas, mIncreaseGrid.width, mIncreaseGrid.height, graphics, pool);
    else
        ChangeIncreaseImage(device, images.datas[0], mGrid.width, mGrid.height, graphics, pool);
    ChangeTextureImage(device, (const void **)images.datas.data(), images.datas.size(), mGrid.width, mGrid.height, graphics, pool);
    mImageCount = imageCount;
}
void SplitImage::ChangeImage(VkDevice device, const void *data, uint32_t width, uint32_t height, VkQueue graphics, VkCommandPool pool){
    VkExtent2D source, destination;
    source.width = width;
    source.height = height;
    destination.height = height / mRow;
    destination.width = width / mColumn;
    // VkExtent2D increaseImageSize = destination;
    effects.increase.increase = effects.increase.rowAndcolumn > 1;
    uint32_t imageCount = ResetImage(device, data, width, height, effects.increase.increase);

    if(mRow * mColumn != mImageCount){
        RecreateImageUniform(device, imageCount);
    }
    if(effects.increase.increase)
        ChangeIncreaseImage(device, mIncreaseImageDatas, mIncreaseGrid.width, mIncreaseGrid.height, graphics, pool);
    else
        ChangeIncreaseImage(device, images.datas[0], destination.width, destination.height, graphics, pool);
    ChangeTextureImage(device, (const void **)images.datas.data(), images.datas.size(), destination.width, destination.height, graphics, pool);
    mImageCount = imageCount;
}
// void SplitImage::SetSplitType(VkDevice device, const std::string &image, SPLIT_TYPE type, uint32_t windowWidth, uint32_t windowHeight, VkQueue graphics, VkCommandPool pool){
//     mSplitType = type;
//     ChangeTextureImage(device, image, windowWidth, windowHeight, graphics, pool);
// }
// void Split::FreeTextureImage(void *data){
//     stbi_image_free(data);
// }
// void Split::SwapImage(uint32_t sourceIndex, uint32_t destIndex)
// {
//     // uint32_t index = mImageIndex[sourceIndex];
//     // mImageIndex[sourceIndex] = mImageIndex[destIndex];
//     // mImageIndex[destIndex] = index;
// }
void SplitImage::SwapImage(VkDevice device, uint32_t sourceIndex, uint32_t destionIndex, VkQueue graphics, VkCommandPool pool){
    VkExtent2D source, destination;
    destination.height = images.size.height / mRow;
    destination.width = images.size.width / mColumn;
    // const uint32_t srcIdex = sourceIndex > effects.increase.index?sourceIndex - 1:sourceIndex, desIndex = destionIndex > effects.increase.index?destionIndex - 1:destionIndex;
    SwapImage(device, images.datas[sourceIndex - 1], destination.width, destination.height, destionIndex - 1, graphics, pool);
}

void SplitImage::SwapImage(VkDevice device, void *datas, uint32_t width, uint32_t height, uint32_t destIndex, VkQueue graphics, VkCommandPool pool){
    const uint32_t imageSize = width * height * 4;
    stbi_uc *data = new stbi_uc[imageSize];
    memcpy(data, images.datas[destIndex], imageSize);
    memcpy(images.datas[destIndex], datas, imageSize);
    memcpy(datas, data, imageSize);
    delete[]data;
    VkExtent2D imageExtent = mGrid;
    vkDeviceWaitIdle(device);
    ChangeTextureImage(device, (const void **)images.datas.data(), images.datas.size(), width, height, graphics, pool);
    mGrid = imageExtent;
}
uint32_t SplitImage::ResetImage(VkDevice device, const void *data, uint32_t width, uint32_t height, bool increase){
    VkExtent2D source, destination;
    source.width = width;
    source.height = height;
    destination.height = height / mRow;
    destination.width = width / mColumn;
    if(mIncreaseImageDatas){
        delete[]mIncreaseImageDatas;
        mIncreaseImageDatas = nullptr;
    }
    for (size_t i = 0; i < images.datas.size(); ++i){
        delete[]images.datas[i];
    }
    uint32_t imageCount;
    if(increase){
        images.datas.resize(mRow * mColumn - effects.increase.rowAndcolumn * effects.increase.rowAndcolumn);
        mGridImage.resize(images.datas.size());
        mOffscreenGridImage.resize(mGridImage.size());
        imageCount = images.datas.size() + 1;//这里应该多画一个, 因为扩大的那一个没在数组内
        uint32_t imageIndex = 0;
        //拷贝图片的时候应不应该加偏移(mOffset)?
        mIncreaseGrid.height = destination.height * effects.increase.rowAndcolumn;
        mIncreaseGrid.width = destination.width * effects.increase.rowAndcolumn;

        const uint32_t imageSize = destination.width * destination.height * 4;
        const uint32_t uiSelectRow = effects.increase.index?effects.increase.index / mRow:0, uiSelectColumn = effects.increase.index?effects.increase.index % mColumn:0;

        // const uint32_t copyRow = height / mIncreaseGrid.height, copyColumn = width / mIncreaseGrid.width;
        // const uint32_t remainingRow = height % mIncreaseGrid.height, remainingColumn = width % mIncreaseGrid.width;
        // if(mRow - uiSelectRow - 1 <= copyRow){
        //     mIncreaseGrid.height = remainingRow;
        // }
        // if(mColumn - uiSelectColumn - 1 <= copyColumn){
        //     mIncreaseGrid.width = remainingColumn;
        // }
        mIncreaseImageDatas = new stbi_uc[mIncreaseGrid.width * mIncreaseGrid.height * 4];
        glm::vec2 increaseGridOffset;
        increaseGridOffset.x = uiSelectColumn * mOffscreenGrid.width;
        increaseGridOffset.y = uiSelectRow * mOffscreenGrid.height;
        copy(data, mIncreaseImageDatas, uiSelectRow, uiSelectColumn, source, mIncreaseGrid,  increaseGridOffset);
        for (uint32_t uiRow = 0; uiRow < mRow; ++uiRow){
            for (uint32_t uiColumn = 0; uiColumn < mColumn; ++uiColumn){
                bool bCanCopy = true;
                for (int32_t iRow = 0; iRow < effects.increase.rowAndcolumn; ++iRow){
                    for (int32_t iColumn = 0; iColumn < effects.increase.rowAndcolumn; ++iColumn){
                        if(uiSelectColumn + iColumn == uiColumn && uiSelectRow + iRow == uiRow){
                            bCanCopy = false;
                            break;
                        }
                    }
                    if(!bCanCopy)break;
                }                
                if(bCanCopy){
                    //这里算出来的行列必须记录，一个不能少
                    mGridImage[imageIndex].row = uiRow;
                    mGridImage[imageIndex].column = uiColumn;
                    mOffscreenGridImage[imageIndex].row = uiRow;
                    mOffscreenGridImage[imageIndex].column = uiColumn;
                    images.datas[imageIndex] = new stbi_uc[imageSize];
                    copy(data, images.datas[imageIndex], uiRow, uiColumn, source, destination, glm::vec2(destination.width, destination.height));
                    ++imageIndex;
                }
                // copy(data, tempImage, uiRow, uiColumn, source, increaseImageSize);
                // if(memcmp(tempImage, mIncreaseImageDatas, sizeof(stbi_uc) * increaseImageSize.width * increaseImageSize.height * 4)){
                //     images.datas[index] = new stbi_uc[imageSize];
                //     copy(data, images.datas[index], uiRow, uiColumn, source, destination);
                //     ++index;
                // }
            }
        }
        // for (size_t i = 0; i < mGridImage.size(); ++i){
        //     printf("index = %d, uiRow = %d, uiColumn = %d\n", i, mGridImage[i].row, mGridImage[i].column);
        // }      
    }
    else{
        mGridImage.resize(mRow * mColumn);
        images.datas.resize(mRow * mColumn);
        mOffscreenGridImage.resize(mGridImage.size());
        imageCount = mRow * mColumn;
        const uint32_t imageSize = destination.width * destination.height * 4;
        for (uint32_t uiRow = 0; uiRow < mRow; ++uiRow){
            for (uint32_t uiColumn = 0; uiColumn < mColumn; ++uiColumn){
                const uint32_t index = ROW_COLUMN_INDEX(uiRow, uiColumn, mColumn);
                mGridImage[index].row = uiRow;
                mGridImage[index].column = uiColumn;
                mOffscreenGridImage[index].row = uiRow;
                mOffscreenGridImage[index].column = uiColumn;
                images.datas[index] = new stbi_uc[imageSize];
                // copy(data, images.datas[index], uiRow, uiColumn, source, destination);
                copy(data, images.datas[index], uiRow, uiColumn, source, destination, glm::vec2(destination.width, destination.height));
            }
        }
    }
    return imageCount;
}
void SplitImage::UpdateTexture(VkDevice device, uint32_t index, const VkExtent2D &grid){
    Uniform ubo;
    // VkExtent2D offscreenGridSize;
    // bool bUseImageArray = effects.increase.column == 1 && effects.increase.row == 1;
    ubo.cartoons.degree = effects.cartoon.degree;
    ubo.cartoons.cartoons = effects.cartoon.cartoons;
    ubo.useImageArray = !effects.increase.increase || index != effects.increase.index;
    const int32_t gridImageIndex = index > effects.increase.index?index - 1:index;
    ubo.imageIndex = effects.increase.increase?gridImageIndex:index;
    if(effects.increase.increase){
        if(index == effects.increase.index)
            ubo.model = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(mIncreaseGridPos, 0)), glm::vec3(grid.width, grid.height, 1));
        else
            ubo.model = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(mGridImage[gridImageIndex].pos, 0)), glm::vec3(grid.width, grid.height, 1));
    }
    else
        ubo.model = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(mGridImage[index].pos, 0)), glm::vec3(grid.width, grid.height, 1));
    // ubo.model = glm::scale(glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(mImagePos[index], 0)), effects.angle[index], glm::vec3(0, 0, 1)), glm::vec3(grid.width, grid.height, 1));
    VulkanSplit::UpdateTexture(device, index, ubo);

    // offscreenGridSize.height = images.size.height / mRow;
    // offscreenGridSize.width = images.size.width / mColumn;
    // if(effects.increase.increase && index == effects.increase.index){
    //     offscreenGridSize.height = offscreenGridSize.height * effects.increase.row + (effects.increase.row - 1) *  mOffset;
    //     offscreenGridSize.width = offscreenGridSize.width * effects.increase.column + (effects.increase.column - 1) *  mOffset;
    // }
    if(effects.increase.increase){
        if(index == effects.increase.index)
            ubo.model = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(mOffscreenIncreaseGridPos, 0)), glm::vec3(mOffscreenIncreaseGrid.width, mOffscreenIncreaseGrid.height, 1));
        else
           ubo.model = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(mOffscreenGridImage[gridImageIndex].pos, 0)), glm::vec3(mOffscreenGrid.width, mOffscreenGrid.height, 1));
    }
    else{
        ubo.model = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(mOffscreenGridImage[index].pos, 0)), glm::vec3(mOffscreenGrid.width, mOffscreenGrid.height, 1));
    }
    // ubo.model = glm::scale(glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(mOffscreenImagePos[index], 0)), effects.angle[index], glm::vec3(0, 0, 1)), glm::vec3(offscreenGridSize.width, offscreenGridSize.height, 1));
    UpdateOffscreenTexture(device, index, ubo);
}
