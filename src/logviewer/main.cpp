#include <iostream>
#include <string>
#include <sstream>
using namespace std;

#include "logviewer.h"
#include <QApplication>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    LogViewer w;
    w.show();

    return app.exec();
}
