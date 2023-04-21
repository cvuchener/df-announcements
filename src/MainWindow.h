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

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QMenu>

namespace Ui { class MainWindow; }
class QLabel;

#include "GameManager.h"
#include "ReportFilterProxyModel.h"

class MainWindow: public QMainWindow
{
	Q_OBJECT
public:
	MainWindow(QWidget *parent = nullptr);
	~MainWindow() override;

private slots:
	void setAllTypes(bool checked);

private slots:
	void on_action_connect_triggered();
	void on_action_autorefresh_toggled(bool);
	void on_action_open_settings_triggered();
	void on_action_copy_triggered();
	void on_action_about_triggered();
	void updateConnectionState(GameManager::State state);
	void updateAutoRefreshAction();
	void updateViewScrollPosition();

private:
	std::unique_ptr<Ui::MainWindow> _ui;
	GameManager _game_manager;
	QSortFilterProxyModel _type_filter;
	ReportFilterProxyModel _report_filter;
	QLabel *_connection_status;
};

#endif
