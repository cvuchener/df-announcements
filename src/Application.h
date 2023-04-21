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

#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>

#include "Settings.h"

class Application: public QApplication
{
	Q_OBJECT
public:
	Application(int &argc, char **argv);
	~Application() override;

	Settings *settings() { return _settings.get(); }

	static Application *instance() {
		return static_cast<Application *>(QCoreApplication::instance());
	}
private:
	std::unique_ptr<Settings> _settings;
};

#endif
