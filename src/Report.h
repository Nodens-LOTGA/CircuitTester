#pragma once
#include <QTableWidget>
#include <QVector>
#include <QDate>
#include <QTime>
#include <QMap>
struct Item {
  enum class Status { Ok, Error, Short = 2, Open = 4};
 
  static QString statusToQStr(const Item::Status stat);

  int circuitNum;
  char pin;
  int circuit;
  QString name;
  QVector<Item> relations;
  Status stat = Status::Ok;
};

class Report {
public:
  Report() = default;
  Report(QMap<int, QVector<Item>> _items, int _curNum, QString _prodName, QString _name)
      : items(_items), curNum(_curNum), prodName(_prodName), name(_name) {
    date = QDate::currentDate().toString("dd.MM.yyyy");
    time = QTime::currentTime().toString("HH:mm:ss");
  };

  QTableWidget *createTableWidget(QWidget *parent, QSize size);
  QTableWidget *createLabelTableWidget(QWidget *parent, QSize size);
  bool saveTo(const QString &file);
  bool createZplLabel(const QString &file, QByteArray &buf);
  bool hasError() const { return error; }

  private:
  void addRow(QTableWidget *table, int row, QString str1, QString str2);

private:
  QMap<int, QVector<Item>> items{};
  int curNum{};
  QString prodName{}, name{}, date{}, time{};
  bool error = false;
};

//Q_DECLARE_OPAQUE_POINTER(Item *)

//Q_DECLARE_METATYPE(Item)