#include "startupdialog.h"
#include "./ui_startupdialog.h"
#include "Settings.h"
#include "tools.h"
#include <QCryptographicHash>
#include <QMessageBox>
#include <QRegExpValidator>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QSettings>

StartUpDialog::StartUpDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::StartUpDialog) {
  ui->setupUi(this);

  connect(ui->enterPB, SIGNAL(clicked()), this, SLOT(tryAccept()));

  ui->pwLE->setValidator(new QRegExpValidator(QRegExp(".{8,32}"), this));
  ui->nameLE->setValidator(
      new QRegExpValidator(QRegExp(tr("[\\w\\s]+")), this));

  //QSettings set;
  //set.clear();

  Settings sett;
  ui->newUserChkB->setEnabled(sett.newUsers);
  updateNames();
  updatePorts();
}

StartUpDialog::~StartUpDialog() { delete ui; }

void StartUpDialog::updateNames() {
  Settings sett;
  int lastUserIndex{};
  QMap<QString, QVariant>::const_iterator i = sett.users.constBegin();
  while (i != sett.users.constEnd()) {
    ui->userCB->addItem(i.key(), i.value());
    if (i.key() == sett.userName)
      lastUserIndex = ui->userCB->count() - 1;
    i++;
  }
  ui->userCB->setCurrentIndex(lastUserIndex);
}

void StartUpDialog::updatePorts() {
  auto ports = QSerialPortInfo::availablePorts();
  for (auto &i : ports)
    ui->portCB->addItem(i.portName());
}

void StartUpDialog::tryAccept() {
  Settings sett;
  QString pw = ui->pwLE->text();
  if (pw.length() < 8) {
    QMessageBox::warning(
        this, RU("Неверный пароль"),
        RU("Пароль должен состоять не менее чем из 8 символов"));
    return;
  }
  if (ui->newUserChkB->isChecked()) {
    QString newUserName = ui->nameLE->text().simplified();
    if (newUserName.isEmpty()) {
      QMessageBox::warning(this, RU("Неверный пользователь"),
                           RU("Имя пользователя не должно быть пустым"));
      return;
    }
    if (sett.users.contains(newUserName)) {
      QMessageBox::warning(this, RU("Неверный пользователь"),
                           RU("Пользователь с таким именем уже существует"));
      return;
    }
    sett.users[newUserName] = pw;
    sett.userName = newUserName;
    sett.isAdmin = false;
    sett.save();
    updateNames();
  } else {
    if (ui->userCB->currentText() == "") {
      QMessageBox::warning(this, RU("Неверный пользователь"),
                           RU("Пожалуйста, выберите пользователя"));
      return;
    }
    QString userName = ui->userCB->currentText();

    if (sett.users.contains(userName)) {
      if (sett.users[userName] != pw) {
        QMessageBox::warning(this, RU("Неверный пароль"),
                             RU("Пароль не совпадает"));
        return;
      } else {
        sett.userName = userName;
      }
    }
    if (userName == sett.adminName)
      sett.isAdmin = true;
    else
      sett.isAdmin = false;
  }
  //TODO:
  /*if (ui->portCB->currentText() == "") {
    QMessageBox::warning(this, RU("Неверный порт"),
                         RU("Пожалуйста, выберетие порт"));
    return;
  } else
    sett.portName = ui->portCB->currentText();*/
  sett.save();
  accept();
}