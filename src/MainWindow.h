#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "startupdialog.h"
#include <tuple>
#include <QMainWindow>
#include <QPrinter>
#include "Report.h"
#include <boost/graph/adjacency_list.hpp>
#ifdef Q_OS_WIN
#include "SerialPort.h"
#endif

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

private slots:
  void init();
  void initSql();
  void start();
  void exit();
  void settings();
  void help();
  bool fillItems();
  void loadSettings();
  void checkCircuits();

private:
  Ui::MainWindow *ui;
  
  SerialPort port;
  int curNum{1};
  QString prodName{}, reportDir{}, workerName{}, labelPrinterName{},
      reportPrinterName{};
  QMap<int, QVector<Item>> circuits;
  QMap<char, QVector<char>> pinRelations;
  QMap<int, Item> items;
};
#endif // MAINWINDOW_H
