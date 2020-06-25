#pragma once
#include "tools.h"
#include <QApplication>
#include <QSettings>
#include <QString>
#include <QMap>

struct Settings {
  Settings(bool isDefault = false)
      : newUsers{false}, isAdmin{true}, num{1}, userName(), portName(){
    prodName = "";
    reportDir = qApp->applicationDirPath() + "/report/";
    adminName = RU("Администратор");
    adminPass = "12345678";
    labelPrinterName = "";
    reportPrinterName = "";
    users[adminName] = adminPass;
    userName = adminName;
    labels = QMap<QString, QVariant>{
        {RU("Термоэтикетка 43х25"), QString("label43x25.prn")},
        {RU("Термоэтикетка 57х40"), QString("label57x40.prn")}};
    label = labels.first().toString();
    if (!isDefault)
      load();
  };
  void save();
  void load();
  void discardSettings();
  void discardUsers();

  bool newUsers, isAdmin;
  int num;
  QString prodName, reportDir, userName, portName, labelPrinterName,
      reportPrinterName, adminName, adminPass, label;
  QMap<QString, QVariant> users, labels;

private:
  template <typename T> inline void loadValue(QString valueName, T &value);

  QSettings sett;
};

template <typename T>
inline void Settings::loadValue(QString valueName, T &value) {
  auto v = sett.value(valueName);
  if (v.isNull()) {
    sett.setValue(valueName, value);
  } else
    value = v.value<T>();
}
