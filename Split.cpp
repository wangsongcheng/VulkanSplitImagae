// #include <algorithm>
#include "Split.h"
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"
void copy(const stbi_uc *source, stbi_uc *destination, uint32_t row, uint32_t column, const VkExtent2D& sourceImage, const VkExtent2D& destinationImage) {
    //如果要截取第2行的图片，需要跳过 row * 目标图片的高度 * 原图宽度
    uint32_t uiLineSize = destinationImage.width * 4;
    uint32_t uiSourceLineSize = sourceImage.width * 4;
    uint32_t imageOffsetStart = row * destinationImage.height * uiSourceLineSize + column * uiLineSize;
    for (size_t dataOffset = 0; dataOffset < destinationImage.height; ++dataOffset) {
        memcpy(destination + dataOffset * uiLineSize, source + imageOffsetStart + dataOffset * uiSourceLineSize, uiLineSize);
    }
}
void SplitImage::UpdateImage(VkDevice device){
    glm::vec2 offscreenImageSize;
    offscreenImageSize.y = images.realSize.height / mRow;
    offscreenImageSize.x = images.realSize.width / mColumn;
    for (uint32_t uiRow = 0; uiRow < mRow; ++uiRow){
        for (uint32_t uiColumn = 0; uiColumn < mColumn; ++uiColumn){
            const uint32_t index = ROW_COLUMN_INDEX(uiRow, uiColumn, mColumn);
            if(mImagePos[index] != glm::vec3(0)){
                UpdateTexture(device, index, mImagePos[index], glm::vec2(images.size.width, images.size.height));
                UpdateOffscreen(device, index, mRealImagePos[index], offscreenImageSize);
            }
            // mImagePos[mImageIndex[index]] = glm::vec3(backgroundPos.x + uiColumn * splitImage.size.width + (uiColumn + 1) * offset, backgroundPos.y + uiRow * splitImage.size.height + (uiRow + 1) * offset, 0);
            // UpdateTexture(device, mImageIndex[index], mImagePos[mImageIndex[index]], glm::vec2(splitImage.size.width, splitImage.size.height));
        }
    }
}
void SplitImage::InitImagePos(uint32_t windowWidth, uint32_t windowHeight){
    mImagePos.resize(mRow * mColumn);
    mRealImagePos.resize(mRow * mColumn);
    const uint32_t offset = 10;
    VkExtent2D offscreenImageSize;
    offscreenImageSize.height = images.realSize.height / mRow;
    offscreenImageSize.width = images.realSize.width / mColumn;
    background.size = glm::vec3((mColumn + 1) * offset + mColumn * images.size.width, (mRow + 1) * offset + mRow * images.size.height, 1);
    background.pos = glm::vec3(windowWidth * .5 - background.size.x * .5, windowHeight * .5 - background.size.y * .5, 0);
    if(mSplitType == JIGSAW){
        mImagePos[0] = glm::vec3(background.pos.x + offset, background.pos.y + offset, 0);
        for (uint32_t uiRow = mRow - 2; uiRow < mRow; ++uiRow){
            for (uint32_t uiColumn = mColumn - 2; uiColumn < mColumn; ++uiColumn){
                const uint32_t index = ROW_COLUMN_INDEX(uiRow, uiColumn, mColumn);
                mImagePos[index] = glm::vec3(background.pos.x + uiColumn * images.size.width + (uiColumn + 1) * offset, background.pos.y + uiRow * images.size.height + (uiRow + 1) * offset, 0);
                mRealImagePos[index] = glm::vec3(uiColumn * offscreenImageSize.width + (uiColumn + 1) * offset, uiRow * offscreenImageSize.height + (uiRow + 1) * offset, 0);
            }
        }
    }
    else if(mSplitType == AAIGNED){
        for (uint32_t uiRow = 0; uiRow < mRow; ++uiRow){
            for (uint32_t uiColumn = 0; uiColumn < mColumn; ++uiColumn){
                const uint32_t index = ROW_COLUMN_INDEX(uiRow, uiColumn, mColumn);
                mImagePos[index] = glm::vec3(background.pos.x + uiColumn * images.size.width + (uiColumn + 1) * offset, background.pos.y + uiRow * images.size.height + (uiRow + 1) * offset, 0);
                mRealImagePos[index] = glm::vec3(uiColumn * offscreenImageSize.width + (uiColumn + 1) * offset, uiRow * offscreenImageSize.height + (uiRow + 1) * offset, 0);
            }
        }
    }
}

SplitImage::SplitImage(/* args */){
}

SplitImage::~SplitImage(){

}

void SplitImage::Cleanup(VkDevice device){
    VulkanSplit::Cleanup(device);
    for (size_t i = 0; i < images.datas.size(); ++i){
        delete[]images.datas[i];
    }
}

void SplitImage::WriteImageToFolder(const std::string&file){
    VkExtent2D imageSize;
    char fileName[500] = {};
    const uint32_t imageCount = mRow * mColumn;
    imageSize.height = images.realSize.height / mRow;
    imageSize.width = images.realSize.width / mColumn;
    for (size_t i = 0; i < imageCount; ++i){
        sprintf(fileName, "%s/picture%d.%s", file.c_str(), i, images.type.c_str());
        if(images.type == "png")stbi_write_png(fileName, imageSize.width, imageSize.height, 4, images.datas[i], 0);
        else if(images.type == "jpg")stbi_write_jpg(fileName, imageSize.width, imageSize.height, 4, images.datas[i], 0);
        else if(images.type == "bmp")stbi_write_bmp(fileName, imageSize.width, imageSize.height, 4, images.datas[i]);
    }
}
void SplitImage::WriteImageToFile(VkPhysicalDevice physicalDevice, VkDevice device, VkQueue graphics, VkCommandPool pool, const std::string&file){
    if(!background.size.x || !background.size.y)return;
    bool supportsBlit = true;
    // Check blit support for source and destination
    VkFormatProperties formatProps;
    // Check if the device supports blitting from optimal images (the swapchain images are in optimal format)
    vkGetPhysicalDeviceFormatProperties(physicalDevice, SWAPCHAIN_FORMAT, &formatProps);
    if (!(formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT)) {
        printf("Device does not support blitting from optimal tiled images, using copy instead of blit!\n");
        supportsBlit = false;
    }
    // Check if the device supports blitting to linear images
    vkGetPhysicalDeviceFormatProperties(physicalDevice, COLOR_FORMAT, &formatProps);
    if (!(formatProps.linearTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT)) {
        printf("Device does not support blitting to linear tiled images, using copy instead of blit!\n");
        supportsBlit = false;
    }
    const uint32_t offset = 10;
    VkExtent2D realImageSize;
    realImageSize.height = (mRow + 1) * offset + images.realSize.height;
    realImageSize.width = (mColumn + 1) * offset + images.realSize.width;
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
    glm::float32 *imageDatas = new glm::float32[imageSize];
    // stbi_uc *imageDatas = new stbi_uc[imageSize];
    // ppm binary pixel data
    for (uint32_t y = 0; y < realImageSize.height; y++){
        glm::float32 *row = (glm::float32 *)data;
        for (uint32_t x = 0; x < realImageSize.width; x++){
            const uint32_t index = ROW_COLUMN_INDEX(y, x, realImageSize.width);
            // printf("%.0f%.0f%.0f%.0f ", *row, *(row + 1), *(row + 2), *(row + 3));
            // imageDatas[index] = *row;
            // imageDatas[index + 1] = *(row + 1);
            // imageDatas[index + 2] = *(row + 2);
            // imageDatas[index + 3] = *(row + 3);
            memcpy(imageDatas + index, row, sizeof(glm::float32) * 4);
            row += sizeof(glm::float32);
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

void SplitImage::UpdateUniform(VkDevice device, uint32_t windowWidth, uint32_t windowHeight){
    const uint32_t offset = 10;
    if(IsLoadTexture()){
        UpdateBackground(device, background.pos, background.size);
        const glm::vec3 size = glm::vec3((mColumn + 1) * offset + images.realSize.width, (mRow + 1) * offset + images.realSize.height, 1);
        UpdateOffscreenBackground(device, size);
    }
    else{
        UpdateBackground(device, glm::vec3(windowWidth - (windowWidth - 3 * offset), windowHeight - (windowHeight - 5 * offset), 0), glm::vec3(windowWidth - 6 * offset, windowHeight - 7 * offset, 1));
    }
    if(!images.datas.empty())UpdateImage(device);
}

void SplitImage::Setup(VkPhysicalDevice physicalDevice, VkDevice device, VkQueue graphics, const VulkanPool &pool){
    if(mSplitType == JIGSAW){
        mRow = 4;
        mColumn = 3;
    }
    else if(mSplitType == AAIGNED){
        mRow = 3;
        mColumn = 3;
    }
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

bool SplitImage::mousecursor(double xpos, double ypos, uint32_t &index){
    for (size_t i = 0; i < mImagePos.size(); ++i){
        // if(xpos > mImagePos[mImageIndex[i]].x && ypos > mImagePos[mImageIndex[i]].y && xpos < mImagePos[mImageIndex[i]].x + mImageSize.width && ypos < mImagePos[mImageIndex[i]].y + mImageSize.height){
        if(xpos > mImagePos[i].x && ypos > mImagePos[i].y && xpos < mImagePos[i].x + images.size.width && ypos < mImagePos[i].y + images.size.height){
            index = i;
            // index = mImageIndex[i];
            return true;
        }
    }
    return false;
}
void SplitImage::ChangeTextureImage(VkDevice device, const std::string &image, uint32_t windowWidth, uint32_t windowHeight, VkQueue graphics, VkCommandPool pool){
    int nrComponents;
    VkExtent2D source, destination;
    stbi_uc* data = stbi_load(image.c_str(), (int *)&source.width, (int *)&source.height, &nrComponents, STBI_rgb_alpha);
    if(data == nullptr){
        printf("load picture error:%s\n", image.c_str());
        return;
    }
    destination.height = source.height / mRow;
    destination.width = source.width / mColumn;
    for (size_t i = 0; i < images.datas.size(); ++i){
        delete[]images.datas[i];
    }
    images.datas.resize(mRow * mColumn);
    const uint32_t imageSize = destination.width * destination.height * 4;
    for (uint32_t uiRow = 0; uiRow < mRow; ++uiRow){
        for (uint32_t uiColumn = 0; uiColumn < mColumn; ++uiColumn){
            const uint32_t index = ROW_COLUMN_INDEX(uiRow, uiColumn, mColumn);
            images.datas[index] = new stbi_uc[imageSize];
            copy(data, images.datas[index], uiRow, uiColumn, source, destination);
        }
    }
    stbi_image_free(data);
    if(mRow * mColumn != mImageCount){
        RecreateImageUniform(device, mRow * mColumn);
    }
    if(images.realSize.width != source.width || images.realSize.height != source.height){
        //修改帧缓冲
        const uint32_t offset = 10;
        const glm::vec3 size = glm::vec3((mColumn + 1) * offset + source.width, (mRow + 1) * offset + source.height, 1);
        ReprepareOffscreenFramebuffer(device, size);
    }
    // vkDeviceWaitIdle(device);
    images.realSize = source;
    VulkanSplit::ChangeTextureImage(device, (const void **)images.datas.data(), images.datas.size(), destination.width, destination.height, graphics, pool);
    if(source.width > windowWidth || source.height > windowHeight){
        const float zoom = .8f;
        do{
            source.width *= zoom;
            source.height *= zoom;
        } while (source.width > windowWidth || source.height > windowHeight);        
    }
    images.size.height = source.height / mRow;
    images.size.width = source.width / mColumn;

    const char *dot = strchr(image.c_str(), '.');
    std::string imageType[] = { ".gif", ".bmp", ".jpg", ".png" };
    for (size_t i = 0; i < sizeof(imageType) / sizeof(std::string); ++i){
        if(!strcmp(imageType[i].c_str(), dot)){
            images.type = imageType[i].c_str() + 1;
            break;
        }
    }
    InitImagePos(windowWidth, windowHeight);
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
void SplitImage::SwapImage(VkDevice device, VkQueue graphics, VkCommandPool pool, uint32_t sourceIndex, uint32_t destIndex){
    VkExtent2D source, destination;
    destination.height = images.realSize.height / mRow;
    destination.width = images.realSize.width / mColumn;
    const uint32_t imageSize = destination.width * destination.height * 4;
    stbi_uc *data = new stbi_uc[imageSize];
    memcpy(data, images.datas[sourceIndex], imageSize);
    memcpy(images.datas[sourceIndex], images.datas[destIndex], imageSize);
    memcpy(images.datas[destIndex], data, imageSize);
    delete[]data;
    VkExtent2D imageExtent = images.size;
    VulkanSplit::ChangeTextureImage(device, (const void **)images.datas.data(), images.datas.size(), destination.width, destination.height, graphics, pool);
    images.size = imageExtent;
}