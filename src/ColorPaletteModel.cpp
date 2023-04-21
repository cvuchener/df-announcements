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

#include "ColorPaletteModel.h"

#include <QGuiApplication>
#include <QPalette>
#include <QSettings>

struct color_def {
	const char *prop_name;
	const char *display_name;
	std::array<QColor, 2> default_color;
};

static const std::array<color_def, ColorPaletteModel::ColorCount> ColorDefs = {
	color_def{"black", QT_TRANSLATE_NOOP("ColorPaletteModel", "Black"),
		QColor(255, 255, 255), QColor(0, 0, 0)},
	color_def{"blue", QT_TRANSLATE_NOOP("ColorPaletteModel", "Blue"),
		QColor(10, 76, 158), QColor(32, 125, 241)},
	color_def{"green", QT_TRANSLATE_NOOP("ColorPaletteModel", "Green"),
		QColor(99, 139, 24), QColor(162, 220, 52)},
	color_def{"cyan", QT_TRANSLATE_NOOP("ColorPaletteModel", "Cyan"),
		QColor(58, 121, 111), QColor(113, 187, 176)},
	color_def{"red", QT_TRANSLATE_NOOP("ColorPaletteModel", "Red"),
		QColor(163, 0, 28), QColor(255, 17, 58)},
	color_def{"magenta", QT_TRANSLATE_NOOP("ColorPaletteModel", "Magenta"),
		QColor(106, 30, 138), QColor(167, 60, 213)},
	color_def{"brown", QT_TRANSLATE_NOOP("ColorPaletteModel", "Brown"),
		QColor(133, 95, 25), QColor(215, 155, 45)},
	color_def{"lgray", QT_TRANSLATE_NOOP("ColorPaletteModel", "Light gray"),
		QColor(128, 128, 128), QColor(192, 192, 192)},
	color_def{"dgray", QT_TRANSLATE_NOOP("ColorPaletteModel", "Dark gray"),
		QColor(160, 160, 160), QColor(160, 160 ,160)},
	color_def{"lblue", QT_TRANSLATE_NOOP("ColorPaletteModel", "Light blue"),
		QColor(93, 41, 255), QColor(140, 102, 255)},
	color_def{"lgreen", QT_TRANSLATE_NOOP("ColorPaletteModel", "Light green"),
		QColor(2, 202, 71), QColor(19, 253, 101)},
	color_def{"lcyan", QT_TRANSLATE_NOOP("ColorPaletteModel", "Light cyan"),
		QColor(1, 203, 162), QColor(18, 254, 207)},
	color_def{"lred", QT_TRANSLATE_NOOP("ColorPaletteModel", "Light red"),
		QColor(209, 85, 0), QColor(255, 113, 17)},
	color_def{"lmagenta", QT_TRANSLATE_NOOP("ColorPaletteModel", "Light magenta"),
		QColor(194, 0, 214), QColor(232, 17, 255)},
	color_def{"yellow", QT_TRANSLATE_NOOP("ColorPaletteModel", "Yellow"),
		QColor(204, 178, 0), QColor(255, 225, 17)},
	color_def{"white", QT_TRANSLATE_NOOP("ColorPaletteModel", "White"),
		QColor(0, 0, 0), QColor(255, 255, 255)},
};

ColorPaletteModel::ColorPaletteModel(QObject *parent):
	QAbstractTableModel(parent)
{
	load();
}

ColorPaletteModel::~ColorPaletteModel()
{
}

int ColorPaletteModel::rowCount(const QModelIndex &) const
{
	return _colors.size();
}

int ColorPaletteModel::columnCount(const QModelIndex &) const
{
	return 2;
}

Qt::ItemFlags ColorPaletteModel::flags(const QModelIndex &index) const
{
	return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

QVariant ColorPaletteModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole)
		return {};
	switch (orientation) {
	case Qt::Horizontal:
		switch (section) {
		case 0: return "Light theme";
		case 1: return "Dark theme";
		default: return {};
		}
	case Qt::Vertical:
		return tr(ColorDefs[section].display_name);
	default:
		return {};
	}
}
QVariant ColorPaletteModel::data(const QModelIndex &index, int role) const
{
	static const std::array<QColor, 2> foreground = {Qt::black, Qt::white};
	static const std::array<QColor, 2> background = {Qt::white, Qt::black};
	const auto &color = _colors[index.row()][index.column()];
	switch (role) {
	case Qt::DisplayRole:
		return color.name();
	case Qt::EditRole:
	case Qt::DecorationRole:
		return color;
	case Qt::ForegroundRole:
		return foreground[index.column()];
	case Qt::BackgroundRole:
		return background[index.column()];
	default:
		return {};
	}
}

bool ColorPaletteModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (role != Qt::EditRole)
		return false;
	if (!value.canConvert<QColor>())
		return false;
	_colors[index.row()][index.column()] = value.value<QColor>();
	dataChanged(index, index);
	return true;
}

QColor ColorPaletteModel::color(int index) const
{
	const auto &color = _colors[index];
	// TODO: use QGuiApplication::styleHints()->colorScheme() (Qt >= 6.5)
	auto application_palette = static_cast<QGuiApplication *>(QCoreApplication::instance())->palette();
	if (application_palette.text().color().lightness() < application_palette.base().color().lightness()) {
		// light theme
		return color[0];
	}
	else {
		// dark theme
		return color[1];
	}
}

void ColorPaletteModel::reset(const QModelIndex &index)
{
	std::size_t row = index.row(), col = index.column();
	_colors[row][col] = ColorDefs[row].default_color[col];
	dataChanged(index, index);
}

void ColorPaletteModel::resetAll()
{
	for (std::size_t i = 0; i < ColorCount; ++i)
		for (std::size_t j = 0; j < 2; ++j)
			_colors[i][j] = ColorDefs[i].default_color[j];
	dataChanged(index(0, 0), index(ColorCount-1, 1));
}


static const char * const ColorPropertyName = "colors";
static const std::array<const char *, 2> ThemePropertyName = {"light", "dark"};

void ColorPaletteModel::load()
{
	QSettings settings;
	settings.beginGroup(ColorPropertyName);
	for (std::size_t i = 0; i < ColorCount; ++i) {
		auto &color = _colors[i];
		const auto &color_def = ColorDefs[i];
		settings.beginGroup(color_def.prop_name);
		for (std::size_t j = 0; j < 2; ++j) {
			color[j] = settings.value(ThemePropertyName[j], color_def.default_color[j]).value<QColor>();
		}
		settings.endGroup();
	}
	settings.endGroup();
}

void ColorPaletteModel::save() const
{
	QSettings settings;
	settings.beginGroup(ColorPropertyName);
	for (std::size_t i = 0; i < ColorCount; ++i) {
		const auto &color = _colors[i];
		const auto &color_def = ColorDefs[i];
		settings.beginGroup(color_def.prop_name);
		for (std::size_t j = 0; j < 2; ++j) {
			settings.setValue(ThemePropertyName[j], color[j]);
		}
		settings.endGroup();
	}
	settings.endGroup();
}
