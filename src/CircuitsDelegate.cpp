#include "CircuitsDelegate.h"
#include "sqltools.h"
#include "tools.h"
#include <QComboBox>
#include <QLineEdit>
#include <QPainter>
#include <QSpinBox>
#include <QToolTip>
#include <QTimer>

QWidget *CircuitsDelegate::createEditor(QWidget *parent,
                                        const QStyleOptionViewItem &option,
                                        const QModelIndex &index) const {
  int column = index.column();
  int row = index.row();
  if (column == 0 || column == 1) {
    QSpinBox *editor = new QSpinBox(parent);
    editor->setFrame(false);
    editor->setMinimum(0);
    editor->setMaximum(255);
    editor->setAlignment(Qt::AlignRight);
    return editor;
  } else if (column == 2) {
    QLineEdit *editor = new QLineEdit(parent);
    editor->setFrame(false);
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
  } else if (column == 2) {
    auto value = index.model()->data(index, Qt::EditRole).toString();

    QLineEdit *lineEdit = static_cast<QLineEdit *>(editor);
    lineEdit->setText(value);
  }
  return QStyledItemDelegate::setEditorData(editor, index);
}

void CircuitsDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                    const QModelIndex &index) const {
  int column = index.column();
  int row = index.row();
  if (column == 0) {
    QSpinBox *spinBox = static_cast<QSpinBox *>(editor);
    spinBox->interpretText();
    auto value = spinBox->value();
    auto oldValue = model->data(index, Qt::EditRole).toInt();
    if (oldValue == value)
      return;
    else if (pins.contains(value)) {
      QTimer::singleShot(100, []() {
        QToolTip::showText(QCursor::pos(),
                           RU("Такое значение уже существует"), nullptr,
                           QRect(), 5000);
      });
      return;
    }
    pins[pins.indexOf(oldValue)] = value;
    model->setData(index, value, Qt::EditRole);
  } else if (column == 1) {
    QSpinBox *spinBox = static_cast<QSpinBox *>(editor);
    spinBox->interpretText();
    auto value = spinBox->value();
    auto oldValue = model->data(index, Qt::EditRole).toInt();
    auto name = model->index(row, column + 1).data(Qt::EditRole).toString();
    if (oldValue == value)
      return;
    else if (circuits.contains(name) && circuits[name].contains(value)) {
      QTimer::singleShot(100, []() {
        QToolTip::showText(QCursor::pos(), RU("Такое значение уже существует"),
                           nullptr, QRect(), 5000);
      });
      return;
    }
    circuits[name][circuits[name].indexOf(oldValue)] = value;
    model->setData(index, value, Qt::EditRole);
  } else if (column == 2) {
    QLineEdit *lineEdit = static_cast<QLineEdit *>(editor);
    auto value = lineEdit->text();
    auto oldValue = model->data(index, Qt::EditRole).toString();
    auto circuit = model->index(row, column - 1).data(Qt::EditRole).toInt();
    if (oldValue == value)
      return;
    else if (circuits.contains(value) && circuits[value].contains(circuit)) {
      QTimer::singleShot(100, []() {
        QToolTip::showText(QCursor::pos(), RU("Такое значение уже существует"),
                           nullptr, QRect(), 5000);
      });
      return;
    }
    if (circuits[oldValue].size() == 1) {
      circuits.remove(oldValue);
      circuits[value].push_back(circuit);
    } else {
      circuits[oldValue].removeOne(circuit);
      circuits[value].push_back(circuit);
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
