#ifndef IMAGE_HPP
#define IMAGE_HPP

#include <cstdint>
#include <string>
#include <vector>

class Image
{
public:
    Image() noexcept;
    ~Image();

    bool create(std::int32_t width, std::int32_t height, const std::uint8_t* pixels = nullptr) noexcept;
    void copy(const Image& source, std::uint32_t destX, std::uint32_t destY, std::uint32_t areaWidth, std::uint32_t areaHeight, const std::uint8_t* srcPixels) noexcept;
    bool loadFromFile(const std::string& filepath)     noexcept;
    bool saveToFile(const std::string& filepath) const noexcept;

    const std::uint8_t* pixels() const noexcept;

    std::uint8_t* pixels() noexcept;
    std::uint32_t width()  const noexcept;
    std::uint32_t height() const noexcept;

private:
    std::vector<std::uint8_t> m_pixels;
    std::uint32_t m_width;
    std::uint32_t m_height;
};

#endif // !IMAGE_HPP