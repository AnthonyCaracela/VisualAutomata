#include <QApplication>

#include "../view/MainWindow.h"

int main(int argc, char* argv[]) {
    QApplication application(argc, argv);
    fa::view::MainWindow window;
    window.show();
    return application.exec();
}
