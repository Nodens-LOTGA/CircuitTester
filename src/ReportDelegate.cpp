#include "ReportDelegate.h"
#include <QPainter>
#include <QPixmap>
#include <QVariant>
#include <Report.h>
#include <QPalette>

void ReportDelegate::paint(QPainter *painter,
                           const QStyleOptionViewItem &option,
                           const QModelIndex &index) const {
  if (index.row() > 1) {
    QPen oldPen = painter->pen();
    painter->setPen(Qt::black);
    painter->drawRect(option.rect);
    painter->setPen(oldPen);
  } 
  if (index.data().toString() == rep::statusToQStr(rep::Status::Ok)) {
    painter->save();
    QPen pen(painter->pen());
    pen.setColor(QColor("#228B22"));
    painter->setPen(pen);
    QRect rect = option.rect;
    rect.moveLeft(rect.x() + 3);
    painter->drawText(rect, Qt::AlignVCenter, index.data().toString());
    painter->restore();
  } else if (index.data().toString() == rep::statusToQStr(rep::Status::Error)) {
    painter->save();
    QPen pen(painter->pen());
    pen.setColor(QColor("#FF2400"));
    painter->setPen(pen);
    QRect rect = option.rect;
    rect.moveLeft(rect.x() + 3);
    painter->drawText(rect, Qt::AlignVCenter, index.data().toString());
    painter->restore();
  } else
    QStyledItemDelegate::paint(painter, option, index);
}

QSize ReportDelegate::sizeHint(const QStyleOptionViewItem &option,
                               const QModelIndex &index) const {
  if (index.data(Qt::DecorationRole).canConvert<QPixmap>()) {
    QPixmap img = qvariant_cast<QPixmap>(index.data(Qt::DecorationRole));
    return img.size();
  }
  return QStyledItemDelegate::sizeHint(option, index);
}
