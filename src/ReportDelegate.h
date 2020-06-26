#pragma once
#include <QStyledItemDelegate>

class ReportDelegate : public QStyledItemDelegate {
  Q_OBJECT
public:
  using QStyledItemDelegate::QStyledItemDelegate;

  ReportDelegate(QWidget *parent = nullptr, bool colorful = true)
      : QStyledItemDelegate(parent), m_colorful(colorful){};

  void paint(QPainter *painter, const QStyleOptionViewItem &option,
             const QModelIndex &index) const override;
  QSize sizeHint(const QStyleOptionViewItem &option,
                 const QModelIndex &index) const override;

private:
  bool m_colorful;
};