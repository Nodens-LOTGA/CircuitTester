#include "Report.h"
#include "ReportDelegate.h"
#include "boost/array.hpp"
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
#include <boost/graph/graph_utility.hpp>
#include <boost/graph/visitors.hpp>
#include <boost/property_map/property_map.hpp>

using namespace rep;

QTableWidget *Report::createTableWidget(QWidget *parent, QSize size,
                                        bool colorful) {
  int rowCount = 9 + circuits.size(), columnCount = 2;

  QTableWidget *table = new QTableWidget(rowCount, columnCount, parent);
  table->setGeometry(0, 0, size.width(), size.height());
  table->verticalHeader()->setVisible(false);
  table->horizontalHeader()->setVisible(false);
  table->horizontalHeader()->resizeSection(0, std::ceill(size.width() / 2.0));
  table->horizontalHeader()->resizeSection(1,
                                           std::floor(size.width() / 2.0) - 1);
  table->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
  table->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  table->setSpan(0, 0, 1, 2);
  table->setSpan(1, 0, 1, 2);
  table->setShowGrid(false);
  table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  table->setFocusPolicy(Qt::NoFocus);
  table->setSelectionMode(QAbstractItemView::NoSelection);
  table->setItemDelegate(new ReportDelegate(table, colorful));
  table->setFrameShape(QFrame::NoFrame);

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
  addRow(table, 7, RU("Результат проверки"), "");
  addRow(table, 8, RU("Замеряемые параметры"), "");

  QSet<QPair<QString, QString>> errorStrings;
  bool internalError = false;
  int rowIndex = 9;
  auto i = circuits.constBegin();
  while (i != circuits.constEnd()) {
    for (auto &j : i.value()) {
      // edge_t e = boost::edge(j.first, j.second, graph).first;
      // auto status = boost::get(&Edge::status, graph, e);
      // if (status != Status::Ok) {
      typename boost::graph_traits<Graph>::out_edge_iterator out_i, out_end;
      for (std::tie(out_i, out_end) = boost::out_edges(j.first, graph);
           out_i != out_end; out_i++) {
        auto stat = boost::get(&Edge::status, graph, *out_i);
        if (stat != Status::Ok) {
          internalError = true;
          auto target = boost::target(*out_i, graph);
          auto source = boost::source(*out_i, graph);
          errorStrings.insert(QPair(
              statusToQStr(stat), graph[source].name + ":" +
                                      QString::number(graph[source].circuit) +
                                      RU(" - ") + graph[target].name + ":" +
                                      QString::number(graph[target].circuit)));
        }
      }
      //}
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
  table->item(7, 1)->setText(error ? statusToQStr(Status::Error)
                                   : statusToQStr(Status::Ok));
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
  if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
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
              QString::number(curNum).rightJustified(6, '0').toLocal8Bit());
  buf.replace("${PROD}", pn.toLocal8Bit());
  buf.replace("${DATE}", isoDate.toLocal8Bit());
  buf.replace("${TIME}", time.toLocal8Bit());
  auto first = buf.indexOf("${C}");
  auto second = buf.indexOf("{/C}");
  buf.replace(first, second - first + 4, code.toLocal8Bit());
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
          QString("SELECT num, pinFrom, pinTo FROM relations%1 ORDER BY num")
              .arg(tableId)))
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
  char rb[256]{}, wb[5];
  int bytesRead{};
  const char cb[] = {0x23, 0x55, 0x48};
  for (auto &i : circuits) {
    for (auto &k : i) {
      memset(rb, 0, sizeof(rb));
      memset(wb, 0, sizeof(wb));
      memcpy(wb, cb, sizeof(cb));
      wb[3] = graph[k.first].pin;
      wb[4] = graph[k.second].pin;
      int attempt{};
      do {
        if (attempt == 3) {
          // TODO:
          return false;
        }
        port.write(wb, sizeof(wb));
        QThread::msleep(250 * (attempt + 1));
        bytesRead = port.read(rb, 256);
        attempt++;
      } while (!(rb[0] == wb[0] && rb[1] == wb[1] && rb[2] == wb[2] &&
                 rb[3] == wb[3]));

      std::vector<vertex_t> predecessors(boost::num_vertices(graph), -1);
      predecessors[k.first] = k.first;
      boost::breadth_first_search(
          graph, k.first,
          boost::visitor(boost::make_bfs_visitor(boost::record_predecessors(
              &predecessors[0], boost::on_tree_edge()))));
#ifndef QT_NO_DEBUG_OUTPUT
      std::cout << "\nGraph Before:\n";
      boost::print_graph(graph);
      std::cout << "Vertex: " << k.first << " : " << (int)graph[k.first].pin
                << "\nPredecessors:";
      for (auto &i : predecessors)
        std::cout << i << " ";
      std::cout << "\n";
#endif
      for (int j = 4; j < bytesRead; j++) { //Поиск полученных контактов в цепи
        if (!pins.contains(rb[j])) { //Если контакт не известен - замыкание
          vertex_t w = boost::add_vertex(
              Vertex{RU("?(КС:") + QString::number(rb[j]) + ")", -1, rb[j]},
              graph);
          boost::add_edge(k.first, w, Edge{Status::Short}, graph);
          continue;
        }
        vertex_t v = pins[rb[j]];
        auto [e, exist] = boost::edge(k.first, v, graph); //Проверка связи
        bool found{false};
        if (!exist) { //Если такой связи нет
          if (predecessors[v] != -1) //Поиск среди достигаемых контактов
            found = true;
          if (!found) { //Если контакт не достигаем - замыкание
            boost::add_edge(k.first, v, Edge{Status::Short}, graph);
            continue;
          }
        }
        //Связь, которая должна быть проверена/контакт достигаема?
        if (exist || found) {
          Status pStatus = Status::Ok;
          if (found) { //Достигаем? Статус = Годен/Замыкание
            pStatus =
                graph[boost::edge(v, predecessors[v], graph).first].status;
            pStatus = (pStatus == Status::Open ? Status::Ok : pStatus);
          }
          //Добавляем связь с достигаемым контактом
          auto [e1, added] = boost::add_edge(k.first, v, Edge{pStatus}, graph);
          if (!added) { //Если связь сущесвует
            //И это связь, которая должна быть проверена
            if (graph[e1].status == Status::Open)
              //Меняем статус на "Годен" ("Обрыв" останется в том случае, если с
              //порта не пришло необходимого контатка
              graph[e1].status = Status::Ok;
          }
        }
      }
      if (std::find(rb + 4, rb + bytesRead, graph[k.second].pin) ==
          rb + bytesRead) {
        graph[boost::edge(k.first, k.second, graph).first].status =
            Status::Open;
      }
#ifndef QT_NO_DEBUG_OUTPUT
      std::cout << "Graph After:\n";
      boost::print_graph(graph);
#endif
    }
  }
  return true;
}