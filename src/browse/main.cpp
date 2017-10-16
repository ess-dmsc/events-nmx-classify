#include "Browser.h"
#include <QApplication>
#include <vector>

int main(int argc, char *argv[])
{
  H5CC::exceptions_off();

  QApplication a(argc, argv);
  a.setWindowIcon(QIcon(":/icons/tpcc.xpm"));

  QCoreApplication::setOrganizationName("ESS");
  QCoreApplication::setApplicationName("tpcClassify");

  Browser w;
  w.show();

  return a.exec();
}
