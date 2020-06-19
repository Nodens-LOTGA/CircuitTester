#pragma once
#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui {
class StartUpDialog;
}
QT_END_NAMESPACE

class StartUpDialog : public QDialog {
  Q_OBJECT

public:
  StartUpDialog(QWidget *parent = nullptr);
  ~StartUpDialog();

public slots:
  void tryAccept();

private slots:
  void updateNames();
  void updatePorts();

private:
  Ui::StartUpDialog *ui;
};