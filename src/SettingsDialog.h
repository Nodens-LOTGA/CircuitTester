#pragma once
#include <QDialog>
#include <QPrinter>
#include <Report.h>
#include <QComboBox>
#include <QtSql>
#include "Settings.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class SettingsDialog;
}
QT_END_NAMESPACE

class SettingsDialog : public QDialog {
  Q_OBJECT

public:
  SettingsDialog(QWidget *parent = nullptr);
  ~SettingsDialog();

private slots:
  void tryAccept();
  void setLabelPrinter();
  void setReportPrinter();
  void setReportDir();
  void addItem();
  void delItem();
  void populateTable();
  void resetSett();
  void delUsers();
  void loadSettings(Settings &sett);
  void changeAdminPass();
  void changeNum();
  void addProd();
  void delProd();
  void updateProducts();
  void editItem();

private:
  Ui::SettingsDialog *ui;

  int curNum{};
  QString reportDir{}, prodName{}, labelPrinterName{}, reportPrinterName{},
      adminPass{};
  QSqlRelationalTableModel model;
};