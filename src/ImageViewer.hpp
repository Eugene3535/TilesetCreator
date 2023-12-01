#include <QtWidgets>
#include <QMainWindow>

#include "Image.hpp"

class ImageViewer: 
    public QMainWindow
{
public:
    ImageViewer(QWidget* parent = nullptr) noexcept;

private:
    void openImage() noexcept;
    void generateImage() noexcept;
    void clearImage() noexcept;

    void setXSize8() noexcept;
    void setXSize16() noexcept;
    void setXSize32() noexcept;

    void updateUI() noexcept;

private:
    std::uint32_t calculateWeight(const std::uint8_t* firstPixel, std::uint32_t tileWidth, std::uint32_t tileHeight, std::uint32_t imageWidth) noexcept;
    void calculateTilesetSize(std::uint32_t& width, std::uint32_t& height, std::uint32_t numPixels) noexcept;

private:
    Image   m_rawPixels;
    QString m_imageName;

    std::unordered_map<std::uint32_t, std::uint8_t*> m_tileMap;
    std::unordered_map<std::uint32_t, std::uint32_t> m_tileIDs;
    std::vector<std::uint32_t>                       m_weights;

private:
    QLabel*  m_imageLabel;
    QAction* m_clearAction;

    QAction* m_x8Action;
    QAction* m_x16Action;
    QAction* m_x32Action;

private:
    std::uint32_t m_tileSize;

    static const std::uint32_t x8;
    static const std::uint32_t x16;
    static const std::uint32_t x32;
};