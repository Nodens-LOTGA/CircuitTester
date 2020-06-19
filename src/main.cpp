#include "mainwindow.h"

#include <QApplication>
#include <QTranslator>
#include <QDebug>
#include <QLibraryInfo>
int main(int argc, char *argv[])
{
  QApplication a(argc, argv);

  QTranslator qtTranslator;
  if (qtTranslator.load(
          QLocale::system(), "qt", "_",
          QLibraryInfo::location(QLibraryInfo::TranslationsPath))) {
    a.installTranslator(&qtTranslator);
  }

  MainWindow w;
  w.show();
  return a.exec();
}
