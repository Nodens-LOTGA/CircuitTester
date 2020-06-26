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
  void tryReject();
  void setLabelPrinter();
  void setReportPrinter();
  void setReportDir();
  void addItem();
  void delItem();
  void updateTables();
  void resetSett();
  void delUsers();
  void loadSettings(Settings &sett);
  void changeNum();
  void addProd();
  void delProd();
  void updateProducts();
  void updateRelationsTable();
  void updateCircuitsTable();
  void pickLabel();
  void editProd();
    
private:
  Ui::SettingsDialog *ui;

  int curNum{};
  QString reportDir{}, prodName{}, labelPrinterName{}, reportPrinterName{},
      label{};
  QSqlRelationalTableModel circuitsModel, relationsModel;
};