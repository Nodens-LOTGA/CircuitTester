#include "./ui_userslistdialog.h"
#include "userslistdialog.h"
#include "Settings.h"
#include <QMessageBox>
#include <QInputDialog>

UsersListDialog::UsersListDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::UsersListDialog) {
  ui->setupUi(this);

  connect(ui->closePB, SIGNAL(clicked()), this, SLOT(accept()));
  connect(ui->delUserPB, SIGNAL(clicked()), this, SLOT(delUser()));
  connect(ui->delAllPB, SIGNAL(clicked()), this, SLOT(delAll()));
  connect(ui->editNamePB, SIGNAL(clicked()), this, SLOT(editName()));
  connect(ui->editPasswordPB, SIGNAL(clicked()), this, SLOT(editPassword()));

  fillUsers();
}

UsersListDialog::~UsersListDialog() { delete ui; }

void UsersListDialog::delUser() {
  Settings sett; 
  QString userName = ui->usersCB->currentText();
  if (userName == sett.adminName) {
    QMessageBox::warning(this, RU("������ �������� ������������"),
                         RU("������ ������� ��������������"));
    return;
  }
  sett.users.remove(userName);
  sett.save();
  fillUsers();
}

void UsersListDialog::delAll() {
  Settings sett;
  sett.discardUsers();
  fillUsers();
}

void UsersListDialog::fillUsers() {
  ui->usersCB->clear();
  Settings sett;
  QMap<QString, QVariant>::const_iterator i = sett.users.constBegin();
  while (i != sett.users.constEnd()) {
    ui->usersCB->addItem(i.key(), i.value());
    i++;
  }
}

void UsersListDialog::editPassword() {
  Settings sett;
  bool ok;
  QString newPass = QInputDialog::getText(
      this, RU("������� ����� ������"), RU("����� ������"),
      QLineEdit::PasswordEchoOnEdit, "", &ok);
  if (ok)
    if (newPass.length() < 8) {
      QMessageBox::warning(
          this, RU("�������� ������"),
          RU("������ ������ �������� �� ����� ��� �� 8 ��������"));
      return;
    }
  QString userName = ui->usersCB->currentText();
  sett.users[userName] = newPass;
  sett.save();
}

void UsersListDialog::editName() {
  Settings sett;
  bool ok;
  QString userName = ui->usersCB->currentText();
  if (userName == sett.adminName) {
    QMessageBox::warning(this, RU("�������� ������������"),
                         RU("������ �������� ��� ��������������"));
    return;
  }
  QString newName = QInputDialog::getText(
      this, RU("������� ����� ���"), RU("����� ���"),
                        QLineEdit::Normal, userName, &ok)
                        .simplified();
  if (newName.isEmpty()) {
    QMessageBox::warning(this, RU("�������� ������������"),
                         RU("��� ������������ �� ������ ���� ������"));
    return;
  }
  if (sett.users.contains(newName)) {
    QMessageBox::warning(this, RU("�������� ������������"),
                         RU("������������ � ����� ������ ��� ����������"));
    return;
  }
  auto pw = sett.users.take(userName);
  sett.users[newName] = pw;
  sett.save();
  fillUsers();
}
