#include <QtWidgets>
#include <QMainWindow>

#include "Image.hpp"

class ImageViewer: 
    public QMainWindow
{
public:
    ImageViewer(QWidget* parent = nullptr);

private slots:
    void openImage();
    void generateImage();
    void clearImage();

    void setXSize8();
    void setXSize16();
    void setXSize32();

    void updateUI();

private:
    std::uint32_t calculateWeight(const std::uint8_t* firstPixel, std::uint32_t tileWidth, std::uint32_t tileHeight, std::uint32_t imageWidth);
    void calculateTilesetSize(std::uint32_t& width, std::uint32_t& height, std::uint32_t numPixels);

private:
    QLabel *imageLabel;
    QPushButton *generateButton;

    QMenu *fileMenu;
    QAction *openAction;
    QAction *clearAction;
    QAction *closeAction;

    QMenu *optionsMenu;
    QMenu *xSizeMenu;
    QAction *x8Action;
    QAction *x16Action;
    QAction *x32Action;

    QMenuBar *menuBar;

private:
    Image m_image;

    std::unordered_map<std::uint32_t, std::uint8_t*> tileMap;
    std::unordered_map<std::uint32_t, std::uint32_t> tileIDs;
    std::vector<std::uint32_t> weights;
};