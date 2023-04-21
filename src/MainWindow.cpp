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

#include "MainWindow.h"

#include <QClipboard>
#include <QMessageBox>
#include <QScrollBar>
#include <QSortFilterProxyModel>

#include "ui_MainWindow.h"
#include "ui_AboutDialog.h"

#include "Application.h"
#include "ReportModel.h"
#include "AnnouncementTypeList.h"
#include "SettingsDialog.h"

MainWindow::MainWindow(QWidget *parent):
	QMainWindow(parent),
	_ui(std::make_unique<Ui::MainWindow>()),
	_report_filter(*_game_manager.typeList())
{
	_ui->setupUi(this);

	auto settings = Application::instance()->settings();

	// Report views
	auto model = _game_manager.reports();
	_report_filter.setSourceModel(model);
	_report_filter.setFilterKeyColumn(static_cast<int>(ReportModel::Columns::Text));
	_report_filter.setFilterCaseSensitivity(Qt::CaseInsensitive);
	connect(_ui->edit_filter_text, &QLineEdit::textChanged,
		&_report_filter, &QSortFilterProxyModel::setFilterWildcard);
	connect(&_report_filter, &QAbstractItemModel::rowsAboutToBeRemoved,
		[this](const QModelIndex &parent, int start, int end) {
			// Clear current index if the row is removed so
			// QItemSelectionModel does not move it instead,
			// triggering an unwanted auto-scroll. This slot must
			// be executed before the view/selection can receive
			// the signal (it must be connected before setModel).
			auto selection_model = _ui->view_reports->selectionModel();
			auto current = selection_model->currentIndex();
			if (current.isValid() && current.parent() == parent
			    && current.row() >= start && current.row() <= end) {
				selection_model->setCurrentIndex({}, QItemSelectionModel::NoUpdate);
			}
		});
	_ui->view_reports->setModel(&_report_filter);
	_ui->view_reports->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
	_ui->view_reports->header()->setStretchLastSection(true);
	_ui->view_reports->header()->setContextMenuPolicy(Qt::CustomContextMenu);
	_ui->view_reports->header()->setSectionHidden(static_cast<int>(ReportModel::Columns::Id), true);
	_ui->view_reports->header()->setSectionHidden(static_cast<int>(ReportModel::Columns::Type), true);
	connect(_ui->view_reports->header(), &QWidget::customContextMenuRequested,
		[this](const QPoint &pos) {
			auto model = _game_manager.reports();
			auto header = _ui->view_reports->header();
			auto column_count = header->count();
			QMenu menu;
			for (int i = 0; i < column_count; ++i) {
				auto action = new QAction(model->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString(), &menu);
				action->setCheckable(true);
				action->setChecked(!header->isSectionHidden(i));
				connect(action, &QAction::toggled, [i, header](bool checked) {
						header->setSectionHidden(i, !checked);
					});
				menu.addAction(action);
			}
			menu.exec(header->mapToGlobal(pos));
		});
	_ui->view_reports->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(_ui->view_reports, &QWidget::customContextMenuRequested,
		[this](const QPoint &pos) {
			_ui->menu_edit->exec(_ui->view_reports->mapToGlobal(pos));
		});


	// Filters
	_type_filter.setSourceModel(_game_manager.typeList());
	_type_filter.setFilterCaseSensitivity(Qt::CaseInsensitive);

	_ui->list_types->setModel(&_type_filter);
	connect(_ui->edit_filter_types, &QLineEdit::textChanged,
		&_type_filter, &QSortFilterProxyModel::setFilterWildcard);
	connect(_ui->button_filter_types_check_all, &QAbstractButton::clicked,
		[this](){ setAllTypes(true); });
	connect(_ui->button_filter_types_uncheck_all, &QAbstractButton::clicked,
		[this](){ setAllTypes(false); });

	// View menu
	_ui->menu_view->addAction(_ui->toolbar->toggleViewAction());
	_ui->menu_view->addSeparator();
	_ui->dock_filters->setVisible(false);
	_ui->menu_view->addAction(_ui->dock_filters->toggleViewAction());

	// Game manager
	connect(_ui->action_disconnect, &QAction::triggered, &_game_manager, &GameManager::disconnect);
	connect(&_game_manager, &GameManager::stateChanged,
		this, &MainWindow::updateConnectionState);
	updateConnectionState(_game_manager.state());
	if (settings->autoconnect())
		on_action_connect_triggered();
	connect(&_game_manager, &GameManager::error,
		[this](const QString &message) {
			QMessageBox::critical(this, "Error", message);
		});

	connect(_ui->action_refresh, &QAction::triggered, &_game_manager, &GameManager::update);

	// Auto refresh
	connect(&_auto_refresh, &QTimer::timeout,
		&_game_manager, &GameManager::update);
	connect(&settings->autorefresh_interval, &SettingPropertyBase::valueChanged,
		this, &MainWindow::updateAutoRefreshInterval);
	updateAutoRefreshInterval();
	_auto_refresh.setSingleShot(false);
	connect(&settings->autorefresh_enabled, &SettingPropertyBase::valueChanged,
		this, &MainWindow::updateAutoRefreshEnabled);
	updateAutoRefreshEnabled();

	// Follow new reports
	connect(model, &QAbstractItemModel::rowsInserted,
		this, &MainWindow::updateViewScrollPosition);
	connect(_ui->action_follow, &QAction::toggled,
		this, &MainWindow::updateViewScrollPosition);
	connect(_ui->view_reports->verticalScrollBar(), &QAbstractSlider::rangeChanged,
		this, &MainWindow::updateViewScrollPosition);
	connect(_ui->view_reports->verticalScrollBar(), &QAbstractSlider::actionTriggered,
		[this](int action) {
			_ui->action_follow->setChecked(false);
		});
}

MainWindow::~MainWindow()
{
}

void MainWindow::setAllTypes(bool checked)
{
	int count = _type_filter.rowCount();
	for (int i = 0; i < count; ++i) {
		auto index = _type_filter.index(i, 0);
		_type_filter.setData(index, checked, Qt::CheckStateRole);
	}
}

void MainWindow::on_action_connect_triggered()
{
	auto settings = Application::instance()->settings();
	_game_manager.connect(settings->host_address(), settings->host_port());
}

void MainWindow::on_action_autorefresh_toggled(bool checked)
{
	auto settings = Application::instance()->settings();
	settings->autorefresh_enabled = checked;
}

void MainWindow::on_action_open_settings_triggered()
{
	SettingsDialog dialog(this);
	dialog.exec();
}

void MainWindow::on_action_copy_triggered()
{
	auto selection = _ui->view_reports->selectionModel()->selectedRows(static_cast<int>(ReportModel::Columns::Text));
	QString text;
	for (const auto &index: selection) {
		text.append(index.data().toString());
		text.append("\n");
	}
	QGuiApplication::clipboard()->setText(text);
}

void MainWindow::on_action_about_triggered()
{
	QDialog dialog;
	Ui::AboutDialog about_ui;
	about_ui.setupUi(&dialog);
	dialog.exec();
}

void MainWindow::updateConnectionState(GameManager::State state)
{
	_ui->action_connect->setEnabled(state != GameManager::Connecting);
	_ui->action_disconnect->setEnabled(state == GameManager::Connected);
	switch (state) {
	case GameManager::Disconnected:
		_ui->statusbar->showMessage(tr("Disconnected"));
		break;
	case GameManager::Connecting:
		_ui->statusbar->showMessage(tr("Connecting..."));
		break;
	case GameManager::Connected:
		_ui->statusbar->showMessage(tr("Connected (DF %1 - DFHack %2)")
				.arg(_game_manager.getDFVersion())
				.arg(_game_manager.getDFHackVersion()));
		break;
	}
	if (state == GameManager::Connected)
		_game_manager.update();
}

void MainWindow::updateAutoRefreshInterval()
{
	auto interval = Application::instance()->settings()->autorefresh_interval();
	_auto_refresh.setInterval(interval*1000);
}

void MainWindow::updateAutoRefreshEnabled()
{
	auto enabled = Application::instance()->settings()->autorefresh_enabled();
	_ui->action_autorefresh->setChecked(enabled);
	if (enabled)
		_auto_refresh.start();
	else
		_auto_refresh.stop();
}

void MainWindow::updateViewScrollPosition()
{
	if (_ui->action_follow->isChecked()) {
		_ui->view_reports->scrollToBottom();
	}
}

