#include "CircuitInputDialog.h"
#include "./ui_CircuitInputDialog.h"
#include "sqltools.h"
#include "tools.h"
#include <QMessageBox>
#include <QRegExpValidator>

CircuitInputDialog::CircuitInputDialog(int tableId,
                                       int curId = -1, QWidget * parent)
    : QDialog(parent), ui(new Ui::CircuitInputDialog), currentId(curId) {
  ui->setupUi(this);

  connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(tryAccept()));
  connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);

  QSqlQuery q;
  if (!q.prepare(Sql::insertCircuitsSql(tableId)))
    Sql::showSqlError(q.lastError());

  if (!q.exec(QString("SELECT id, num, nameFrom, circuitFrom, pinFrom, "
                      "nameTo, circuitTo, pinTo FROM "
                      "circuits%1 ORDER BY id")
                  .arg(tableId)))
    Sql::showSqlError(q.lastError());
  q.last();
  id = q.value(0).toInt();
  num = q.value(1).toInt();
  nameFrom = q.value(2).toString();
  circuitFrom = q.value(3).toInt();
  pinFrom = q.value(4).toInt();
  nameTo = q.value(5).toString();
  circuitTo = q.value(6).toInt();
  pinTo = q.value(7).toInt();

  ui->numSB->setValue(num);
  ui->nameFromLE->setText(nameFrom);
  ui->circuitFromSB->setValue(circuitFrom);
  ui->pinFromSB->setValue(pinFrom);
  ui->nameToLE->setText(nameTo);
  ui->circuitToSB->setValue(circuitTo);
  ui->pinToSB->setValue(pinTo);
}

CircuitInputDialog::~CircuitInputDialog() { delete ui; }

void CircuitInputDialog::tryAccept() {
  QString nFrom = ui->nameFromLE->text().simplified(),
          nTo = ui->nameToLE->text().simplified();
  int n = ui->numSB->value(), cFrom = ui->circuitFromSB->value(),
      cTo = ui->circuitToSB->value();
  char pFrom = ui->pinFromSB->value(), pTo = ui->pinToSB->value();

  if (nFrom.isEmpty() || nTo.isEmpty()) {
    QMessageBox::warning(this, RU("Неверное наименование колодки"),
                         RU("Наименование колодки не должно быть пустым"));
    return;
  }

  QSqlQuery q;

  if (!q.exec(QString("SELECT num FROM circuit%1 WHERE num = \'%2\'")
                  .arg(id).arg(n)))
    Sql::showSqlError(q.lastError());
  else {
    while (q.next()) {
      int s = q.value(0).toInt();
      if (s == n) {
        QMessageBox::warning(this, RU("Неверный номер цепи"),
                             RU("Цепь с таким номером уже существует"));
        return;
      }
    }
  }
  if (!q.exec(QString("SELECT nameFrom, pinFrom FROM circuit%1 WHERE nameFrom = \'%2\', pinFrom = \'%3\'").arg(id)
                  .arg(nFrom)
                  .arg(pFrom)))
    Sql::showSqlError(q.lastError());
  else {
    while (q.next()) {
      QString s = q.value(0).toString();
      int i = q.value(1).toInt();
      if (s == nFrom && i == pFrom) {
        QMessageBox::warning(this, RU("Неверное сочетание колодка-"),
                             RU("Цепь с таким номером уже существует"));
        return;
      }
    }
  }
  accept();
}
