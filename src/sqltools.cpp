#include <sqltools.h>
#include <tools.h>

void Sql::addCircuit(QSqlQuery &q, int pin, int circuit, const QString &name) {
  q.addBindValue(pin);
  q.addBindValue(circuit);
  q.addBindValue(name);
  q.exec();
}

void Sql::addRelation(QSqlQuery &q, int num, int pinFrom, int pinTo) {
  q.addBindValue(num);
  q.addBindValue(pinFrom);
  q.addBindValue(pinTo);
  q.exec();
}

QString Sql::circuitsSql(int id) {
  return QString("create table circuits%1(pin integer PRIMARY KEY NOT NULL, circuit integer NOT NULL, name "
                 "varchar NOT NULL)")
      .arg(id);
}

QString Sql::relationsSql(int id) {
  return QString(
             "create table relations%1(id INTEGER PRIMARY KEY NOT NULL, num integer NOT NULL, "
             "pinFrom integer NOT NULL, pinTo integer NOT NULL, FOREIGN KEY (pinFrom) REFERENCES "
             "circuits%1(pin) ON DELETE CASCADE ON UPDATE CASCADE, FOREIGN KEY (pinTo) REFERENCES circuits%1(pin) ON DELETE CASCADE ON UPDATE CASCADE)")
      .arg(id);
}

QString Sql::insertCircuitsSql(int id) {
  return QString("insert into circuits%1(pin, circuit, name) values(?, ?, ?)").arg(id).toLatin1();
}

QString Sql::insertRealationsSql(int id) {
  return QString("insert into relations%1(num, pinFrom, pinTo) values(?, ?, ?)")
      .arg(id)
      .toLatin1();
}

QString Sql::productsSql() {
  return QString("create table products(id integer primary key, name varchar)");
}

QString Sql::insertProductsSql() {
  return QString("insert into products(name) values(?)");
}

QSqlError Sql::initDb() {
  QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
  db.setDatabaseName(qApp->applicationDirPath() + "/circuits.db");

  if (!db.open())
    return db.lastError();

  QStringList tables = db.tables();
  if (tables.contains("products", Qt::CaseInsensitive))
    return QSqlError();

  QSqlQuery q;
  if (!q.exec(productsSql()))
    return q.lastError();

  if (!q.prepare(insertProductsSql()))
    return q.lastError();

  if (!q.exec("PRAGMA foreign_keys = ON;"))
    return q.lastError();

  return QSqlError();
}

void Sql::closeDb() {
  QSqlDatabase::database().close();
  QSqlDatabase::removeDatabase(QSqlDatabase::database().connectionName());
}

void Sql::addProd(QSqlQuery &q, const QString &prodName) {
  q.addBindValue(prodName);
  q.exec();
}

void Sql::showSqlError(const QSqlError &err, QWidget *parent) {
  QMessageBox::critical(parent, RU("Ошибка БД"),
                        RU("Ошибка при работе с БД: ") + err.text());
}
