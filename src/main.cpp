#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <fstream>
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

//  Containers
    std::unordered_map<std::uint32_t, std::uint8_t*> tileMap;
    std::unordered_map<std::uint32_t, std::uint32_t> tileIDs;
    std::vector<std::uint32_t> weights;

    const uint32_t tileWidth  = 16U;
    const uint32_t tileHeight = 16U;
    const uint32_t mapWidth   = image.width();
    const uint32_t mapHeight  = image.height();

    const auto pixels = image.pixels();

    std::uint32_t cnt = 0U;
    std::uint32_t tileNum = 0U;

    for (std::uint32_t y = 0u; y < mapHeight; y += tileHeight)
        for (std::uint32_t x = 0u; x < mapWidth; x += tileWidth)
        {
            auto offset = x + y * mapWidth;
            offset <<= 2;
            auto firstPixel = pixels + offset;
            auto weight = CalculateWeight(firstPixel, tileWidth, tileHeight, mapWidth);

            tileMap.try_emplace(weight, firstPixel);
            weights.push_back(weight);

            if(auto pair = tileIDs.try_emplace(weight, tileNum); pair.second)
            {
                tileNum++;
            }

            cnt++;
        }

    std::cout << "Tileset size: " << tileMap.size() << '\n';
    std::cout << "Map size in tiles: " << cnt << '\n';

    std::uint32_t columns  = 0U;
    std::uint32_t rows = 0U;
    CalculateTilesetSize(columns, rows, static_cast<std::uint32_t>(tileMap.size()));

    std::cout << "Tileset width: " << columns << "\theight: " << rows << '\n';

    Image image_out;
	image_out.create(columns * tileWidth, rows * tileHeight);

    for (std::uint32_t y = 0, index = 0; y < rows; ++y)
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

    for(auto& n : weights)
    {
        if(auto id = tileIDs.find(n); id != tileIDs.end())
        {
            n = id->second;
        }
    }

    const uint32_t tileMapWidth  = mapWidth / tileWidth;
    const uint32_t tileMapHeight = mapHeight / tileHeight;

    std::string buffer;
    std::ofstream ofs("test.txt");

    if(ofs.is_open())
    {
        for (std::uint32_t y = 0; y < tileMapHeight; ++y)
        {
            for (std::uint32_t x = 0; x < tileMapWidth; ++x)
            {
                const uint32_t index = y * tileMapWidth + x;
                buffer += std::to_string(weights[index]);
                buffer.push_back(',');
            }
            buffer.push_back('\n');
        }
//      Remove tre last comma from file and close it
        buffer.pop_back();
        buffer.pop_back();
        ofs << buffer;
        ofs.close();
    }

    return 0;
}