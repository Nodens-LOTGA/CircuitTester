#include "settingsdialog.h"
#include "./ui_settingsdialog.h"
#include "CircuitInputDialog.h"
#include "CircuitsDelegate.h"
#include "ProductInputDialog.h"
#include "Report.h"
#include "Settings.h"
#include "sqltools.h"
#include "tools.h"
#include <QComboBox>
#include <QDialog>
#include <QFileDialog>
#include <QFontDialog>
#include <QInputDialog>
#include <QPrintDialog>
#include <QPrinter>
#include <QPrinterInfo>
#include <QSettings>
#include <QStringList>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::SettingsDialog) {
  ui->setupUi(this);

  connect(ui->okPB, SIGNAL(clicked()), this, SLOT(tryAccept()));
  connect(ui->cancelPB, SIGNAL(clicked()), this, SLOT(reject()));
  connect(ui->setLabelPrinterPB, SIGNAL(clicked()), this,
          SLOT(setLabelPrinter()));
  connect(ui->setReportPrinterPB, SIGNAL(clicked()), this,
          SLOT(setReportPrinter()));
  connect(ui->reportDirTB, SIGNAL(clicked()), this, SLOT(setReportDir()));
  connect(ui->addTB, SIGNAL(clicked()), this, SLOT(addItem()));
  connect(ui->delTB, SIGNAL(clicked()), this, SLOT(delItem()));
  connect(ui->resetSettPB, SIGNAL(clicked()), this, SLOT(resetSett()));
  connect(ui->delUsersPB, SIGNAL(clicked()), this, SLOT(delUsers()));
  connect(ui->changeAdminPassPB, SIGNAL(clicked()), this,
          SLOT(changeAdminPass()));
  connect(ui->changeNumPB, SIGNAL(clicked()), this, SLOT(changeNum()));
  connect(ui->addProdTB, SIGNAL(clicked()), this, SLOT(addProd()));
  connect(ui->delProdTB, SIGNAL(clicked()), this, SLOT(delProd()));
  connect(ui->prodNameCB, SIGNAL(currentIndexChanged(int)),
          SLOT(populateTable()));
  connect(ui->editTB, SIGNAL(clicked()), this, SLOT(editItem));

  loadSettings(Settings());

  updateProducts();
  populateTable();
}

SettingsDialog::~SettingsDialog() { delete ui; }

void SettingsDialog::tryAccept() {
  Settings sett;
  reportDir = ui->reportDirLE->text();
  prodName = ui->prodNameCB->currentText();
  sett.reportDir = reportDir;
  sett.prodName = prodName;
  sett.newUsers = ui->newUsersCB->isChecked();
  sett.num = curNum;
  sett.labelPrinterName = labelPrinterName;
  sett.reportPrinterName = reportPrinterName;
  sett.users[sett.adminName] = adminPass;
  sett.save();
  accept();
}

void SettingsDialog::setLabelPrinter() {
  auto printers = QPrinterInfo::availablePrinterNames();
  bool ok;
  QString item = QInputDialog::getItem(this, RU("Выберите принтер"),
                                       RU("Принтер:"), printers, 0, false, &ok);
  if (ok && !item.isEmpty()) {
    labelPrinterName = item;
    ui->labelPrinterL->setText(item);
  }
}

void SettingsDialog::setReportPrinter() {
  auto printers = QPrinterInfo::availablePrinterNames();
  bool ok;
  QString item = QInputDialog::getItem(this, RU("Выберите принтер"),
                                       RU("Принтер:"), printers, 0, false, &ok);
  if (ok && !item.isEmpty()) {
    reportPrinterName = item;
    ui->reportPrinterL->setText(item);
  }
}

void SettingsDialog::setReportDir() {
  QString dir = QFileDialog::getExistingDirectory(
      this, RU("Сохранить отчёт в"), "",
      QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

  if (!dir.isEmpty()) {
    ui->reportDirLE->setText(dir);
    reportDir = dir;
  }
}

void SettingsDialog::addItem() {
  if (ui->prodNameCB->currentText() == "") {
    QMessageBox::warning(this, RU("Ошибка при добавление цепи"),
                         RU("Необходимо выбрать продукт"));
    return;
  }
  QSqlQuery q;
  int tableId = ui->prodNameCB->currentData().toInt();
  if (!q.prepare(Sql::insertCircuitsSql(tableId)))
    Sql::showSqlError(q.lastError());
  
  CircuitInputDialog dialog(tableId, this);

  if (!dialog.result() == QDialog::Accepted)
    return;

  auto id = dialog.id;
  auto n = dialog.num;
  auto nFrom = dialog.nameFrom;
  auto cFrom = dialog.circuitFrom;
  auto pFrom = dialog.pinFrom;
  auto nTo = dialog.nameTo;
  auto cTo = dialog.circuitTo;
  auto pTo = dialog.pinTo;

  if (!q.prepare(Sql::insertCircuitsSql(tableId)))
    Sql::showSqlError(q.lastError());
  Sql::addItem(q, n, nFrom, cFrom, pFrom, nTo, cTo, pTo);
  if (!model.submitAll())
    Sql::showSqlError(q.lastError());
  model.select();
}

void SettingsDialog::delItem() {
  if (ui->prodNameCB->currentText() == "") {
    QMessageBox::warning(this, RU("Ошибка при удаление цепи"),
                         RU("Необходимо выбрать продукт"));
    return;
  }
  QSqlQuery q;
  for (auto &i : ui->circuitTV->selectionModel()->selectedIndexes())
    model.removeRow(i.row());
  if (!model.submitAll())
    Sql::showSqlError(q.lastError());
  model.select();
}

void SettingsDialog::populateTable() {
  if (ui->prodNameCB->currentText() == "") {
    return;
  }

  int id = ui->prodNameCB->currentData().toInt();
  model.setTable("circuits" + QString::number(id));
  model.setEditStrategy(QSqlTableModel::OnFieldChange);
  // TODO:
  model.setHeaderData(0, Qt::Horizontal, RU("№"));
  model.setHeaderData(1, Qt::Horizontal, RU("№ Цепи"));
  model.setHeaderData(2, Qt::Horizontal, RU("Колодка"));
  model.setHeaderData(3, Qt::Horizontal, RU("№ контакта"));
  model.setHeaderData(4, Qt::Horizontal, RU("Контакт\n стенда"));
  model.setHeaderData(5, Qt::Horizontal, RU("Колодка"));
  model.setHeaderData(6, Qt::Horizontal, RU("№ контакта"));
  model.setHeaderData(7, Qt::Horizontal, RU("Контакт\n стенда"));

  model.select();

  ui->circuitTV->setModel(&model);
  // ui->circuitTV->verticalHeader()->setVisible(false);
  ui->circuitTV->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  ui->circuitTV->setItemDelegate(new CircuitsDelegate(ui->circuitTV));
  ui->circuitTV->setEditTriggers(QAbstractItemView::NoEditTriggers);

  prodName = ui->prodNameCB->currentText();
}

void SettingsDialog::resetSett() {
  Settings sett(true);
  loadSettings(sett);
}

void SettingsDialog::delUsers() {
  Settings sett;
  sett.discardUsers();
}

void SettingsDialog::loadSettings(Settings &sett) {
  reportDir = sett.reportDir;
  ui->reportDirLE->setText(reportDir);
  prodName = sett.prodName;
  ui->newUsersCB->setChecked(sett.newUsers);
  curNum = sett.num;
  labelPrinterName = sett.labelPrinterName;
  reportPrinterName = sett.reportPrinterName;
  adminPass = sett.users[sett.adminName].toString();
  ui->labelPrinterL->setText(labelPrinterName);
  ui->reportPrinterL->setText(reportPrinterName);
}

void SettingsDialog::changeAdminPass() {
  Settings sett;
  bool ok;
  QString newPass = QInputDialog::getText(
      this, RU("Введите новый пароль"), RU("Новый пароль"),
      QLineEdit::PasswordEchoOnEdit, "", &ok);
  if (ok)
    if (newPass.length() < 8) {
      QMessageBox::warning(
          this, RU("Неверный пароль"),
          RU("Пароль должен состоять не менее чем из 8 символов"));
      return;
    } else
      adminPass = newPass;
}

void SettingsDialog::changeNum() {
  bool ok;
  int newNum = QInputDialog::getInt(this, RU("Введите новый номер проверки"),
                                    RU("Новый номер проверки"), curNum, 0,
                                    214783647, 1, &ok);
  if (ok)
    curNum = newNum;
}

void SettingsDialog::addProd() {
  bool ok;
  ProductInputDialog dialog(prodName.section(" ", 0, 0),
                            prodName.section(" ", 1, 1), this);
  dialog.exec();

  

  if (!dialog.result() == QDialog::Accepted)
    return;

  QString newProd = dialog.art + " " + dialog.name;

  QSqlQuery q;

  if (!q.exec(QString("SELECT id, name FROM products WHERE name = \'%1\'")
                  .arg(newProd)))
    Sql::showSqlError(q.lastError());
  else {
    while (q.next()) {
      QString s = q.value(1).toString();
      if (q.value(1).toString() == newProd) {
        QMessageBox::warning(this, RU("Неверное название жгута"),
                             RU("Жгут с таким именем уже существует"));
        return;
      }
    }
  }

  if (!q.prepare(Sql::insertProductsSql()))
    Sql::showSqlError(q.lastError());
  else {
    Sql::addProd(q, newProd);
    if (!q.exec(QString("SELECT id, name FROM products WHERE name = \"%1\"")
                    .arg(newProd)))
      Sql::showSqlError(q.lastError());
    q.next();
    int id = q.value(0).toInt();
    if (!q.exec(Sql::circuitsSql(id)))
      Sql::showSqlError(q.lastError());
  }
  updateProducts();
}

void SettingsDialog::delProd() {
  if (ui->prodNameCB->currentText() == "") {
    QMessageBox::warning(this, RU("Ошибка при удаление продукта"),
                         RU("Необходимо выбрать продукт"));
    return;
  }
  int id = ui->prodNameCB->currentData().toInt();
  model.clear();
  QSqlQuery q;
  if (!q.exec("DROP TABLE circuits" + QString::number(id)))
    Sql::showSqlError(q.lastError());
  if (!q.exec("DELETE FROM products WHERE id =" + QString::number(id)))
    Sql::showSqlError(q.lastError());
  ui->prodNameCB->removeItem(ui->prodNameCB->currentIndex());
  updateProducts();
}

void SettingsDialog::updateProducts() {
  QSqlQuery q;
  if (!q.exec("SELECT id, name FROM products"))
    Sql::showSqlError(q.lastError());
  ui->prodNameCB->clear();
  Settings sett;
  int id{};
  QString name{};
  int lastProdIndex{};
  while (q.next()) {
    id = q.value(0).toInt();
    name = q.value(1).toString();
    ui->prodNameCB->addItem(name, id);
    if (name == sett.prodName)
      lastProdIndex = ui->prodNameCB->count() - 1;
  }
  ui->prodNameCB->setCurrentIndex(lastProdIndex);
}

void SettingsDialog::editItem() {
  if (ui->prodNameCB->currentText() == "") {
    QMessageBox::warning(this, RU("Ошибка при добавление цепи"),
                         RU("Необходимо выбрать продукт"));
    return;
  }
  QSqlQuery q;
  int tableId = ui->prodNameCB->currentData().toInt();
  if (!q.prepare(Sql::insertCircuitsSql(tableId)))
    Sql::showSqlError(q.lastError());

  CircuitInputDialog dialog(tableId, this);

  if (!dialog.result() == QDialog::Accepted)
    return;

  auto id = dialog.id;
  auto n = dialog.num;
  auto nFrom = dialog.nameFrom;
  auto cFrom = dialog.circuitFrom;
  auto pFrom = dialog.pinFrom;
  auto nTo = dialog.nameTo;
  auto cTo = dialog.circuitTo;
  auto pTo = dialog.pinTo;

  if (!dialog.result() == QDialog::Accepted)
    return;

  if (!q.prepare(Sql::updateCircuitSql(tableId, id, n, nFrom, cFrom, pFrom, nTo, cTo, pTo)))
    Sql::showSqlError(q.lastError());
  if (!model.submitAll())
    Sql::showSqlError(q.lastError());
  model.select();
}
