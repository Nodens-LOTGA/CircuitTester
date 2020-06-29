#pragma once
#include <QDialog>
#include <QtHelp>
#include "HelpBrowser.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class HelpDialog; 
}
QT_END_NAMESPACE

class HelpDialog : public QDialog {
  Q_OBJECT

public:
  HelpDialog(const QString & collection, QWidget *parent = nullptr);
  ~HelpDialog();

private:
  Ui::HelpDialog *ui;

  QHelpEngine *helpEngine;
};