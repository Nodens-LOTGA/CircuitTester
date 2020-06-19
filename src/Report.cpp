#include "Report.h"
#include "ReportDelegate.h"
#include "tools.h"
#include <QByteArray>
#include <QDate>
#include <QFile>
#include <QFontMetrics>
#include <QHeaderView>
#include <QStringList>
#include <QTextStream>
#include <QTime>
#include <QPair>

QString Item::statusToQStr(const Item::Status stat) {
  switch (stat) {
  case Item::Status::Ok:
    return QString("OK");
  case Item::Status::Open:
    return QString("Open");
  case Item::Status::Short:
    return QString("Short");
  case Item::Status::Error:
    return QString("Error");
  }
}

QTableWidget *Report::createTableWidget(QWidget *parent, QSize size) {
  int rowCount = 9 + items.size(), columnCount = 2;

  QTableWidget *table = new QTableWidget(rowCount, columnCount, parent);
  table->setGeometry(0, 0, size.width(), size.height());
  table->verticalHeader()->setVisible(false);
  table->horizontalHeader()->setVisible(false);
  table->horizontalHeader()->resizeSection(0, size.width() / 2);
  table->horizontalHeader()->resizeSection(1, size.width() / 2 - 3);
  table->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
  table->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  table->setSpan(0, 0, 1, 2);
  table->setSpan(1, 0, 1, 2);
  table->setShowGrid(false);
  table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  table->setFocusPolicy(Qt::NoFocus);
  table->setSelectionMode(QAbstractItemView::NoSelection);
  table->setItemDelegate(new ReportDelegate(table));

  QTableWidgetItem *itemLogo = new QTableWidgetItem;
  QPixmap img = QPixmap(":/img/logo.png").scaledToWidth(table->width() - 4);
  itemLogo->setData(Qt::DecorationRole, img);
  table->setItem(0, 0, itemLogo);
  table->setRowHeight(0, img.height());

  QTableWidgetItem *itemHeader =
      new QTableWidgetItem(RU("Протокол проверки жгута проводов"));
  itemHeader->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  table->setItem(1, 0, itemHeader);

  addRow(table, 2, RU("Отчёт №"),
         QString::number(curNum).rightJustified(6, '0'));
  addRow(table, 3, RU("Изделие"), prodName);
  addRow(table, 4, RU("Дата"), date);
  addRow(table, 5, RU("Время"), time);
  addRow(table, 6, RU("Ф.И.О"), name);
  addRow(table, 7, RU("Замеряемые параметры"), "");

  QVector<QPair<QString, QString>> errorStrings;
  bool internalError = false;
  int rowIndex = 8;
  auto i = items.constBegin();
  while (i != items.constEnd()) {
    for (auto& j : i.value()) {
      if (j.stat != Item::Status::Ok){
        internalError = true;
        for (auto& k : j.relations) {
          if (k.stat != Item::Status::Ok)
            errorStrings.push_back(QPair(Item::statusToQStr(k.stat), j.name + " <---> " + k.name));
        }
      }
    }
    addRow(table, rowIndex, RU("Цепь № ") + QString::number(i.key()), internalError ? Item::statusToQStr(Item::Status::Error)
               : Item::statusToQStr(Item::Status::Ok));
    rowIndex++;
    if (internalError)
      error = true;
    internalError = false;
    i++;
  }

  addRow(table, rowCount - 1, RU("Результат проверки"),
         error ? Item::statusToQStr(Item::Status::Error)
               : Item::statusToQStr(Item::Status::Ok));
  if (error) {
    table->setRowCount(++rowCount);
    addRow(table, rowCount - 1, RU("Неисправности"), RU("Разъём, контакт"));
    for (auto &i : errorStrings) {
        table->setRowCount(++rowCount);
        addRow(table, rowCount - 1,  i.first, i.second);
      }
    }

  return table;
}

QTableWidget *Report::createLabelTableWidget(QWidget *parent, QSize size) {
  QTableWidget *label = new QTableWidget(4, 2, parent);
  label->setGeometry(0, 0, size.width(), size.height());
  label->verticalHeader()->setVisible(false);
  label->horizontalHeader()->setVisible(false);
  label->horizontalHeader()->resizeSection(0, size.width() / 3);
  label->horizontalHeader()->resizeSection(1, 2 * size.width() / 3 - 3);
  label->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
  label->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  // label->setShowGrid(false);
  label->setEditTriggers(QAbstractItemView::NoEditTriggers);
  label->setFocusPolicy(Qt::NoFocus);
  label->setSelectionMode(QAbstractItemView::NoSelection);

  QTableWidget *table = this->createTableWidget(nullptr, size);
  for (int i = 0; i < 4; i++) {
    addRow(label, i, table->item(i + 2, 0)->text(),
           table->item(i + 2, 1)->text());
  }
  delete table;
  return label;
}

bool Report::saveTo(const QString &file) {
  QTableWidget *table = this->createTableWidget(nullptr, QSize(330, 330));
  QFile f(file);
  if (!f.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Truncate))
    return false;
  QTextStream out(&f);
  int rows = table->rowCount();
  out << "\"" << table->item(1, 0)->text() << "\";\"\"" << endl;
  for (int i = 2; i < rows; i++) {
    out << "\"" << table->item(i, 0)->text() << "\";\""
        << table->item(i, 1)->text() << "\"" << endl;
  }
  return true;
}

bool Report::createZplLabel(const QString &file, QByteArray &buf) {
  QFile f(file);
  if (!f.open(QIODevice::ReadWrite | QIODevice::Text))
    return false;
  while (!f.atEnd()) {
    buf += f.readLine();
  }
  auto pn = prodName.section(" ", 0, 0);
  QString code = QString(RU("Отчёт №: %1; Изделие: %2; Дата: %3; Время: %4"))
                     .arg(curNum)
                     .arg(pn)
                     .arg(date)
                     .arg(time);
  auto isoDate(date);   
  isoDate.replace('.', '-');
  buf.replace(
      "${NUM}",
              QString::number(curNum).rightJustified(6, '0').toUtf8().data());
  buf.replace("${PROD}", pn.toUtf8().data());
  buf.replace("${DATE}", isoDate.toUtf8().data());  
  buf.replace("${TIME}", time.toUtf8().data()); 
  auto first = buf.indexOf("${C}");
  auto second = buf.indexOf("{/C}");
  buf.replace(first, second - first + 4, code.toUtf8().data());
  return true;
}

void Report::addRow(QTableWidget *table, int row, QString str1, QString str2) {
  QTableWidgetItem *item1 = new QTableWidgetItem(str1);
  table->setItem(row, 0, item1);
  QTableWidgetItem *item2 = new QTableWidgetItem(str2);
  table->setItem(row, 1, item2);
}
