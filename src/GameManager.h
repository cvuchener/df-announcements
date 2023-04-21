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

#ifndef GAME_MANAGER_H
#define GAME_MANAGER_H

#include <QObject>

#include <dfhack-client-qt/Client.h>
#include <dfhack-client-qt/Function.h>
#include <dfhack-client-qt/Basic.h>
#include "reports.pb.h"

class AnnouncementTypeList;
class ReportModel;

namespace Reports {
using GetAnnouncements = DFHack::Function<
	"Reports", "GetAnnouncements",
	dfproto::EmptyMessage,
	dfproto::Reports::ReportList>;
using GetReports = DFHack::Function<
	"Reports", "GetReports",
	dfproto::EmptyMessage,
	dfproto::Reports::ReportList>;
}

class GameManager: public QObject
{
	Q_OBJECT
	Q_PROPERTY(State state READ state NOTIFY stateChanged)
public:
	GameManager(QObject *parent = nullptr);
	~GameManager() override;

	enum State {
		Disconnected,
		Connecting,
		Connected,
	};
	Q_ENUM(State)
	State state() const { return _state; }

	const QString &getDFHackVersion() const { return _dfhack_version; };
	const QString &getDFVersion() const { return _df_version; };

	Q_INVOKABLE AnnouncementTypeList *typeList() { return _type_list.get(); }
	Q_INVOKABLE ReportModel *reports() { return _reports.get(); }

public slots:
	void connect(const QString &host, quint16 port);
	void disconnect();
	void update();

signals:
	void stateChanged(State);
	void error(const QString &);

private slots:
	void onConnectionChanged(bool);
	void onNotification(DFHack::Color color, const QString &text);

private:
	void setState(State state);

	std::unique_ptr<AnnouncementTypeList> _type_list;
	std::unique_ptr<ReportModel> _reports;

	State _state;
	QString _dfhack_version;
	QString _df_version;

	DFHack::Client _dfhack;
	DFHack::Basic::GetVersion _get_version;
	DFHack::Basic::GetDFVersion _get_df_version;
	Reports::GetAnnouncements _get_announcements;
	Reports::GetReports _get_reports;
};

#endif
