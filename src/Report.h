#pragma once
#include "SerialPort.h"
#include <QDate>
#include <QMap>
#include <QTableWidget>
#include <QTime>
#include <QVector>
#include <boost/graph/adjacency_list.hpp>

namespace rep {

enum class Status { Ok, Error, Short = Error | 2, Open = Error | 4};

static QString statusToQStr(const Status stat) {
  switch (stat) {
  case Status::Ok:
    return QString("OK");
  case Status::Open:
    return QString("Open");
  case Status::Short:
    return QString("Short");
  case Status::Error:
    return QString("Error");
  }
};

struct Vertex {
  QString name;
  int circuit;
  char pin;
};

struct Edge {
  Status status = Status::Open;
};

using Graph = boost::adjacency_list<boost::setS, boost::vecS,
                                    boost::undirectedS, Vertex, Edge>;

using vertex_t = boost::graph_traits<Graph>::vertex_descriptor;
using edge_t = boost::graph_traits<Graph>::edge_descriptor;

class Report {
public:
  Report() = default;
  Report(int _curNum, QString _prodName, QString _name)
      : curNum(_curNum), prodName(_prodName), name(_name) {
    date = QDate::currentDate().toString("dd.MM.yyyy");
    time = QTime::currentTime().toString("HH:mm:ss");
  };

  QTableWidget *createTableWidget(QWidget *parent, QSize size);
  QTableWidget *createLabelTableWidget(QWidget *parent, QSize size);
  bool saveTo(const QString &file);
  bool createZplLabel(const QString &file, QByteArray &buf);
  bool hasError() const { return error; }
  bool fill();
  bool checkAll(SerialPort &port);

private:
  void addRow(QTableWidget *table, int row, QString str1, QString str2);

private:
  QMap<int, QVector<QPair<vertex_t, vertex_t>>> circuits;
  QMap<char, vertex_t> pins;
  Graph graph;
  int curNum{};
  QString prodName{}, name{}, date{}, time{};
  bool error = false;
};

} // namespace rep

// Q_DECLARE_OPAQUE_POINTER(Item *)

// Q_DECLARE_METATYPE(Item)