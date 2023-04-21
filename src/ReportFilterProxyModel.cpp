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

#include "ReportFilterProxyModel.h"

#include "AnnouncementTypeList.h"
#include "ReportModel.h"

ReportFilterProxyModel::ReportFilterProxyModel(const AnnouncementTypeList &type_list, QObject *parent):
	QSortFilterProxyModel(parent),
	_type_list(type_list)
{
	connect(&type_list, &AnnouncementTypeList::typesChanged, [this]() {
			invalidateRowsFilter();
		});
}

ReportFilterProxyModel::~ReportFilterProxyModel()
{
}

bool ReportFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
	auto index = sourceModel()->index(source_row, static_cast<int>(ReportModel::Columns::Type), source_parent);
	return index.isValid()
		&& _type_list.isTypeEnabled(index.data(Qt::DisplayRole).toByteArray())
		&& QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
}
