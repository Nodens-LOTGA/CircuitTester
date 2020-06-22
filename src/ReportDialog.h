#pragma once
#include <QDialog>
#include <QTableWidget>
#include "Report.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class ReportDialog;
}
QT_END_NAMESPACE

class ReportDialog : public QDialog {
  Q_OBJECT

public:
  ReportDialog(rep::Report &report, QWidget *parent = nullptr);
  ~ReportDialog();


private slots:
  void printReport();
  void printLabel();

private:
  Ui::ReportDialog *ui;
  rep::Report report;
};