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

#ifndef SETTINGS_DIALOG_H
#define SETTINGS_DIALOG_H

#include <QDialog>
#include <QIntValidator>

namespace Ui { class SettingsDialog; }
class ColorDelegate;

class SettingsDialog: public QDialog
{
	Q_OBJECT
public:
	SettingsDialog(QWidget *parent = nullptr);
	~SettingsDialog() override;

protected:
	void showEvent(QShowEvent *) override;

private slots:
	void on_button_reset_selected_colors_clicked();
	void on_button_reset_all_colors_clicked();

private:
	void loadSettings();
	void saveSettings() const;
	void restoreSettings() const;

	std::unique_ptr<Ui::SettingsDialog> _ui;
	std::unique_ptr<ColorDelegate> _color_delegate;
	QIntValidator _port_validator;
};

#endif
