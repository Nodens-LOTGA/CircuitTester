#include "helpdialog.h"
#include "./ui_helpdialog.h"


HelpDialog::HelpDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::HelpDialog) {
  ui->setupUi(this);

  connect(ui->okPB, SIGNAL(clicked()), this, SLOT(accept()));

}

HelpDialog::~HelpDialog() { delete ui; }