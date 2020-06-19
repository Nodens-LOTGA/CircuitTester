#include "CircuitsDelegate.h"
#include <QComboBox>
#include <QLineEdit>
#include <QPainter>
#include <QSpinBox>
QWidget *CircuitsDelegate::createEditor(QWidget *parent,
                                        const QStyleOptionViewItem &option,
                                        const QModelIndex &index) const {

  int column = index.column();
  if (column == 0) {
    QLineEdit *editor = new QLineEdit(parent);
    editor->setFrame(false);
    editor->setReadOnly(true);
    return editor;
  } else if (column == 2 || column == 5) {
    QLineEdit *editor = new QLineEdit(parent);
    editor->setFrame(false);
    return editor;
  } else {
    QSpinBox *editor = new QSpinBox(parent);
    editor->setFrame(false);
    editor->setMinimum(1);
    editor->setMaximum(256);
    editor->setAlignment(Qt::AlignRight);
    return editor;
  }
}

void CircuitsDelegate::setEditorData(QWidget *editor,
                                     const QModelIndex &index) const {
  int column = index.column();
  if (column == 0) {
    auto value = index.model()->data(index, Qt::DisplayRole).toString();

    QLineEdit *lineEdit = static_cast<QLineEdit *>(editor);
    lineEdit->setText(value);
  } else if (column == 2 || column == 5) {
    auto value = index.model()->data(index, Qt::EditRole).toString();

    QLineEdit *lineEdit = static_cast<QLineEdit *>(editor);
    lineEdit->setText(value);
  } else {
    auto value = index.model()->data(index, Qt::EditRole).toInt();

    QSpinBox *spinBox = static_cast<QSpinBox *>(editor);
    spinBox->setValue(value);
  }
}

void CircuitsDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                    const QModelIndex &index) const {

  int column = index.column();
  if (column == 0) {
    QLineEdit *lineEdit = static_cast<QLineEdit *>(editor);
    auto value = lineEdit->text();

    model->setData(index, value, Qt::DisplayRole);
  } else if (column == 2 || column == 5) {
    QLineEdit *lineEdit = static_cast<QLineEdit *>(editor);
    auto value = lineEdit->text();

    model->setData(index, value, Qt::EditRole);
  } else {
    QSpinBox *spinBox = static_cast<QSpinBox *>(editor);
    spinBox->interpretText();
    auto value = spinBox->value();
    
    model->setData(index, value, Qt::EditRole);
  }
}

void CircuitsDelegate::updateEditorGeometry(QWidget *editor,
                                            const QStyleOptionViewItem &option,
                                            const QModelIndex &index) const {
  editor->setGeometry(option.rect);
}

void CircuitsDelegate::paint(QPainter *painter,
                             const QStyleOptionViewItem &option,
                             const QModelIndex &index) const {
  QStyledItemDelegate::paint(painter, option, index);
  int column = index.column();
  if (column == 4) {
    painter->save();

    //painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(Qt::black);
    painter->setBrush(Qt::black);

    double cX = option.rect.bottomRight().x() - 2;
    double cY = option.rect.center().y() + 2;
    double size = 6;
    QPointF triangle[3] = {QPointF(cX - size , cY + size / 2), QPointF(cX - size, cY - size / 2),
                           QPointF(cX, cY)};
    painter->drawPolygon(triangle, 3);
    
    painter->restore();
  }
}
