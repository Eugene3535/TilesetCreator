#include <vector>
#include <string>
#include <unordered_map>
#include <fstream>
#include <cmath>

#include "ImageViewer.hpp"

const std::uint32_t ImageViewer::x8  = 8U;
const std::uint32_t ImageViewer::x16 = 16U;
const std::uint32_t ImageViewer::x32 = 32U;

ImageViewer::ImageViewer(QWidget* parent) noexcept :
    QMainWindow(parent),
	m_imageLabel(nullptr),
	m_x8Action(nullptr),
	m_x16Action(nullptr),
	m_x32Action(nullptr),
    m_tileSize(x16)
{
    resize(480, 360);

    m_imageLabel = new QLabel(this);
    m_imageLabel->setStyleSheet("QLabel { border: 1px solid black; }");
    m_imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    m_imageLabel->setScaledContents(true);

    QPushButton* generateButton = new QPushButton("Генерировать", this);

    QMenu* fileMenu = new QMenu("Файл", this);

    QAction* openAction = fileMenu->addAction("Открыть");
    m_clearAction = fileMenu->addAction("Очистить");
    QAction* closeAction = fileMenu->addAction("Закрыть");

    QMenu* optionsMenu = new QMenu("Опции", this);
    QMenu* xSizeMenu = new QMenu("Размер тайла", this);
    m_x8Action  = xSizeMenu->addAction("x8");
    m_x16Action = xSizeMenu->addAction("x16");
    m_x32Action = xSizeMenu->addAction("x32");
    optionsMenu->addMenu(xSizeMenu);

    QMenuBar* menuBar = new QMenuBar(this);
    menuBar->addMenu(fileMenu);
    menuBar->addMenu(optionsMenu);
    setMenuBar(menuBar);

    QHBoxLayout* buttonLayout = new QHBoxLayout(this);
    buttonLayout->addStretch();
    buttonLayout->addWidget(generateButton);

    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->addWidget(m_imageLabel);
    mainLayout->addLayout(buttonLayout);

    connect(openAction, &QAction::triggered, this, &ImageViewer::openImage);
    connect(m_clearAction, &QAction::triggered, this, &ImageViewer::clearImage);
    connect(closeAction, &QAction::triggered, this, &ImageViewer::close);
    
    connect(m_x8Action, &QAction::triggered, this, &ImageViewer::setXSize8);
    connect(m_x16Action, &QAction::triggered, this, &ImageViewer::setXSize16);
    connect(m_x32Action, &QAction::triggered, this, &ImageViewer::setXSize32);

    connect(generateButton, &QPushButton::pressed, this, &ImageViewer::generateImage);
}

void ImageViewer::openImage() noexcept
{ 
    QString imagePath = QFileDialog::getOpenFileName(this, "Открыть изображение", QDir::currentPath(), "Изображения (*.png *.jpg *.bmp *.jpeg)");

    if (!imagePath.isEmpty()) 
    {
        if(m_rawPixels.loadFromFile(imagePath.toStdString()))
        {
            QPixmap image(imagePath);
            m_imageLabel->setPixmap(image);

            QDir dir(imagePath);
            m_imageName = dir.dirName();
        }
    }

    updateUI();
}

void ImageViewer::generateImage() noexcept
{
    if(m_rawPixels.empty())
        return;

    if (m_rawPixels.width() < (m_tileSize >> 2)) return;
    if (m_rawPixels.height() < (m_tileSize >> 2)) return;

    QString savePath = QFileDialog::getExistingDirectory(this, "Выберите директорию для сохранения", QDir::currentPath(), QFileDialog::ShowDirsOnly);

    if (!savePath.isEmpty())
    {
        uint32_t tileWidth  = m_tileSize;
        uint32_t tileHeight = m_tileSize;
        uint32_t mapWidth   = m_rawPixels.width();
        uint32_t mapHeight  = m_rawPixels.height();

//      Check out of bounds
        if (auto remainder = (mapWidth % tileWidth); remainder != 0)
            mapWidth = (mapWidth - remainder);

        if (auto remainder = (mapHeight % tileHeight); remainder != 0)
            mapHeight = (mapHeight - remainder);

        const auto pixels = m_rawPixels.pixels();

        std::uint32_t total_cnt = 0U;
        std::uint32_t tileNum = 0U;

        for (std::uint32_t y = 0u; y < mapHeight; y += tileHeight)
            for (std::uint32_t x = 0u; x < mapWidth; x += tileWidth)
            {
                auto offset = x + y * mapWidth;
                offset <<= 2;
                auto firstPixel = pixels + offset;
                auto weight = calculateWeight(firstPixel, tileWidth, tileHeight, mapWidth);

                m_tileMap.try_emplace(weight, firstPixel);
                m_weights.push_back(weight);

                if(auto pair = m_tileIDs.try_emplace(weight, tileNum); pair.second)
                {
                    tileNum++;
                }

                total_cnt++;
            }

        std::uint32_t columns  = 0U;
        std::uint32_t rows = 0U;
        calculateTilesetSize(columns, rows, static_cast<std::uint32_t>(m_tileMap.size()));

        Image image_out;
        image_out.create(columns * tileWidth, rows * tileHeight);

        for (std::uint32_t y = 0, index = 0; y < rows; ++y)
            for (std::uint32_t x = 0; x < columns; ++x)
            {
                if(index < m_tileMap.size())
                {
                    auto it = m_tileMap.begin();
                    std::advance(it, index++);
                    image_out.copy(m_rawPixels, x * tileWidth, y * tileHeight, tileWidth, tileHeight, it->second);
                }
            }

        image_out.saveToFile((savePath + "/tileset_" + m_imageName).toStdString());

        for(auto& n : m_weights)
        {
            if(auto id = m_tileIDs.find(n); id != m_tileIDs.end())
            {
                n = id->second;
            }
        }

        const uint32_t tileMapWidth  = mapWidth / tileWidth;
        const uint32_t tileMapHeight = mapHeight / tileHeight;

        std::string buffer;
        std::string csvName = m_imageName.toStdString().substr(0, m_imageName.size() - 4);

        std::ofstream ofs(savePath.toStdString() + '/' + csvName + "_csv.txt");

        if(ofs.is_open())
        {
            for (std::uint32_t y = 0; y < tileMapHeight; ++y)
            {
                for (std::uint32_t x = 0; x < tileMapWidth; ++x)
                {
                    const uint32_t index = y * tileMapWidth + x;
                    buffer += std::to_string(m_weights[index]);
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

void ImageViewer::clearImage() noexcept
{ 
    m_imageLabel->clear();
    m_rawPixels.clear();
    m_imageName.clear();
    updateUI();
}

void ImageViewer::setXSize8() noexcept
{ 
    m_tileSize = x8;
}

void ImageViewer::setXSize16() noexcept
{ 
    m_tileSize = x16;
}

void ImageViewer::setXSize32() noexcept
{ 
    m_tileSize = x32;
}

void ImageViewer::updateUI() noexcept
{
    bool imageOpened = ((!m_imageLabel->pixmap().isNull()) && (!m_rawPixels.empty()));
    m_clearAction->setEnabled(imageOpened);
}

std::uint32_t ImageViewer::calculateWeight(const std::uint8_t* firstPixel, std::uint32_t tileWidth, std::uint32_t tileHeight, std::uint32_t imageWidth) noexcept
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

void ImageViewer::calculateTilesetSize(std::uint32_t& width, std::uint32_t& height, std::uint32_t numPixels) noexcept
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