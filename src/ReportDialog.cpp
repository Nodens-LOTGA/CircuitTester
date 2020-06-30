#include "ReportDialog.h"
#include "./ui_reportdialog.h"
#include "Settings.h"
#include "winapiprint.h"
#include <QDesktopWidget>
#include <QHeaderView>
#include <QMessageBox>
#include <QPainter>
#include <QPixmap>
#include <QPrinter>
#include <QPrinterInfo>
#include <QScreen>
#include <QThread>
#include <QTimer>

ReportDialog::ReportDialog(rep::Report &report, QWidget *parent)
    : QDialog(parent), ui(new Ui::ReportDialog) {
  ui->setupUi(this);

  connect(ui->closePB, SIGNAL(clicked()), this, SLOT(reject()));
  connect(ui->printLabelPB, SIGNAL(clicked()), this, SLOT(printLabel()));
  connect(ui->printReportPB, SIGNAL(clicked()), this, SLOT(printReport()));

  QDesktopWidget desk;
  QRect screenres = QGuiApplication::screens().first()->availableGeometry();
  setGeometry(QRect(screenres.width() * 1.0 / 6.0, 0,
                    screenres.width() * 2.0 / 3.0, screenres.height() - 40));

  this->report = report;
  ui->printLabelPB->setDisabled(report.hasError());
  auto table = report.createTableWidget(
      this, QSize(screenres.width() * 2.0 / 3.0 - 28, screenres.height() - 60));
  table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  table->setParent(ui->tableF);
  table->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
  table->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
  ui->gridLayout_2->addWidget(table);
  setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
  setWindowTitle(RU("Отчёт. Результат: ") +
                 (report.hasError() ? rep::statusToQStr(rep::Status::Error)
                                    : rep::statusToQStr(rep::Status::Ok)));
}

ReportDialog::~ReportDialog() { delete ui; }

void ReportDialog::printLabel() {
  ui->printLabelPB->setDisabled(true);
  QTimer::singleShot(3000, ui->printLabelPB,
                     [this]() { ui->printLabelPB->setEnabled(true); });
  Settings sett;
  QByteArray buf;
  auto label = sett.labels[sett.label].toString();
  if (!report.createZplLabel(label, buf)) {
    QMessageBox::warning(this, RU("Ошибка при создание этикетки"),
                         RU("Неудалось открыть файл с этикеткой: ") + label);
    return;
  }

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

  ui->printReportPB->setDisabled(true);
  QTimer::singleShot(3000, ui->printReportPB,
                     [this]() { ui->printReportPB->setEnabled(true); });

  QPrinter printer(printerInfo);
  printer.setResolution(300);
  printer.setPageSize(QPageSize(QPageSize::A4));
  printer.setPageMargins(QMarginsF(2.0, 2.0, 4.0, 2.0),
                         QPageLayout::Millimeter);
  printer.setFullPage(false);
  QRect pageRect = printer.pageRect();
  QSize tableSize = QPageSize::sizePixels(QPageSize::A4, 100);
  auto table = report.createTableWidget(nullptr, tableSize, false);
  table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  QFont font(table->font());
  font.setPointSize(14);
  table->setFont(font);
  table->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  table->setAttribute(Qt::WA_DontShowOnScreen);

  double xscale = pageRect.width() / double(tableSize.width() + 40);
  double yscale = pageRect.height() / double(tableSize.height() + 40);
  double scale = qMin(xscale, yscale);

  int height = table->verticalHeader()->length() * scale;
  int page = 1;
  int pageHeight = tableSize.height() * scale;
  int j{};

  if (height > pageHeight) {
    table->setRowCount(table->rowCount() + 1);
    QTableWidgetItem *item = new QTableWidgetItem();
    item->setSizeHint(
        QSize(0, table->verticalHeader()->length() + 2 * tableSize.height()));
    table->setItem(table->rowCount() - 1, 0, item);
  }

  QPainter printPainter(&printer);
  printPainter.translate(printer.paperRect().center());
  printPainter.scale(scale, scale);
  printPainter.translate(-tableSize.width() / 2, -tableSize.height() / 2);
  printPainter.setRenderHints(QPainter::Antialiasing |
                              QPainter::TextAntialiasing |
                              QPainter::SmoothPixmapTransform);
  QRect drawRect(table->rect());
  do {
    table->scrollTo(table->model()->index(j, 0),
                    QAbstractItemView::PositionAtTop);
    int drawHeight{1};
    for (int i = j; i < table->rowCount(); i++) {
      int rowHeight = table->verticalHeader()->sectionSize(i);
      drawHeight += rowHeight;
      if (drawHeight * scale > pageHeight) {
        drawHeight -= rowHeight;
        j = i;
        break;
      }
    }

    /*QImage image(size, QImage::Format_ARGB32);
    image.fill(Qt::transparent);

    QPainter painter(&image);
    // painter.setRenderHint(QPainter::Antialiasing);
    drawRect.setHeight(drawHeight);
    table->render(&painter, QPoint(), drawRect);
    image.save(QString("page%1.png").arg(page));*/

    drawRect.setHeight(drawHeight);
    table->render(&printPainter, QPoint(), drawRect);

    height -= pageHeight;
    if (height > 0) {
      page++;
      printer.newPage();
    }
  } while (height > 0);

  table->close();
}
