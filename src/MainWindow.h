#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "Report.h"
#include "startupdialog.h"
#include <QLabel>
#include <QMainWindow>
#include <QPrinter>
#include <tuple>
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
  void updateProd();
  void setProdName(const QString &name);

private:
  Ui::MainWindow *ui;
  QLabel *prodNameL;

  SerialPort port;
  int curNum{1};
  QString prodName{}, reportDir{}, workerName{}, labelPrinterName{},
      reportPrinterName{};
};
#endif // MAINWINDOW_H
