#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "startupdialog.h"
#include <tuple>
#include <QMainWindow>
#include <QPrinter>
#include "Report.h"
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
  void loadSettings();

private:
  Ui::MainWindow *ui;
  
  SerialPort port;
  int curNum{1};
  QString prodName{}, reportDir{}, workerName{}, labelPrinterName{},
      reportPrinterName{};
};
#endif // MAINWINDOW_H
