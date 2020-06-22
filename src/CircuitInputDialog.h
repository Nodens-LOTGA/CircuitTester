#pragma once
#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui {
class CircuitInputDialog;
}
QT_END_NAMESPACE

class CircuitInputDialog : public QDialog {
  Q_OBJECT

public:
  CircuitInputDialog(int tableId, int curId = -1, QWidget *parent = nullptr);
  ~CircuitInputDialog();

  QString nameFrom = "X1", nameTo = "X2";
  int num = 1, circuitFrom = 1, circuitTo = 2, id = 0;
  char pinFrom = 1, pinTo = 2;
  int currentId = -1;
private slots:
  void tryAccept();

private:
  Ui::CircuitInputDialog *ui;
};