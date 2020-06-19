#pragma once
#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui {
class ProductInputDialog;
}
QT_END_NAMESPACE

class ProductInputDialog : public QDialog {
  Q_OBJECT

public:
  ProductInputDialog(const QString &lastArt, const QString &lastName,
                     QWidget *parent = nullptr);
  ~ProductInputDialog();

  QString name, art;
private slots:
  void tryAccept();

private:
  Ui::ProductInputDialog *ui;
};