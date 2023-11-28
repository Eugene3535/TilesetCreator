#include <iostream>
#include <unordered_map>
#include <cmath>

#include "Image.hpp"

std::uint32_t CalculateWeight(const std::uint8_t* firstPixel, std::uint32_t tileWidth, std::uint32_t tileHeight, std::uint32_t imageWidth)
{
    std::uint32_t weight = 0u;

    for (std::uint32_t y = 0u; y < tileHeight; ++y)
        for (std::uint32_t x = 0u; x < tileWidth; ++x)
        {
            auto offset = x + y * imageWidth;
            offset <<= 2;
            const std::uint8_t* pixel = firstPixel + offset;

            weight += static_cast<std::uint32_t>(pixel[0]);
            weight += static_cast<std::uint32_t>(pixel[1]);
            weight += static_cast<std::uint32_t>(pixel[2]);
            weight += static_cast<std::uint32_t>(pixel[3]);
        }

    return weight;
}

void CalculateTilesetSize(std::uint32_t& width, std::uint32_t& height, std::uint32_t numPixels)
{
    std::uint32_t squareRoot = static_cast<std::uint32_t>(std::sqrt(numPixels));
    width  = squareRoot;
    height = squareRoot;

    while (width * height < numPixels) 
    {
        if (width <= height) 
            ++width;
        else 
            ++height;  
    }
}

int main(int argc, char *argv[])
{
    Image image;

    if( ! image.loadFromFile("./res/map.png"))
        return -1;

    std::unordered_map<std::uint32_t, std::uint8_t*> tileMap;

    const uint32_t tileWidth  = 16U;
    const uint32_t tileHeight = 16U;
    const uint32_t mapWidth   = image.width();
    const uint32_t mapHeight  = image.height();

    const auto pixels = image.pixels();

    std::uint32_t cnt = 0U;

    for (std::uint32_t y = 0u; y < mapHeight; y += tileHeight)
        for (std::uint32_t x = 0u; x < mapWidth; x += tileWidth)
        {
            auto offset = x + y * mapWidth;
            offset <<= 2;
            auto firstPixel = pixels + offset;
            auto weight = CalculateWeight(firstPixel, tileWidth, tileHeight, mapWidth);

            tileMap.try_emplace(weight, firstPixel);

            cnt++;
        }

    std::cout << "Tileset size: " << tileMap.size() << '\n';
    std::cout << "Map size in tiles: " << cnt << '\n';

    std::uint32_t columns  = 0;
    std::uint32_t rows = 0;
    CalculateTilesetSize(columns, rows, static_cast<std::uint32_t>(tileMap.size()));

    std::cout << "Tileset width: " << columns << "\theight: " << rows << '\n';

    Image image_out;
	image_out.create(columns * tileWidth, rows * tileHeight);

    std::size_t index = 0;

    for (std::uint32_t y = 0; y < rows; ++y)
        for (std::uint32_t x = 0; x < columns; ++x)
        {
            if(index < tileMap.size())
            {
                auto it = tileMap.begin();
                std::advance(it, index++);
                image_out.copy(image, x * tileWidth, y * tileHeight, tileWidth, tileHeight, it->second);
            }
        }

    image_out.saveToFile("test.png");
    
    return 0;
}