#include <sqltools.h>
#include <tools.h>

void Sql::addItem(QSqlQuery &q, int num, const QString &nameFrom,
                  int circuitFrom, int pinFrom, const QString &nameTo,
                  int circuitTo, int pinTo) {
  q.addBindValue(num);
  q.addBindValue(nameFrom);
  q.addBindValue(circuitFrom);
  q.addBindValue(pinFrom);
  q.addBindValue(nameTo);
  q.addBindValue(circuitTo);
  q.addBindValue(pinTo);
  q.exec();
}

QString Sql::circuitsSql(int id) {
  return QString("create table circuits%1(id integer primary key, num integer, "
              "nameFrom varchar, circuitFrom integer, pinFrom integer, nameTo "
              "varchar, circuitTo integer, pinTo integer)")
          .arg(id);
}

QString Sql::insertCircuitsSql(int id) {
  return QString("insert into circuits%1(num, nameFrom, circuitFrom, pinFrom, nameTo, circuitTo, pinTo) values(?, ?, ?, ?, ?, ?, ?)").arg(id).toLatin1();
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
