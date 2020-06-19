#include "ProductInputDialog.h"
#include "./ui_ProductInputDialog.h"
#include "tools.h"
#include <QMessageBox>
#include <QRegExpValidator>

ProductInputDialog::ProductInputDialog(const QString &lastArt,
                                       const QString &lastName, QWidget *parent)
    : QDialog(parent), ui(new Ui::ProductInputDialog) {
  ui->setupUi(this);

  connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(tryAccept()));
  connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  ui->nameLE->setValidator(new QRegExpValidator(QRegExp(tr("[^|]+")), this));
  ui->artLE->setValidator(new QRegExpValidator(QRegExp(tr("[\\d-]+")), this));
  setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
  ui->nameLE->setText(lastName);
  ui->artLE->setText(lastArt);
}

ProductInputDialog::~ProductInputDialog() { delete ui; }

void ProductInputDialog::tryAccept() {
  QString n = ui->nameLE->text().simplified(),
          a = ui->artLE->text().simplified();
  if (n.isEmpty()) {
    QMessageBox::warning(this, RU("Неверное наименование"),
                         RU("Наименование не должно быть пустым"));
    return;
  }
  if (a.isEmpty()) {
    QMessageBox::warning(this, RU("Неверный артикул"),
                         RU("Артикул не должен быть пустым"));
    return;
  }
  name = n;
  art = a;

  accept();
}
