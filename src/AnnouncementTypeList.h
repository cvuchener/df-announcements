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

#ifndef ANNOUNCEMENT_TYPE_LIST_H
#define ANNOUNCEMENT_TYPE_LIST_H

#include <QAbstractListModel>

class AnnouncementTypeList: public QAbstractListModel
{
	Q_OBJECT
public:
	AnnouncementTypeList(QObject *parent = nullptr);
	~AnnouncementTypeList() override;

	Qt::ItemFlags flags(const QModelIndex &) const override;
	int rowCount(const QModelIndex &) const override;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

	bool isTypeEnabled(const QByteArray &type) const;

public slots:
	void addType(const QByteArray &type, bool enabled = true);

signals:
	void typesChanged();

private:
	std::vector<std::pair<QByteArray, bool>> _types;
};

#endif
