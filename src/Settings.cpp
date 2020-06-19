#include "Settings.h"
#include <QVariant>
void Settings::save() {
  sett.sync();
  sett.setValue("main/isAdmin", isAdmin);
  sett.setValue("users/users", users);
  sett.setValue("main/newUsers", newUsers);
  sett.setValue("main/num", num);
  sett.setValue("main/prodName", prodName);
  sett.setValue("main/reportDir", reportDir);
  sett.setValue("main/portName", portName);
  sett.setValue("main/userName", userName);
  sett.setValue("print/labelPrinterName", labelPrinterName);
  sett.setValue("print/reportPrinterName", reportPrinterName);
  sett.sync();
}

void Settings::load() {
  sett.sync();
  loadValue("main/isAdmin", isAdmin);
  loadValue("users/users", users);
  loadValue("main/newUsers", newUsers);
  loadValue("main/num", num);
  loadValue("main/prodName", prodName);
  loadValue("main/reportDir", reportDir);
  loadValue("main/portName", portName);
  loadValue("main/userName", userName);
  loadValue("print/labelPrinterName", labelPrinterName);
  loadValue("print/reportPrinterName", reportPrinterName);
  sett.sync();
}

void Settings::discardSettings() {
  sett.sync();
  sett.remove("main");
  sett.remove("print");
  Settings();
}

void Settings::discardUsers() {
  sett.sync();
  sett.remove("users");
  sett.sync();
}
