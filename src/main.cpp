#include "mainwindow.h"

#include <QApplication>
#include <QDebug>
#include <QLibraryInfo>
#include <QTranslator>
int main(int argc, char *argv[]) {
  QApplication a(argc, argv);

  QTranslator qtTranslator;
  if (qtTranslator.load(
          QLocale::system(), "qt", "_",
          QLibraryInfo::location(QLibraryInfo::TranslationsPath))) {
    a.installTranslator(&qtTranslator);
  }

  a.setStyleSheet(R"<>( * {
  font-family: "Verdana";
  font-size: 32px;
  })<>");
  
  MainWindow w;
  w.show();
  return a.exec();
}
