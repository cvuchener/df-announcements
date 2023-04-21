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

#ifndef COLOR_PALETTE_MODEL_H
#define COLOR_PALETTE_MODEL_H

#include <QAbstractTableModel>
#include <QColor>

class ColorPaletteModel: public QAbstractTableModel
{
	Q_OBJECT
public:
	ColorPaletteModel(QObject *parent = nullptr);
	~ColorPaletteModel() override;

	static constexpr std::size_t ColorCount = 16;

	int rowCount(const QModelIndex &) const override;
	int columnCount(const QModelIndex &) const override;
	Qt::ItemFlags flags(const QModelIndex &index) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

	QColor color(int index) const;

	void reset(const QModelIndex &index);
	void resetAll();

	void load();
	void save() const;

private:
	std::array<std::array<QColor, 2>, ColorCount> _colors;
};

#endif
