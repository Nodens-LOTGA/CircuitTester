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
#include <QFontMetrics>
#include <QHeaderView>
#include <QImage>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QPaintEngine>
#include <QPainter>
#include <QPrinterInfo>
#include <QScreen>
#include <QTableView>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QToolButton>
#include <QtSql>
#include <QString>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  connect(ui->exitB, SIGNAL(clicked()), this, SLOT(exit()));
  connect(ui->settingsB, SIGNAL(clicked()), this, SLOT(settings()));
  connect(ui->helpB, SIGNAL(clicked()), this, SLOT(help()));
  connect(ui->startB, SIGNAL(clicked()), this, SLOT(start()));

  QCoreApplication::setOrganizationName("Ladaplast");
  QCoreApplication::setOrganizationDomain("temp.com");
  QCoreApplication::setApplicationName("CircuitTester");

  prodNameL = new QLabel(ui->prodNameTB);
  prodNameL->setAlignment(Qt::AlignCenter);
  prodNameL->setMouseTracking(false);
  prodNameL->setTextInteractionFlags(Qt::NoTextInteraction);
  prodNameL->setWordWrap(true);
  ui->prodNameTB->setLayout(new QHBoxLayout(ui->prodNameTB));
  ui->prodNameTB->layout()->setContentsMargins(11, 11, 11, 11);
  ui->prodNameTB->layout()->addWidget(prodNameL);
  ui->prodNameTB->layout()->setSizeConstraint(QLayout::SetMinimumSize);

  QMenu *menu = new QMenu(ui->prodNameTB);
  ui->prodNameTB->setMenu(menu);

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
  initSql();
  loadSettings();

  // TODO:

  if (!port.open(ui->portL->text().toStdString(), false, SerialPort::BaudRate::Baud115200)) {
    QMessageBox::warning(this, RU("Ошибка порта"),
                         RU("Не удалось открыть порт"));
    QTimer::singleShot(0, this, [this]() { exit(false); });
  }

  this->showFullScreen();
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

  if (!port.reopen()) {
    QMessageBox::warning(this, RU("Ошибка порта"),
                         RU("Не удалось открыть порт"));
    return;
  }
  if (!report.checkAll(port)) {
    QMessageBox::warning(this, RU("Ошибка при проверки жгута"),
                         RU("Превышен лимит ожидания от стэнда"));
  }
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

void MainWindow::exit(bool save) {
  if (save) {
    if (QMessageBox::question(this, RU("Завершение работы"),
                              RU("Вы действительно хотите выйти?")) ==
        QMessageBox::Yes) {
      Settings sett;
      sett.num = curNum;
      sett.prodName = prodName;
      sett.save();
    } else
      return;
  }
  this->hide();
  port.close();
  Sql::closeDb();
  QApplication::quit(); // TODO:
  // init();
}

void MainWindow::settings() {
  SettingsDialog settingsDialog(this);
  connect(&settingsDialog, SIGNAL(accepted()), this, SLOT(loadSettings()));
  connect(&settingsDialog, SIGNAL(rejected()), this, SLOT(updateProd()));

  settingsDialog.setWindowState(Qt::WindowFullScreen);
  settingsDialog.setGeometry(QGuiApplication::screens().first()->geometry());
  settingsDialog.exec();
}

void MainWindow::help() {
  HelpDialog helpDialog("help.qhc", this);
  helpDialog.setWindowState(Qt::WindowFullScreen);
  helpDialog.setGeometry(QGuiApplication::screens().first()->geometry());
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
  reportPrinterName = sett.reportPrinterName;
  labelPrinterName = sett.labelPrinterName;
  updateProd();
}

void MainWindow::updateProd() {
  Settings sett;
  QSqlQuery q;
  if (!q.exec("SELECT id, name FROM products"))
    Sql::showSqlError(q.lastError());
  QString name{}, lastName{};
  auto menu = ui->prodNameTB->menu();
  menu->clear();
  while (q.next()) {
    name = q.value(1).toString();
    auto action = new QAction(name, menu);
    connect(action, &QAction::triggered, [=](bool chk) { setProdName(name); });
    menu->addAction(action);
    if (name == sett.prodName)
      lastName = name;
  }
  setProdName(lastName);
}

void MainWindow::setProdName(const QString &name) {
  prodNameL->setText(name);
  prodName = name;
}
