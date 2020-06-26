#include "ReportDelegate.h"
#include <QPainter>
#include <QPalette>
#include <QPixmap>
#include <QVariant>
#include <Report.h>

void ReportDelegate::paint(QPainter *painter,
                           const QStyleOptionViewItem &option,
                           const QModelIndex &index) const {
  if (index.row() > 1) {
    painter->save();
    painter->setPen(Qt::black);
    painter->drawRect(option.rect);
    painter->restore();
  }
  if (m_colorful) {
    if (index.data().toString() == rep::statusToQStr(rep::Status::Ok)) {
      painter->save();
      /*QPen pen(painter->pen());
      pen.setColor(QColor("#228B22"));
      painter->setPen(pen);
      QRect rect = option.rect;
      rect.moveLeft(rect.x() + 3);
      painter->drawText(rect, Qt::AlignVCenter, index.data().toString());*/
      painter->setBrush(QColor("#228B22"));
      painter->drawRect(option.rect);
      painter->restore();
    } else if (index.data().toString() ==
               rep::statusToQStr(rep::Status::Error)) {
      painter->save();
      /*QPen pen(painter->pen());
      pen.setColor(QColor("#FF2400"));
      painter->setPen(pen);
      QRect rect = option.rect;
      rect.moveLeft(rect.x() + 3);
      painter->drawText(rect, Qt::AlignVCenter, index.data().toString());*/
      painter->setBrush(QColor("#FF2400"));
      painter->drawRect(option.rect);
      painter->restore();
    }
  }
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
