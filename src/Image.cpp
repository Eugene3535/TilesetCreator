#include <iostream>
#include <algorithm>
#include <cctype>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include "Image.hpp"

Image::Image() noexcept:
    m_width(0),
    m_height(0)
{
}

Image::~Image()
{
}

bool Image::create(std::int32_t width, std::int32_t height, const std::uint8_t* pixels) noexcept
{
    if((width < 1) || (height < 1))
        return false;

    m_pixels.clear();
    m_width  = static_cast<std::uint32_t>(width);
    m_height = static_cast<std::uint32_t>(height);

    m_pixels.resize(static_cast<size_t>(m_width * m_height * 4));

    if(pixels)
        std::memcpy(&m_pixels[0], pixels, m_pixels.size());

    return true;
}

void Image::copy(const Image& source, std::uint32_t destX, std::uint32_t destY, std::uint32_t areaWidth, std::uint32_t areaHeight, const std::uint8_t* srcPixels) noexcept
{
    if ((source.m_width == 0U) || (source.m_height == 0U) || (m_width == 0U) || (m_height == 0U) || (!srcPixels))
        return;

    std::uint32_t pitch     = areaWidth << 2;
    std::uint32_t rows      = areaHeight;
    std::uint32_t srcStride = source.m_width << 2;
    std::uint32_t dstStride = m_width << 2;

    std::uint8_t* dstPixels = &m_pixels[0] + ((destX + destY * m_width) << 2);

    for (std::uint32_t i = 0U; i < rows; ++i)
    {
        std::memcpy(dstPixels, srcPixels, pitch);
        srcPixels += srcStride;
        dstPixels += dstStride;
    }
}

bool Image::loadFromFile(const std::string& filepath) noexcept
{
    m_pixels.clear();

    std::int32_t width = 0;
    std::int32_t height = 0;
    std::int32_t bytePerPixel = 0;
    std::uint8_t* data = stbi_load(filepath.c_str(), &width, &height, &bytePerPixel, STBI_rgb_alpha);

    if (!data)
        return false;

    bool result = create(width, height, data);
    stbi_image_free(data);

    return result;
}

bool Image::saveToFile(const std::string& filename) const noexcept
{
    if ((!m_pixels.empty()) && (m_width > 0u) && (m_height > 0u))
    {
        // Extract the extension
        const std::size_t dot = filename.find_last_of('.');
        std::string extension = (dot != std::string::npos) ? filename.substr(dot + 1) : std::string();

        if(extension.empty())
            return false;

        std::transform(extension.cbegin(), extension.cend(), extension.begin(),
                [](unsigned char c) { return std::tolower(c); });

        std::int32_t width  = static_cast<std::int32_t>(m_width);
        std::int32_t height = static_cast<std::int32_t>(m_height);

        if (extension == "bmp") // BMP
        {
            if (stbi_write_bmp(filename.c_str(), width, height, 4, m_pixels.data()))
                return true;
        }
        else if (extension == "tga") // TGA
        {     
            if (stbi_write_tga(filename.c_str(), width, height, 4, m_pixels.data()))
                return true;
        }
        else if (extension == "png") // PNG
        {        
            if (stbi_write_png(filename.c_str(), width, height, 4, m_pixels.data(), 0))
                return true;
        }
        else if (extension == "jpg" || extension == "jpeg") // JPG
        {     
            if (stbi_write_jpg(filename.c_str(), width, height, 4, m_pixels.data(), 90))
                return true;
        }
    }

    std::cerr << "Failed to save image \"" << filename << "\"\n";

    return false;
}

const std::uint8_t* Image::pixels() const noexcept
{
    return m_pixels.data();
}

std::uint8_t* Image::pixels() noexcept
{
    if(!m_pixels.empty())
        return &m_pixels[0];

    return nullptr;
}

std::uint32_t Image::width() const noexcept
{
    return m_width;
}

std::uint32_t Image::height() const noexcept
{
    return m_height;
}

void Image::clear() noexcept
{
    m_pixels.clear();
    m_width = 0U;
    m_height = 0U;
}

bool Image::empty() const noexcept
{
    return m_pixels.empty();
}