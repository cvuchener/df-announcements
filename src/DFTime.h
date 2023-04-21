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

#ifndef DF_TIME_H
#define DF_TIME_H

#include <chrono>
#include <QString>

namespace DF {

using time = std::chrono::duration<int64_t>;
using tick = std::chrono::duration<int32_t>;
using day = std::chrono::duration<int32_t, std::ratio<1200>>;
using week = std::chrono::duration<int32_t, std::ratio<1200*7>>;
using month = std::chrono::duration<int32_t, std::ratio<1200*28>>;
using season = std::chrono::duration<int32_t, std::ratio<1200*28*3>>;
using year = std::chrono::duration<int32_t, std::ratio<1200*28*12>>;

constexpr std::array<QStringView, 4> Seasons = {
	u"Spring",
	u"Summer",
	u"Autumn",
	u"Winter",
};

constexpr std::array<QStringView, 12> Months = {
	u"Granite",
	u"Slate",
	u"Felsite",
	u"Hematite",
	u"Malachite",
	u"Galena",
	u"Limestone",
	u"Sandstone",
	u"Timber",
	u"Moonstone",
	u"Opal",
	u"Obsidian",
};

template <typename... Ds>
struct date: std::tuple<Ds...>
{
	template <typename Rep, typename Period>
	date(std::chrono::duration<Rep, Period> duration) {
		[this, &duration]<std::size_t... Is>(std::index_sequence<Is...>) {
			([this, &duration]<std::size_t I>(std::integral_constant<std::size_t, I>){
				using D = std::tuple_element_t<I, std::tuple<Ds...>>;
				std::get<I>(*this) = std::chrono::duration_cast<D>(duration);
				duration -= std::get<I>(*this);
			}(std::integral_constant<std::size_t, Is>{}), ...);
		}(std::index_sequence_for<Ds...>{});
	}

	template <typename Rep, typename Period>
	operator std::chrono::duration<Rep, Period>() const {
		return [this]<std::size_t... Is>(std::index_sequence<Is...>) {
			return (std::chrono::duration<Rep, Period>(std::get<Is>(*this)) + ...);
		}(std::index_sequence_for<Ds...>{});
	}
};

template <typename Rep, typename Period>
QString prettyDate(std::chrono::duration<Rep, Period> duration) {
	auto d = date<year, month, day>(duration);
	return QString("%1%2 %3 %4")
		.arg(std::get<2>(d).count()+1)
		.arg([](int32_t day){
			switch (day) {
			case 1:
			case 21:
				return "st";
			case 2:
			case 22:
				return "nd";
			case 3:
			case 23:
				return "rd";
			default:
				return "th";
			}
		}(std::get<2>(d).count()+1))
		.arg(Months[std::get<1>(d).count()])
		.arg(std::get<0>(d).count());
}

}

#endif
