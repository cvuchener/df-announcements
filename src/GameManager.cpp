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

#include "GameManager.h"

#include "Application.h"
#include "AnnouncementTypeList.h"
#include "ReportModel.h"

#include <QEventLoop>

GameManager::GameManager(QObject *parent):
	QObject(parent),
	_type_list(std::make_unique<AnnouncementTypeList>()),
	_reports(std::make_unique<ReportModel>(*_type_list)),
	_state(Disconnected),
	_get_version(&_dfhack),
	_get_df_version(&_dfhack),
	_get_announcements(&_dfhack),
	_get_reports(&_dfhack)
{
	auto settings = Application::instance()->settings();
	QObject::connect(
		&_dfhack, &DFHack::Client::connectionChanged,
		this, &GameManager::onConnectionChanged);
	QObject::connect(
		&_dfhack, &DFHack::Client::notification,
		this, &GameManager::onNotification);
	QObject::connect(
		&settings->report_source, &SettingPropertyBase::valueChanged,
		this, &GameManager::update);
	QObject::connect(
		&settings->autorefresh_interval, &SettingPropertyBase::valueChanged,
		this, &GameManager::onAutorefreshIntervalChanged);
	onAutorefreshIntervalChanged();
	QObject::connect(
		&settings->autorefresh_enabled, &SettingPropertyBase::valueChanged,
		this, &GameManager::onAutorefreshEnabledChanged);
	_refresh_timer.setSingleShot(true);
	QObject::connect(
		&_refresh_timer, &QTimer::timeout,
		this, &GameManager::update);
}

GameManager::~GameManager()
{
}

void GameManager::connect(const QString &host, quint16 port)
{
	switch (_state) {
	case Disconnected:
		// Proceed with connection
		break;
	case Connecting:
		// Do nothing
		return;
	case Connected: {
		// Disconnect before proceeding
		QEventLoop loop;
		QMetaObject::Connection conn = QObject::connect(this, &GameManager::stateChanged, [&]() {
			loop.quit();
			QObject::disconnect(conn);
		});
		_dfhack.disconnect();
		loop.exec();
		break;
	}
	}
	setState(Connecting);
	using VersionReply = DFHack::CallReply<dfproto::StringMessage>;
	_dfhack.connect(host, port).then(this, [this](bool connected) {
		if (!connected) {
			setState(Disconnected);
			throw tr("Connection failed");
		}
		return DFHack::bindAll(
			_get_version,
			_get_df_version,
			_get_announcements,
			_get_reports
		);
	}).unwrap().then(this, [this](bool success) {
		if (!success) {
			_dfhack.disconnect();
			throw tr("Failed to bind functions");
		}
		auto calls = QList<QFuture<VersionReply>>()
			<< _get_version.call().first
			<< _get_df_version.call().first;
		return QtFuture::whenAll(calls.begin(), calls.end());
	}).unwrap().then([this](const QList<QFuture<VersionReply>> &r) {
		auto version_result = r[0].result();
		auto df_version_result = r[1].result();
		if (!version_result || !df_version_result) {
			_dfhack.disconnect();
			throw tr("Failed to get versions");
		}
		_dfhack_version = QString::fromUtf8(version_result->value());
		_df_version = QString::fromUtf8(df_version_result->value());
		setState(Connected);
		update();
	}).onFailed([this](QString message) {
		error(message);
	});
}

void GameManager::disconnect()
{
	_dfhack.disconnect();
}

void GameManager::update()
{
	if (_state != Connected)
		return;
	auto settings = Application::instance()->settings();
	auto result = [this, settings]() {
		switch (settings->report_source()) {
		case ReportSource::Announcements:
			return _get_announcements.call().first;
		case ReportSource::Reports:
			return _get_reports.call().first;
		default:
			Q_UNREACHABLE();
		}
	}();
	using Reply = DFHack::CallReply<dfproto::Reports::ReportList>;
	result.then(this, [this, settings](Reply reply) {
		if (!reply) {
			error(tr("Failed to get reports"));
		}
		else {
			_reports->update(*reply);
		}
		if (settings->autorefresh_enabled())
			_refresh_timer.start();
	});
}

void GameManager::onConnectionChanged(bool connected)
{
	if (!connected) {
		_refresh_timer.stop();
		_reports->clear();
		setState(Disconnected);
	}
}

void GameManager::onNotification(DFHack::Color color, const QString &text)
{
	qInfo() << text;
}

void GameManager::onAutorefreshIntervalChanged()
{
	auto interval = Application::instance()->settings()->autorefresh_interval();
	_refresh_timer.setInterval(interval*1000);
}

void GameManager::onAutorefreshEnabledChanged()
{
	auto enabled = Application::instance()->settings()->autorefresh_enabled();
	if (enabled)
		_refresh_timer.start();
	else
		_refresh_timer.stop();
}

void GameManager::setState(State state)
{
	if (_state != state)
		stateChanged(_state = state);
}
