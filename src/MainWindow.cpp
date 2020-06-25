#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "QSettings"
#include "Report.h"
#include "ReportDelegate.h"
#include "ReportDialog.h"
#include "Settings.h"
#include "helpdialog.h"
#include "settingsdialog.h"
#include "sqltools.h"
#include "tools.h"
#include "winapiprint.h"
#include <QByteArray>
#include <QDate>
#include <QDesktopWidget>
#include <QHeaderView>
#include <QImage>
#include <QMessageBox>
#include <QPaintEngine>
#include <QPainter>
#include <QPrinterInfo>
#include <QTableView>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QtSql>

//Выбор жгута пользователем
//

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  connect(ui->exitB, SIGNAL(clicked()), this, SLOT(exit()));
  connect(ui->settingsB, SIGNAL(clicked()), this, SLOT(settings()));
  connect(ui->helpB, SIGNAL(clicked()), this, SLOT(help()));
  connect(ui->startB, SIGNAL(clicked()), this, SLOT(start()));

  QCoreApplication::setOrganizationName("Ladaplast");
  QCoreApplication::setOrganizationDomain("temp.com");
  QCoreApplication::setApplicationName("Tester");

  init();
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::init() {
  StartUpDialog startUpDialog(this);
  startUpDialog.setWindowFlags(Qt::Dialog /*| Qt::CustomizeWindowHint |
                               Qt::FramelessWindowHint*/);

  startUpDialog.exec();
  if (startUpDialog.result() != QDialog::Accepted) {
    QTimer::singleShot(0, qApp, SLOT(quit()));
    return;
  }
  loadSettings();

  this->showFullScreen();

  /*if (!port.open(ui->portL->text().toStdString())) {
    QMessageBox::warning(this, RU("Ошибка порта"),
                         RU("Неудалось открыть порт"));
    exit();
  }*/

  initSql();
}

void MainWindow::initSql() {
  if (!QSqlDatabase::drivers().contains("QSQLITE"))
    QMessageBox::critical(this, RU("Невозможно создать БД"),
                          RU("Для БД необходимо наличие SQLITE драйвера"));

  QSqlError err = Sql::initDb();
  if (err.type() != QSqlError::NoError) {
    Sql::showSqlError(err, this);
    exit();
    return;
  }
}

void MainWindow::start() {
  ui->dateL->setText(QDate::currentDate().toString("dd.MM.yyyy"));

  rep::Report report(curNum, prodName, workerName);

  if (!report.fill()) {
    QMessageBox::warning(this, RU("Ошибка при создание отчёта"),
                         RU("Не выбрана таблица цепей"));
    return;
  }

  // report.checkAll(port);
  // TODO:

  QString dir = reportDir;
  if (!QDir(dir).exists())
    QDir().mkdir(dir);
  dir += "/" + prodName.section(" ", 0, 0);
  if (!QDir(dir).exists())
    QDir().mkdir(dir);
  if (!report.saveTo(dir + "/" + QString::number(curNum) + ".csv")) {
    QMessageBox::warning(this, RU("Ошибка сохранения отчёта"),
                         RU("Неудалось сохранить отчёт на диск"));
    return;
  }
  if (!report.hasError()) {
    Settings sett;
    curNum++;
    ui->chkNumL->setText(QString::number(curNum).rightJustified(6, '0'));
    sett.num = curNum;
    sett.save();
  }

  ReportDialog dialog(report, this);
 
  dialog.exec();
}

void MainWindow::exit() {
  Settings sett;
  sett.num = curNum;
  sett.save();
  this->hide();
  port.close();
  Sql::closeDb();
  QApplication::quit(); // TODO:
  // init();
}

void MainWindow::settings() {
  SettingsDialog settingsDialog(this);
  connect(&settingsDialog, SIGNAL(accepted()), this, SLOT(loadSettings()));

  settingsDialog.setWindowState(Qt::WindowFullScreen);
  QDesktopWidget desk;
  QRect screenres = desk.screenGeometry(0);
  settingsDialog.setGeometry(QRect(screenres.x(), screenres.y(),
                                   screenres.width(), screenres.height()));
  settingsDialog.exec();
}

void MainWindow::help() {
  HelpDialog helpDialog(this);
  helpDialog.exec();
}

void MainWindow::loadSettings() {
  Settings sett;

  workerName = sett.userName;
  ui->workerNameL->setText(workerName);
  ui->portL->setText(sett.portName);
  ui->dateL->setText(QDate::currentDate().toString("dd.MM.yyyy"));
  ui->settingsB->setEnabled(sett.isAdmin);
  curNum = sett.num;
  ui->chkNumL->setText(QString::number(curNum).rightJustified(6, '0'));
  reportDir = sett.reportDir;
  prodName = sett.prodName;
  ui->prodNameL->setText(prodName);
  reportPrinterName = sett.reportPrinterName;
  labelPrinterName = sett.labelPrinterName;
}
