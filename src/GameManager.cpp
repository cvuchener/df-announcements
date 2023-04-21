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
	_dfhack.connect(host, port).then(this, [this](bool connected) {
		if (!connected) {
			setState(Disconnected);
			throw tr("Connection failed");
		}
		else {
			return DFHack::bindAll(
				_get_version,
				_get_df_version,
				_get_announcements,
				_get_reports
			);
		}
	}).unwrap().then(this, [this](bool success) {
		if (!success) {
			_dfhack.disconnect();
			throw tr("Failed to bind functions");
		}
		else {
			auto calls = QList<QFuture<DFHack::CallReply<dfproto::StringMessage>>>()
				<< _get_version.call().first
				<< _get_df_version.call().first;
			return QtFuture::whenAll(calls.begin(), calls.end());
		}
	}).unwrap().then([this](const QList<QFuture<DFHack::CallReply<dfproto::StringMessage>>> &r) {
		auto version_result = r[0].result();
		auto df_version_result = r[1].result();
		if (!version_result || !df_version_result) {
			_dfhack.disconnect();
			throw tr("Failed to get versions");
		}
		_dfhack_version = QString::fromUtf8(version_result->value());
		_df_version = QString::fromUtf8(df_version_result->value());
		setState(Connected);
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
	auto result = [this]() {
		auto settings = Application::instance()->settings();
		switch (settings->report_source()) {
		case ReportSource::Announcements:
			return _get_announcements.call().first;
		case ReportSource::Reports:
			return _get_reports.call().first;
		default:
			Q_UNREACHABLE();
		}
	}();
	result.then(this, [this](DFHack::CallReply<dfproto::Reports::ReportList> reply) {
		if (!reply) {
			error(tr("Failed to get reports"));
		}
		else {
			_reports->update(*reply);
		}
	});
}

void GameManager::onConnectionChanged(bool connected)
{
	if (!connected) {
		_reports->clear();
		setState(Disconnected);
	}
}

void GameManager::onNotification(DFHack::Color color, const QString &text)
{
	qInfo() << text;
}

void GameManager::setState(State state)
{
	if (_state != state)
		stateChanged(_state = state);
}
