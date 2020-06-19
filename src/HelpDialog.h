#pragma once
#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui {
class HelpDialog;
}
QT_END_NAMESPACE

class HelpDialog : public QDialog {
  Q_OBJECT

public:
  HelpDialog(QWidget *parent = nullptr);
  ~HelpDialog();

private:
  Ui::HelpDialog *ui;
};