#include "ReportDialog.h"
#include "./ui_reportdialog.h"
#include "Settings.h"
#include "winapiprint.h"
#include <QHeaderView>
#include <QMessageBox>
#include <QPainter>
#include <QPixmap>
#include <QPrinter>
#include <QPrinterInfo>

ReportDialog::ReportDialog(rep::Report &report, QWidget *parent)
    : QDialog(parent), ui(new Ui::ReportDialog) {
  ui->setupUi(this);

  connect(ui->closePB, SIGNAL(clicked()), this, SLOT(reject()));
  connect(ui->printLabelPB, SIGNAL(clicked()), this, SLOT(printLabel()));
  connect(ui->printReportPB, SIGNAL(clicked()), this, SLOT(printReport()));

  this->report = report;
  ui->printLabelPB->setDisabled(report.hasError());
  auto table = report.createTableWidget(this, QSize(800, 800));
  table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  table->setParent(ui->tableF);
  ui->gridLayout_2->addWidget(table);
  setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
}

ReportDialog::~ReportDialog() { delete ui; }

void ReportDialog::printLabel() {
  QByteArray buf;
  if (!report.createZplLabel("label.prn", buf)) {
    QMessageBox::warning(this, RU("Ошибка при создание этикетки"),
                         RU("Неудалось открыть файл с этикеткой (label.zpl)"));
    return;
  }

  Settings sett;
  QString printerName = sett.labelPrinterName;
  QPrinterInfo printerInfo = QPrinterInfo::printerInfo(printerName);
  if (printerInfo.isNull()) {
    QMessageBox::warning(this, RU("Ошибка при печати этикетки"),
                         RU("Выбранный принтер не найден"));
    return;
  }

  if (!RawDataToPrinter(printerName.toLocal8Bit().data(), buf.data(),
                        buf.size())) {
    QMessageBox::warning(this, RU("Ошибка при печати этикетки"),
                         RU("Неудалось напечатать этикетку"));
    return;
  }
}

void ReportDialog::printReport() {
  Settings sett;
  QString printerName = sett.reportPrinterName;
  QPrinterInfo printerInfo = QPrinterInfo::printerInfo(printerName);
  if (printerInfo.isNull()) {
    QMessageBox::warning(this, RU("Ошибка при печати отчёта"),
                         RU("Выбранный принтер не найден"));
    return;
  }

  QPrinter printer(printerInfo);
  printer.setPageSize(QPageSize(QPageSize::A4));
  QRect pageSize = printer.pageLayout().paintRectPixels(printer.resolution());
  QSize size = pageSize.size();
  auto table = report.createTableWidget(nullptr, size);
  //table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  QFont font(table->font());
  font.setPointSize(20);
  table->setFont(font);
  auto s = table->verticalHeader()->length();
  for (int i = 20; s > pageSize.height(); i--) {
    font.setPointSize(i);
    table->setFont(font);
    s = table->verticalHeader()->length();
  }

  QImage image1(pageSize.size(), QImage::Format_ARGB32);

  image1.fill(Qt::transparent);

  QPainter painter(&image1);
  // painter.setRenderHint(QPainter::Antialiasing);
  table->render(&painter);
  image1.save("test_img.png");
  delete table;
}
