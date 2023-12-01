#include "ImageViewer.hpp"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    ImageViewer viewer;
    viewer.show();

    return app.exec();
}