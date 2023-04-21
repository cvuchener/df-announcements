/*
 * Copyright 2023 Clement Vuchener
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "ColorDelegate.h"

#include <QColorDialog>

ColorDelegate::ColorDelegate(QObject *parent):
	QStyledItemDelegate(parent)
{
}

ColorDelegate::~ColorDelegate()
{
}

QWidget *ColorDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	auto dialog = new QColorDialog(parent);
	dialog->setWindowModality(Qt::WindowModal);
	return dialog;
}

void ColorDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
	auto dialog = qobject_cast<QColorDialog *>(editor);
	if (dialog)
		dialog->setCurrentColor(index.data(Qt::EditRole).value<QColor>());
}

void ColorDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
	auto dialog = qobject_cast<QColorDialog *>(editor);
	if (dialog && dialog->result() == QDialog::Accepted)
		model->setData(index, dialog->currentColor());
}

