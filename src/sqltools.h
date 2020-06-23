#pragma once
#include <QMessageBox>
#include <QtSql>

namespace Sql {

void addCircuit(QSqlQuery &q, int pin, int circuit, const QString &name);
void addRelation(QSqlQuery &q, int num, int pinFrom, int pinTo);

QString circuitsSql(int id);
QString insertCircuitsSql(int id);
QString relationsSql(int id);
QString insertRealationsSql(int id);
QString productsSql();
QString insertProductsSql();

QSqlError initDb();
void closeDb();
void addProd(QSqlQuery &q, const QString &prodName);

void showSqlError(const QSqlError &err, QWidget *parent = 0);

}