#include <vector>
#include <string>
#include <unordered_map>
#include <fstream>
#include <cmath>

#include "ImageViewer.hpp"

ImageViewer::ImageViewer(QWidget* parent):
    QMainWindow(parent)
{
    resize(480, 360);

    imageLabel = new QLabel;
    imageLabel->setStyleSheet("QLabel { border: 1px solid black; }");
    imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageLabel->setScaledContents(true);
    generateButton = new QPushButton("Генерировать");

    fileMenu = new QMenu("Файл");
    openAction = fileMenu->addAction("Открыть");

    clearAction = fileMenu->addAction("Очистить");
    closeAction = fileMenu->addAction("Закрыть");

    optionsMenu = new QMenu("Опции");
    xSizeMenu = new QMenu("X-размер");
    x8Action = xSizeMenu->addAction("x8");
    x16Action = xSizeMenu->addAction("x16");
    x32Action = xSizeMenu->addAction("x32");
    optionsMenu->addMenu(xSizeMenu);

    menuBar = new QMenuBar;
    menuBar->addMenu(fileMenu);
    menuBar->addMenu(optionsMenu);
    setMenuBar(menuBar);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    buttonLayout->addWidget(generateButton);

    QWidget *centralWidget = new QWidget;
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->addWidget(imageLabel);
    mainLayout->addLayout(buttonLayout);

    connect(openAction, &QAction::triggered, this, &ImageViewer::openImage);
    connect(clearAction, &QAction::triggered, this, &ImageViewer::clearImage);
    connect(closeAction, &QAction::triggered, this, &ImageViewer::close);
    
    connect(x8Action, &QAction::triggered, this, &ImageViewer::setXSize8);
    connect(x16Action, &QAction::triggered, this, &ImageViewer::setXSize16);
    connect(x32Action, &QAction::triggered, this, &ImageViewer::setXSize32);

    connect(generateButton, &QPushButton::pressed, this, &ImageViewer::generateImage);
}

void ImageViewer::openImage()   
{ 
    QString imagePath = QFileDialog::getOpenFileName(this, "Открыть изображение", "", "Изображения (*.png *.jpg *.bmp *.jpeg)");

    if (!imagePath.isEmpty()) 
    {
        if(m_image.loadFromFile(imagePath.toStdString()))
        {
            QPixmap image(imagePath);
            imageLabel->setPixmap(image);
        }
    }

    updateUI();
}

void ImageViewer::generateImage() 
{
    if(m_image.empty())
        return;

    QString savePath = QFileDialog::getExistingDirectory(this, "Выберите директорию для сохранения", "", QFileDialog::ShowDirsOnly);

    if (!savePath.isEmpty())
    {
        const uint32_t tileWidth  = 16U;
        const uint32_t tileHeight = 16U;
        const uint32_t mapWidth   = m_image.width();
        const uint32_t mapHeight  = m_image.height();

        const auto pixels = m_image.pixels();

        std::uint32_t cnt = 0U;
        std::uint32_t tileNum = 0U;

        for (std::uint32_t y = 0u; y < mapHeight; y += tileHeight)
            for (std::uint32_t x = 0u; x < mapWidth; x += tileWidth)
            {
                auto offset = x + y * mapWidth;
                offset <<= 2;
                auto firstPixel = pixels + offset;
                auto weight = calculateWeight(firstPixel, tileWidth, tileHeight, mapWidth);

                tileMap.try_emplace(weight, firstPixel);
                weights.push_back(weight);

                if(auto pair = tileIDs.try_emplace(weight, tileNum); pair.second)
                {
                    tileNum++;
                }

                cnt++;
            }

        std::uint32_t columns  = 0U;
        std::uint32_t rows = 0U;
        calculateTilesetSize(columns, rows, static_cast<std::uint32_t>(tileMap.size()));

        Image image_out;
        image_out.create(columns * tileWidth, rows * tileHeight);

        for (std::uint32_t y = 0, index = 0; y < rows; ++y)
            for (std::uint32_t x = 0; x < columns; ++x)
            {
                if(index < tileMap.size())
                {
                    auto it = tileMap.begin();
                    std::advance(it, index++);
                    image_out.copy(m_image, x * tileWidth, y * tileHeight, tileWidth, tileHeight, it->second);
                }
            }

        image_out.saveToFile(savePath.toStdString() + "/test.png");

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
        std::ofstream ofs(savePath.toStdString() + "/test.txt");

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
        updateUI();
    }
}

void ImageViewer::clearImage()
{ 
    imageLabel->clear();
    updateUI();
}

void ImageViewer::setXSize8()
{ 
    
}

void ImageViewer::setXSize16()
{ 
    
}

void ImageViewer::setXSize32()
{ 
    
}

void ImageViewer::updateUI() 
{
    bool imageOpened = !imageLabel->pixmap().isNull();
    clearAction->setEnabled(imageOpened);
}

std::uint32_t ImageViewer::calculateWeight(const std::uint8_t* firstPixel, std::uint32_t tileWidth, std::uint32_t tileHeight, std::uint32_t imageWidth)
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

void ImageViewer::calculateTilesetSize(std::uint32_t& width, std::uint32_t& height, std::uint32_t numPixels)
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