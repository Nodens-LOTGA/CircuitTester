#pragma once
#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui {
class UsersListDialog;
}
QT_END_NAMESPACE

class UsersListDialog : public QDialog {
  Q_OBJECT

public:
  UsersListDialog(QWidget *parent = nullptr);
  ~UsersListDialog();

private slots:
  void delUser();
  void delAll();
  void fillUsers();

private:
  Ui::UsersListDialog *ui;
};