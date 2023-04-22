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

#include "AnnouncementTypeList.h"

#include <QSettings>

static const char * const AnnouncementTypeGroupName = "announcement_types";

AnnouncementTypeList::AnnouncementTypeList(QObject *parent):
	QAbstractListModel(parent)
{
	QSettings settings;
	settings.beginGroup(AnnouncementTypeGroupName);
	for (const auto &key: settings.childKeys()) {
		addType(key.toLocal8Bit(), settings.value(key).toBool());
	}
	settings.endGroup();
}

AnnouncementTypeList::~AnnouncementTypeList()
{
	QSettings settings;
	settings.beginGroup(AnnouncementTypeGroupName);
	for (const auto &[name, enabled]: _types) {
		settings.setValue(name, enabled);
	}
	settings.endGroup();
}

Qt::ItemFlags AnnouncementTypeList::flags(const QModelIndex &) const
{
	return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
}

int AnnouncementTypeList::rowCount(const QModelIndex &parent) const
{
	if (parent.isValid())
		return 0;
	else
		return _types.size();
}

QVariant AnnouncementTypeList::data(const QModelIndex &index, int role) const
{
	const auto &type = _types[index.row()];
	switch (role) {
	case Qt::DisplayRole:
		return type.first;
	case Qt::CheckStateRole:
		return type.second ? Qt::Checked : Qt::Unchecked;
	default:
		return {};
	}
}

bool AnnouncementTypeList::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (role != Qt::CheckStateRole)
		return false;
	_types[index.row()].second = value.toBool();
	dataChanged(index, index, {Qt::CheckStateRole});
	typesChanged();
	return true;
}

bool AnnouncementTypeList::isTypeEnabled(const QByteArray &type) const
{
	auto it = std::ranges::lower_bound(_types, type, std::less<>{}, &decltype(_types)::value_type::first);
	if (it == _types.end() || it->first != type)
		return true;
	return it->second;
}

void AnnouncementTypeList::addType(const QByteArray &type, bool enabled)
{
	auto it = std::ranges::lower_bound(_types, type, std::less<>{}, &decltype(_types)::value_type::first);
	if (it != _types.end() && it->first == type)
		return;
	int row = std::distance(_types.begin(), it);
	beginInsertRows({}, row, row);
	_types.insert(it, {type, enabled});
	endInsertRows();
}
