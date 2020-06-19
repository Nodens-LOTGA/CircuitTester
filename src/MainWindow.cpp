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

  // TODO:
  ui->i_saveDirL->hide();
  ui->saveDirL->hide();

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

  if (!fillItems())
    return;

  // TODO:

  Report report(circuits, curNum, prodName, workerName);
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
  settingsDialog.exec();
}

void MainWindow::help() {
  HelpDialog helpDialog(this);
  helpDialog.exec();
}

bool MainWindow::fillItems() {
  QSqlQuery q;

  int tableId{-1};
  if (!q.exec(QString("SELECT id, name FROM products WHERE name = \'%1\'")
                  .arg(prodName)))
    Sql::showSqlError(q.lastError());
  else {
    while (q.next()) {
      QString s = q.value(1).toString();
      if (q.value(1).toString() == prodName) {
        tableId = q.value(0).toInt();
        break;
      }
    }
  }

  if (tableId == -1) {
    QMessageBox::warning(this, RU("Ошибка при создание отчёта"),
                         RU("Не выбрана таблица цепей"));
    return false;
  }

  pinRelations.clear();
  circuits.clear();
  if (!q.exec(QString("SELECT id, num, nameFrom, circuitFrom, pinFrom, nameTo, "
                      "circuitTo, "
                      "pinTo FROM circuits%1 ORDER "
                      "BY id")
                  .arg(tableId)))
    Sql::showSqlError(q.lastError());
  while (q.next()) {
    Item item;
    item.circuitNum = q.value(1).toInt();
    item.name = q.value(2).toString();
    item.circuit = q.value(3).toInt();
    item.pin = q.value(4).toInt();
    Item relation;
    relation.name = q.value(5).toString();
    relation.circuit = q.value(6).toInt();
    relation.pin = q.value(7).toInt();
    item.relations.push_back(relation);
    circuits[item.circuitNum].push_back(item);
    pinRelations[item.pin].push_back(relation.pin);
    if (!items.contains(item.pin))
      items[item.pin] = item;
    if (!items.contains(relation.pin))
      items[relation.pin] = relation;
  }
  return true;
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
  ui->saveDirL->setText(reportDir);
  prodName = sett.prodName;
  ui->prodNameL->setText(prodName);
  reportPrinterName = sett.reportPrinterName;
  labelPrinterName = sett.labelPrinterName;
}

void MainWindow::checkCircuits() {
  QByteArray rb, wb;
  const char cb[] = {0x23, 0x55, 0x48};
  for (auto &i : circuits) {
    for (auto &k : i) {
      wb.clear();
      wb.append(cb);
      wb += k.pin;
      wb += k.relations[0].pin;
      int attempt{};
      do {
        if (attempt == 3) {
          //TODO:
          return;
        }    
        QThread::sleep(1);
        port.write(wb.data(), wb.size());
        port.read(rb.data(), 255);
      } while (!rb.startsWith(wb.chopped(4)));
      for (int j = 4; j < rb.size(); j++) {
        if (rb.at(j) == k.relations[0].pin)
          break;
        else if (pinRelations.value(k.pin).contains(rb.at(j))) {
          k.stat = Item::Status::Error;
          Item item = items.value(rb.at(j));
          item.stat = Item::Status::Short;
          k.relations.push_back(item);
        } else {
          k.stat = Item::Status::Open;
        }
      }
    }
  }
}
