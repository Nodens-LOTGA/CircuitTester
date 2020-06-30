#include "mainwindow.h"
#include <QApplication>
#include <QDebug>
#include <QLibraryInfo>
#include <QTranslator>
#include <QStyleFactory>

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);

  QTranslator qtTranslator;
  if (qtTranslator.load(
          QLocale::system(), "qt", "_",
          QLibraryInfo::location(QLibraryInfo::TranslationsPath))) {
    a.installTranslator(&qtTranslator);
  }

  a.setStyle(QStyleFactory::create("Fusion"));
  a.setStyleSheet(R"<>( * {
  font-family: "Verdana";
  font-size: 28px;
  })<>");

  MainWindow w;

  w.show();
  return a.exec();
}
