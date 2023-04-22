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

#include "ReportModel.h"

#include <array>
#include <QColor>

#include "AnnouncementTypeList.h"
#include "Application.h"

ReportModel::ReportModel(QObject *parent):
	QAbstractTableModel(parent),
	_type_list(Application::instance()->settings()->announcement_types)
{
	auto settings = Application::instance()->settings();
	connect(&settings->color_palette, &QAbstractItemModel::dataChanged, [this]() {
			int col = static_cast<int>(Columns::Text);
			dataChanged(index(0, col), index(_reports.size()-1, col), {Qt::ForegroundRole});
		});
}

int ReportModel::rowCount(const QModelIndex &parent) const
{
	if (parent.isValid())
		return 0;
	else
		return _reports.size();
}

int ReportModel::columnCount(const QModelIndex &) const
{
	return static_cast<int>(Columns::Count);
}

QVariant ReportModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation != Qt::Horizontal)
		return {};
	if (role != Qt::DisplayRole)
		return {};
	switch (static_cast<Columns>(section)) {
	case Columns::Id:
		return tr("Id");
	case Columns::Date:
		return tr("Date");
	case Columns::Text:
		return tr("Text");
	case Columns::Type:
		return tr("Type");
	default:
		return {};
	}
}

QVariant ReportModel::data(const QModelIndex &index, int role) const
{
	auto settings = Application::instance()->settings();
	const auto &report = _reports[index.row()];
	switch (static_cast<Columns>(index.column())) {
	case Columns::Id:
		switch (role) {
		case Qt::DisplayRole:
			return report.id;
		default:
			return {};
		}
	case Columns::Date:
		switch (role) {
		case Qt::DisplayRole:
			return DF::prettyDate(report.time);
		case SortRole:
			return static_cast<qlonglong>(report.time.count());
		default:
			return {};
		}
	case Columns::Text:
		switch (role) {
		case Qt::DisplayRole:
			if (report.repeat > 0)
				return QString("%1 (×%2)").arg(report.text).arg(report.repeat+1);
			else
				return report.text;
		case Qt::ForegroundRole:
			return settings->color_palette.color(report.color);
		default:
			return {};
		}
	case Columns::Type:
		switch (role) {
		case Qt::DisplayRole:
			return report.type;
		default:
			return {};
		}
	default:
		return {};
	}
}

void ReportModel::update(const dfproto::Reports::ReportList &report_list)
{
	auto report = _reports.begin();
	const auto &df_reports = report_list.reports();
	auto df_report = df_reports.begin();
	while (true) {
		auto [report_equal_end, df_report_equal_end] = std::mismatch(
				report, _reports.end(),
				df_report, df_reports.end(),
				[](const auto &a, const auto &b){return a.id == b.id();});
		{
			auto start_index = index(std::distance(_reports.begin(), report), 0);
			while (report != report_equal_end)
				(report++)->update(*(df_report++));
			auto end_index = index(std::distance(_reports.begin(), report), static_cast<int>(Columns::Count)-1);
			dataChanged(start_index, end_index);
		}
		if (report == _reports.end() && df_report == df_reports.end())
			break;
		if (report == _reports.end() || df_report->id() < report->id) {
			auto insert_end = report == _reports.end()
				? df_reports.end()
				: std::lower_bound(df_report, df_reports.end(), report->id,
					[](const auto &report, int id){return report.id() < id;});
			auto row = std::distance(_reports.begin(), report);
			auto count = std::distance(df_report, insert_end);
			beginInsertRows({}, row, row + count - 1);
			report = _reports.insert(report, count, {});
			for (int i = 0; i < count; ++i) {
				auto &new_report = *(report++);
				new_report.init(*(df_report++));
				_type_list.addType(new_report.type);
			}
			endInsertRows();
		}
		else if (df_report == df_reports.end() || df_report->id() > report->id) {
			auto remove_end = df_report == df_reports.end()
				? _reports.end()
				: std::lower_bound(report, _reports.end(), df_report->id(),
					[](const auto &report, int id){return report.id < id;});
			auto first = std::distance(_reports.begin(), report);
			auto last = std::distance(_reports.begin(), remove_end) - 1;
			beginRemoveRows({}, first, last);
			report = _reports.erase(report, remove_end);
			endRemoveRows();
		}
	}
}

void ReportModel::clear()
{
	beginResetModel();
	_reports.clear();
	endResetModel();
}

void ReportModel::report::init(const dfproto::Reports::Report &df_report)
{
	id = df_report.id();
	time = DF::tick(df_report.time()) + DF::year(df_report.year());
	text = QString::fromUtf8(df_report.text());
	type = QByteArray::fromStdString(df_report.type());
	color = df_report.color() + (df_report.bright() ? 8 : 0);
	repeat = df_report.repeat();
}

void ReportModel::report::update(const dfproto::Reports::Report &df_report)
{
	repeat = df_report.repeat();
}
