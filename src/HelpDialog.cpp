#include "helpdialog.h"
#include "./ui_helpdialog.h"
#include "tools.h"

HelpDialog::HelpDialog(const QString &collection, QWidget *parent)
    : QDialog(parent), ui(new Ui::HelpDialog) {
  ui->setupUi(this);

  connect(ui->okPB, SIGNAL(clicked()), this, SLOT(accept()));

  helpEngine = new QHelpEngine(collection);
  helpEngine->setupData(); 

  HelpBrowser *textBrowser = new HelpBrowser(helpEngine, this);
  QSizePolicy sizePolicy1(QSizePolicy::Expanding,
                          QSizePolicy::MinimumExpanding);
  sizePolicy1.setHorizontalStretch(0);
  sizePolicy1.setVerticalStretch(0);
  sizePolicy1.setHeightForWidth(textBrowser->sizePolicy().hasHeightForWidth());
  textBrowser->setSizePolicy(sizePolicy1);

  QTabWidget *tabWidget = new QTabWidget(this);
  tabWidget->addTab(helpEngine->contentWidget(), RU("Содержание"));
  //tabWidget->addTab(helpEngine->indexWidget(), RU("Указатель"));

  QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
  tabWidget->setMaximumWidth(400); 
  splitter->insertWidget(0, tabWidget);
  splitter->insertWidget(1, textBrowser);
  ui->horizontalLayout_2->addWidget(splitter);

  textBrowser->setSource(
      QUrl("qthelp://ladaplast.qt.circuittester/doc/index.html"));

  connect(helpEngine->contentWidget(), SIGNAL(linkActivated(QUrl)), textBrowser,
          SLOT(setSource(QUrl)));
  /*connect(helpEngine->indexWidget(), SIGNAL(linkActivated(QUrl, QString)),
          textBrowser, SLOT(setSource(QUrl)));*/
}

HelpDialog::~HelpDialog() { delete ui; }