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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QSettings>

class SettingPropertyBase: public QObject
{
	Q_OBJECT
public:
	SettingPropertyBase(QObject *parent = nullptr);
	~SettingPropertyBase() override;

signals:
	void valueChanged();
};

template <typename T>
class SettingProperty: public SettingPropertyBase
{
public:
	SettingProperty(const QString &name, T default_value, QObject *parent = nullptr):
		SettingPropertyBase(parent),
		_name(name),
		_default_value(default_value),
		_value(QSettings().value(name, QVariant::fromValue(default_value)).template value<T>())
	{
	}
	~SettingProperty() override = default;

	// getters
	const T &value() const noexcept { return _value; }
	const T &operator()() const noexcept { return _value; }

	const T &defaultValue() const noexcept { return _default_value; }

	// setters
	template <typename U> requires std::assignable_from<T &, U &&>
	void setValue(U &&value) {
		if (value != _value) {
			_value = std::forward<U>(value);
			QSettings().setValue(_name, QVariant::fromValue(_value));
			valueChanged();
		}
	}
	template <typename U> requires std::assignable_from<T &, U &&>
	SettingProperty &operator=(U &&value) {
		setValue(std::forward<U>(value));
		return *this;
	}

private:
	const QString _name;
	const T _default_value;
	T _value;
};

#include "ColorPaletteModel.h"
#include "AnnouncementTypeList.h"

enum class ReportSource {
	Announcements,
	Reports,
};
Q_DECLARE_METATYPE(ReportSource)

struct Settings
{
	SettingProperty<QString> host_address = {"host/address", "localhost"};
	SettingProperty<quint16> host_port = {"host/port", 5000};
	SettingProperty<bool> autoconnect = {"host/connect_on_startup", false};

	SettingProperty<ReportSource> report_source = {"report/source", ReportSource::Announcements};

	SettingProperty<bool> autorefresh_enabled = {"autorefresh/enabled", true};
	SettingProperty<double> autorefresh_interval = {"autorefresh/interval", 2.0};

	ColorPaletteModel color_palette;
	AnnouncementTypeList announcement_types;
};

#endif
