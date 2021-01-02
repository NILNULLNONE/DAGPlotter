#include <QApplication>
#include "DAGTest.hpp"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    DAGTestNS::Run();
    return a.exec();
}
