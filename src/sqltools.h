#pragma once
#include <QMessageBox>
#include <QtSql>

namespace Sql {

void addItem(QSqlQuery &q, int num, const QString &nameFrom, int circuitFrom,
                int pinFrom, const QString &nameTo, int circuitTo, int pinTo);

QString circuitsSql(int id);
QString insertCircuitsSql(int id);
QString productsSql();
QString insertProductsSql();

QSqlError initDb();
void closeDb();
void addProd(QSqlQuery &q, const QString &prodName);

void showSqlError(const QSqlError &err, QWidget *parent = 0);

}