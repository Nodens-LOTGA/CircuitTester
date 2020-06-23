#include "CircuitsDelegate.h"
#include <QComboBox>
#include <QLineEdit>
#include <QPainter>
#include <QSpinBox>
#include "sqltools.h"
QWidget *CircuitsDelegate::createEditor(QWidget *parent,
                                        const QStyleOptionViewItem &option,
                                        const QModelIndex &index) const {

  int column = index.column();
  if (column == 0) {
    QSpinBox *editor = new QSpinBox(parent);
    editor->setFrame(false);
    editor->setMinimum(0);
    editor->setMaximum(255);
    editor->setAlignment(Qt::AlignRight);
    connect(editor, QOverload<int>::of(&QSpinBox::valueChanged), [=](int i) {
      while (pins.contains(i % 255))
        i++;
      editor->setValue(i);
    }); 
    return editor;
  } else if (column == 1) {
    QSpinBox *editor = new QSpinBox(parent);
    editor->setFrame(false);
    editor->setMinimum(0);
    editor->setMaximum(255);
    editor->setAlignment(Qt::AlignRight);
    connect(editor, QOverload<int>::of(&QSpinBox::valueChanged), [=](int i) {
      while (circuits.contains(i % 255))
        i++;
      editor->setValue(i);
    });
    return editor;
  }
  return QStyledItemDelegate::createEditor(parent, option, index);
}

void CircuitsDelegate::setEditorData(QWidget *editor,
                                     const QModelIndex &index) const {
  int column = index.column();
  if (column == 0 || column == 1) {
    auto value = index.model()->data(index, Qt::EditRole).toInt();

    QSpinBox *spinBox = static_cast<QSpinBox *>(editor);
    spinBox->setValue(value);
  }
  return QStyledItemDelegate::setEditorData(editor, index);
}

void CircuitsDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                    const QModelIndex &index) const {

  int column = index.column();
  if (column == 0) {
    QSpinBox *spinBox = static_cast<QSpinBox *>(editor);
    spinBox->interpretText();
    auto value = spinBox->value();
    auto oldValue = index.model()->data(index, Qt::EditRole).toInt();
    if (oldValue != value) {
      pins[pins.indexOf(oldValue)] = value;
    }
    model->setData(index, value, Qt::EditRole);
  } else if (column == 1) {
    QSpinBox *spinBox = static_cast<QSpinBox *>(editor);
    spinBox->interpretText();
    auto value = spinBox->value();
    auto oldValue = index.model()->data(index, Qt::EditRole).toInt();
    if (oldValue != value) {
      circuits[circuits.indexOf(oldValue)] = value;
    }
    model->setData(index, value, Qt::EditRole);
  }
  return QStyledItemDelegate::setModelData(editor, model, index);
}

void CircuitsDelegate::updateEditorGeometry(QWidget *editor,
                                            const QStyleOptionViewItem &option,
                                            const QModelIndex &index) const {
  editor->setGeometry(option.rect);
}
