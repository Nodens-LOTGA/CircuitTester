#include "settingsdialog.h"
#include "./ui_settingsdialog.h"
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
#include <QSpinBox>
#include <QStringList>
#include "UsersListDialog.h"
#include "SpinBoxDelegate.h"

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
  connect(ui->resetSettPB, SIGNAL(clicked()), this, SLOT(resetSett()));
  connect(ui->delUsersPB, SIGNAL(clicked()), this, SLOT(delUsers()));
  connect(ui->changeAdminPassPB, SIGNAL(clicked()), this,
          SLOT(changeAdminPass()));
  connect(ui->changeNumPB, SIGNAL(clicked()), this, SLOT(changeNum()));
  connect(ui->addProdTB, SIGNAL(clicked()), this, SLOT(addProd()));
  connect(ui->delProdTB, SIGNAL(clicked()), this, SLOT(delProd()));
  connect(ui->prodNameCB, SIGNAL(currentIndexChanged(int)),
          SLOT(updateTables()));
  connect(ui->addTB, SIGNAL(clicked()), this, SLOT(addItem()));
  connect(ui->delTB, SIGNAL(clicked()), this, SLOT(delItem()));
  connect(ui->pageCB, SIGNAL(currentIndexChanged(int)), this,
          SLOT(updateTables()));

  loadSettings(Settings());

  updateProducts();
  updateTables();
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
    QMessageBox::warning(this, RU("Ошибка при добавление"),
                         RU("Необходимо выбрать продукт"));
    return;
  }
  QSqlQuery q;
  int tableId = ui->prodNameCB->currentData().toInt();

  int lastPin{-1}, lastCircuit{-1};

  if (!q.exec(QString("SELECT pin FROM circuits%1 ORDER BY pin").arg(tableId)))
    Sql::showSqlError(q.lastError());
  QList<int> pins;
  while (q.next())
    pins.push_back(q.value(0).toInt());
  if (q.last())
    lastPin = q.value(0).toInt();
  if (!q.exec(QString("SELECT circuit FROM circuits%1 ORDER BY circuit")
                  .arg(tableId)))
    Sql::showSqlError(q.lastError());
  QList<int> circuits;
  while (q.next())
    circuits.push_back(q.value(0).toInt());
  if (q.last())
    lastCircuit = q.value(0).toInt();

  if (ui->stackedWidget->currentIndex() == 0) {
    lastPin++;
    lastCircuit++;
    if (!q.prepare(Sql::insertCircuitsSql(tableId)))
      Sql::showSqlError(q.lastError());
    Sql::addCircuit(q, lastPin, lastCircuit, "XX");
    if (!circuitsModel.submitAll())
      Sql::showSqlError(q.lastError());
    static_cast<CircuitsDelegate *>(ui->circuitTV->itemDelegate())->pins = pins;
    static_cast<CircuitsDelegate *>(ui->circuitTV->itemDelegate())->circuits =
        circuits;
    circuitsModel.select();
  } else if (ui->stackedWidget->currentIndex() == 1) {
    if (pins.size() < 2) {
      QMessageBox::warning(this, RU("Ошибка при добавление цепи"),
                           RU("Для цепи необходимо как минимум 2 контакта"));
      return;
    }
    if (!q.prepare(Sql::insertRealationsSql(tableId)))
      Sql::showSqlError(q.lastError());
    Sql::addRelation(q, 1, lastPin, lastPin);
    if (!relationsModel.submitAll())
      Sql::showSqlError(q.lastError());
    relationsModel.select();
  }
}

void SettingsDialog::delItem() {
  if (ui->prodNameCB->currentText() == "") {
    QMessageBox::warning(this, RU("Ошибка при удаление цепи"),
                         RU("Необходимо выбрать продукт"));
    return;
  }
  QSqlQuery q;

  if (ui->stackedWidget->currentIndex() == 0) {
    auto &p =
        static_cast<CircuitsDelegate *>(ui->circuitTV->itemDelegate())->pins;
    auto &c = static_cast<CircuitsDelegate *>(ui->circuitTV->itemDelegate())
                  ->circuits;
    for (auto &i : ui->circuitTV->selectionModel()->selectedIndexes()) {
      p.removeOne(
          circuitsModel.data(circuitsModel.index(i.row(), 0), Qt::EditRole)
              .toInt());
      c.removeOne(
          circuitsModel.data(circuitsModel.index(i.row(), 1), Qt::EditRole)
              .toInt());
      circuitsModel.removeRow(i.row());
    }
    if (!circuitsModel.submitAll())
      Sql::showSqlError(q.lastError());
    circuitsModel.select();
  } else if (ui->stackedWidget->currentIndex() == 1) {
    for (auto &i : ui->relationTV->selectionModel()->selectedIndexes())
      relationsModel.removeRow(i.row());
    if (!relationsModel.submitAll())
      Sql::showSqlError(q.lastError());
    relationsModel.select();
  }
}

void SettingsDialog::updateTables() {
  if (ui->prodNameCB->currentText() == "") {
    return;
  }

  if (ui->pageCB->currentIndex() == 0)
    updateCircuitsTable();
  else
    updateRelationsTable();

  prodName = ui->prodNameCB->currentText();
}

void SettingsDialog::resetSett() {
  Settings sett(true);
  loadSettings(sett);
}

void SettingsDialog::delUsers() {
  UsersListDialog dialog(this);
  dialog.exec();
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
    if (!q.exec(Sql::relationsSql(id)))
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
  circuitsModel.clear();
  relationsModel.clear();
  QSqlQuery q;
  if (!q.exec("DROP TABLE circuits" + QString::number(id)))
    Sql::showSqlError(q.lastError());
  if (!q.exec("DROP TABLE relations" + QString::number(id)))
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

void SettingsDialog::updateCircuitsTable() {
  int id = ui->prodNameCB->currentData().toInt();

  circuitsModel.setTable("circuits" + QString::number(id));
  circuitsModel.setEditStrategy(QSqlTableModel::OnFieldChange);
  circuitsModel.setHeaderData(0, Qt::Horizontal, RU("Контакт\nстенда"));
  circuitsModel.setHeaderData(1, Qt::Horizontal, RU("№ контакта"));
  circuitsModel.setHeaderData(2, Qt::Horizontal, RU("Колодка"));
  circuitsModel.select();
  ui->circuitTV->setModel(&circuitsModel);
  ui->circuitTV->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  ui->circuitTV->setItemDelegate(new CircuitsDelegate(ui->circuitTV));

  QSqlQuery q;
  if (!q.exec(QString("SELECT pin FROM circuits%1 ORDER BY pin").arg(id)))
    Sql::showSqlError(q.lastError());
  QList<int> pins;
  while (q.next())
    pins.push_back(q.value(0).toInt());
  if (!q.exec(QString("SELECT circuit FROM circuits%1 ORDER BY circuit")
                  .arg(id)))
    Sql::showSqlError(q.lastError());
  QList<int> circuits;
  while (q.next())
    circuits.push_back(q.value(0).toInt());
  static_cast<CircuitsDelegate *>(ui->circuitTV->itemDelegate())->pins = pins;
  static_cast<CircuitsDelegate *>(ui->circuitTV->itemDelegate())->circuits =
      circuits;
}

void SettingsDialog::updateRelationsTable() {
  int id = ui->prodNameCB->currentData().toInt();

  relationsModel.setTable("relations" + QString::number(id));
  relationsModel.setEditStrategy(QSqlTableModel::OnFieldChange);
  relationsModel.setHeaderData(1, Qt::Horizontal, RU("№ Цепи"));
  relationsModel.setHeaderData(2, Qt::Horizontal, RU("Контакт\nстенда №1"));
  relationsModel.setHeaderData(3, Qt::Horizontal, RU("Контакт\nстенда №2"));
  relationsModel.setRelation(
      2, QSqlRelation("circuits" + QString::number(id), "pin", "pin"));
  relationsModel.setRelation(
      3, QSqlRelation("circuits" + QString::number(id), "pin", "pin"));
  relationsModel.select();
  ui->relationTV->setModel(&relationsModel);
  ui->relationTV->horizontalHeader()->setSectionResizeMode(
      QHeaderView::Stretch);
  ui->relationTV->setItemDelegate(new QSqlRelationalDelegate(ui->relationTV));
  ui->relationTV->setItemDelegateForColumn(1, new SpinBoxDelegate(ui->relationTV));
  ui->relationTV->hideColumn(0);
}
