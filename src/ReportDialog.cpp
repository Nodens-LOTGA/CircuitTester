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

ReportDialog::ReportDialog(rep::Report &report, QWidget *parent)
    : QDialog(parent), ui(new Ui::ReportDialog) {
  ui->setupUi(this);

  connect(ui->closePB, SIGNAL(clicked()), this, SLOT(reject()));
  connect(ui->printLabelPB, SIGNAL(clicked()), this, SLOT(printLabel()));
  connect(ui->printReportPB, SIGNAL(clicked()), this, SLOT(printReport()));

  QDesktopWidget desk;
  QRect screenres = QGuiApplication::screens().first()->availableGeometry();
  setGeometry(QRect(screenres.width() * 1.0 / 6.0, 0, screenres.width() * 2.0 / 3.0,
                    screenres.height() - 40));

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

  QPrinter printer(printerInfo);
  printer.setPageSize(QPageSize(QPageSize::A4));
  QRect pageRect = printer.pageLayout().paintRectPixels(printer.resolution());
  QSize size = pageRect.size();
  auto table = report.createTableWidget(nullptr, size, false);

  // table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  QFont font(table->font());
  font.setPointSize(14);
  table->setFont(font);
  table->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  table->setAttribute(Qt::WA_DontShowOnScreen);

  int height = table->verticalHeader()->length();
  int page = 1;
  int pageHeight = pageRect.height();
  int j{};
  QRect drawRect{pageRect};
  table->setRowCount(table->rowCount() + 1);
  QTableWidgetItem *item = new QTableWidgetItem();
  item->setSizeHint(QSize(0, height + 2 * pageHeight));
  table->setItem(table->rowCount() - 1, 0, item);

  QPainter printPainter(&printer);
  do {
    table->scrollTo(table->model()->index(j, 0),
                    QAbstractItemView::PositionAtTop);
    int drawHeight{1};
    for (int i = j; i < table->rowCount(); i++) {
      int rowHeight = table->verticalHeader()->sectionSize(i);
      drawHeight += rowHeight;
      if (drawHeight > pageHeight) {
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

    table->render(&printPainter, QPoint(), drawRect);

    height -= pageHeight;
    if (height > 0) {
      page++;
      printer.newPage();
    }
  } while (height > 0);

  table->close();
}
