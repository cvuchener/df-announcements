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

#ifndef REPORT_FILTER_PROXY_MODEL_H
#define REPORT_FILTER_PROXY_MODEL_H

#include <QSortFilterProxyModel>

class AnnouncementTypeList;

class ReportFilterProxyModel: public QSortFilterProxyModel
{
	Q_OBJECT
public:
	ReportFilterProxyModel(const AnnouncementTypeList &type_list, QObject *parent = nullptr);
	~ReportFilterProxyModel() override;

protected:
	bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
	const AnnouncementTypeList &_type_list;
};

#endif
