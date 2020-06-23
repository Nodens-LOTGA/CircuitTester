#include "./ui_userslistdialog.h"
#include "userslistdialog.h"
#include "Settings.h"
#include <QMessageBox>

UsersListDialog::UsersListDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::UsersListDialog) {
  ui->setupUi(this);

  connect(ui->closePB, SIGNAL(clicked()), this, SLOT(accept()));
  connect(ui->delUserPB, SIGNAL(clicked()), this, SLOT(delUser()));
  connect(ui->delAllPB, SIGNAL(clicked()), this, SLOT(delAll()));

  fillUsers();
}

UsersListDialog::~UsersListDialog() { delete ui; }

void UsersListDialog::delUser() {
  Settings sett; 
  QString userName = ui->usersCB->currentText();
  if (userName == sett.adminName) {
    QMessageBox::warning(this, RU("Ошибка удаления пользователя"),
                         RU("Нельзя удалить администратора"));
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
