#pragma once
#include <QDebug>
#include <QHelpEngine>
#include <QTextBrowser>

class HelpBrowser : public QTextBrowser {
public:
  HelpBrowser(QHelpEngine *helpEngine, QWidget *parent = 0);
  QVariant loadResource(int type, const QUrl &name);

private:
  QHelpEngine *helpEngine;
};
