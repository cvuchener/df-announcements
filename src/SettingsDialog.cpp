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

#include "SettingsDialog.h"

#include "ui_SettingsDialog.h"
#include "Application.h"
#include "ColorDelegate.h"

SettingsDialog::SettingsDialog(QWidget *parent):
	QDialog(parent),
	_ui(std::make_unique<Ui::SettingsDialog>()),
	_color_delegate(std::make_unique<ColorDelegate>()),
	_port_validator(1, 65535)
{
	_ui->setupUi(this);
	_ui->edit_host_port->setValidator(&_port_validator);
	_ui->colors_view->setItemDelegate(_color_delegate.get());

	_ui->combo_report_source->addItem(tr("Announcements"), QVariant::fromValue(ReportSource::Announcements));
	_ui->combo_report_source->addItem(tr("Reports"), QVariant::fromValue(ReportSource::Reports));

	connect(this, &QDialog::accepted, this, &SettingsDialog::saveSettings);
	connect(this, &QDialog::rejected, this, &SettingsDialog::restoreSettings);
}

SettingsDialog::~SettingsDialog()
{
}

void SettingsDialog::showEvent(QShowEvent *e)
{
	loadSettings();
	QDialog::showEvent(e);
}

void SettingsDialog::on_button_reset_selected_colors_clicked()
{
	auto &model = Application::instance()->settings()->color_palette;
	auto selection = _ui->colors_view->selectionModel()->selection();
	for (const auto &index: selection.indexes())
		model.reset(index);
}

void SettingsDialog::on_button_reset_all_colors_clicked()
{
	auto &model = Application::instance()->settings()->color_palette;
	model.resetAll();
}

void SettingsDialog::loadSettings()
{
	auto settings = Application::instance()->settings();

	_ui->edit_host_address->setText(settings->host_address());
	_ui->edit_host_port->setText(QString::number(settings->host_port()));
	_ui->check_autoconnect->setChecked(settings->autoconnect());

	auto source_index = _ui->combo_report_source->findData(QVariant::fromValue(settings->report_source()));
	_ui->combo_report_source->setCurrentIndex(source_index);

	_ui->check_autorefresh->setChecked(settings->autorefresh_enabled());
	_ui->spin_autorefresh_rate->setEnabled(settings->autorefresh_enabled());
	_ui->spin_autorefresh_rate->setValue(settings->autorefresh_interval());

	_ui->colors_view->setModel(&settings->color_palette);
}

void SettingsDialog::saveSettings() const
{
	auto settings = Application::instance()->settings();

	settings->host_address = _ui->edit_host_address->text();
	settings->host_port = _ui->edit_host_port->text().toInt();
	settings->autoconnect = _ui->check_autoconnect->isChecked();

	settings->report_source = _ui->combo_report_source->currentData().value<ReportSource>();

	settings->autorefresh_enabled = _ui->check_autorefresh->isChecked();
	settings->autorefresh_interval = _ui->spin_autorefresh_rate->value();

	settings->color_palette.save();
}

void SettingsDialog::restoreSettings() const
{
	auto settings = Application::instance()->settings();
	settings->color_palette.load();
}

