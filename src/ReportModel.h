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

#ifndef REPORT_MODEL_H
#define REPORT_MODEL_H

#include <QAbstractTableModel>

#include <vector>

#include "reports.pb.h"
#include "DFTime.h"

class AnnouncementTypeList;

class ReportModel: public QAbstractTableModel
{
	Q_OBJECT
public:
	ReportModel(QObject *parent = nullptr);
	~ReportModel() override = default;

	enum class Columns {
		Id = 0,
		Date,
		Type,
		Text,
		Count
	};

	enum ItemDataRoles {
		SortRole = Qt::UserRole,
	};

	int rowCount(const QModelIndex &parent) const override;
	int columnCount(const QModelIndex &parent) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

public slots:
	void update(const dfproto::Reports::ReportList &report_list);
	void clear();

private:
	AnnouncementTypeList &_type_list;
	struct report {
		int id;
		DF::time time;
		QString text;
		QByteArray type;
		int color;
		int repeat;

		void init(const dfproto::Reports::Report &report);
		void update(const dfproto::Reports::Report &report);
	};
	std::vector<report> _reports;
};

#endif
