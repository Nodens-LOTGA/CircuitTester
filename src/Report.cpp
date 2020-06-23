#include "Report.h"
#include "ReportDelegate.h"
#include "sqltools.h"
#include "tools.h"
#include <QByteArray>
#include <QDate>
#include <QFile>
#include <QFontMetrics>
#include <QHeaderView>
#include <QPair>
#include <QSql>
#include <QStringList>
#include <QTextStream>
#include <QTime>
#include <boost/graph/breadth_first_search.hpp>
#include "boost/array.hpp"

using namespace rep;

QTableWidget *Report::createTableWidget(QWidget *parent, QSize size) {
  int rowCount = 9 + circuits.size(), columnCount = 2;

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
  auto i = circuits.constBegin();
  while (i != circuits.constEnd()) {
    for (auto &j : i.value()) {
      edge_t e = boost::edge(j.first, j.second, graph).first;
      auto status = boost::get(&Edge::status, graph, e);
      if (status != Status::Ok) {
        internalError = true;
        typename boost::graph_traits<Graph>::out_edge_iterator out_i, out_end;
        for (std::tie(out_i, out_end) = boost::out_edges(j.first, graph);
             out_i != out_end; out_i++) {
          auto stat = boost::get(&Edge::status, graph, *out_i);
          if (stat != Status::Ok)
            errorStrings.push_back(
                QPair(statusToQStr(stat), graph[j.first].name + ":" +
                                              QString::number(graph[j.first].circuit) + RU(" ⇔ ") +
                          graph[j.second].name + ":" +
                          QString::number(graph[j.second].circuit)));
        }
      }
    }
    addRow(table, rowIndex, RU("Цепь № ") + QString::number(i.key()),
           internalError ? statusToQStr(Status::Error)
                         : statusToQStr(Status::Ok));
    rowIndex++;
    if (internalError)
      error = true;
    internalError = false;
    i++;
  }

  addRow(table, rowCount - 1, RU("Результат проверки"),
         error ? statusToQStr(Status::Error) : statusToQStr(Status::Ok));
  if (error) {
    table->setRowCount(++rowCount);
    addRow(table, rowCount - 1, RU("Неисправности"), RU("Разъём, контакт"));
    for (auto &i : errorStrings) {
      table->setRowCount(++rowCount);
      addRow(table, rowCount - 1, i.first, i.second);
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
  buf.replace("${NUM}",
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

bool Report::fill() {
  QSqlQuery q;

  int tableId{-1};
  if (!q.exec(QString("SELECT id, name FROM products WHERE name = \'%1\'")
                  .arg(prodName)))
    Sql::showSqlError(q.lastError());
  else {
    while (q.next()) {
      QString s = q.value(1).toString();
      if (q.value(1).toString() == prodName) {
        tableId = q.value(0).toInt();
        break;
      }
    }
  }

  if (tableId == -1)
    return false;

  graph.clear();
  circuits.clear();
  pins.clear();
  if (!q.exec(QString("SELECT pin, circuit, name FROM circuits%1 ORDER BY pin")
                  .arg(tableId)))
    Sql::showSqlError(q.lastError());
  while (q.next()) {
    char pin = static_cast<char>(q.value(0).toInt());
    vertex_t v = boost::add_vertex(
        Vertex{q.value(2).toString(), q.value(1).toInt(), pin}, graph);
    pins[pin] = v;
  }
  if (!q.exec(
          QString("SELECT num, pinFrom, pinTo FROM relations%1 ORDER BY num").arg(tableId)))
    Sql::showSqlError(q.lastError());
  while (q.next()) {
    char pinFrom = static_cast<char>(q.value(1).toInt());
    char pinTo = static_cast<char>(q.value(2).toInt());
    auto v1 = pins[pinFrom];
    auto v2 = pins[pinTo];
    boost::add_edge(v1, v2, graph);
    circuits[q.value(0).toInt()].push_back(QPair(v1, v2));
  }
  return true;
}

bool rep::Report::checkAll(SerialPort &port) {
  QByteArray rb, wb;
  const char cb[] = {0x23, 0x55, 0x48};
  for (auto &i : circuits) {
    for (auto &k : i) {
      wb.clear();
      wb.append(cb);
      wb += graph[k.first].pin;
      wb += graph[k.second].pin;
      int attempt{};
      do {
        if (attempt == 3) {
          // TODO:
          return false;
        }
        port.write(wb.data(), wb.size());
        port.read(rb.data(), 255);
        attempt++;
        QThread::msleep(50);
      } while (!rb.startsWith(wb.left(4)));
      for (int j = 4; j < rb.size(); j++) {
        vertex_t v = pins[rb.at(j)];
        auto [e, exist] = boost::edge(k.first, v, graph);
        bool found{false};
        if (!exist) {
          boost::array<vertex_t, 256> predecessors;
          predecessors[k.first] = k.first;
          boost::breadth_first_search(
              graph, k.first,
              boost::visitor(boost::make_bfs_visitor(boost::record_predecessors(
                  predecessors.begin(), boost::on_tree_edge{}))));
          for (auto &n : predecessors)
            if (n == v) {
              found = true;
              break;
            }
          if (!found) {
            boost::add_edge(k.first, v, Edge{Status::Short}, graph);
          }
        }
        if (exist || found) {
          boost::add_edge(k.first, v, Edge{Status::Ok}, graph);
        }
      }
    }
  }
  return true;
}
