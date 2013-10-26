// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef SRC_EIXTK_PERCENTAGE_H_
#define SRC_EIXTK_PERCENTAGE_H_ 1

#include <string>

#include "eixTk/eixint.h"

class PercentStatus {
	public:
		typedef eix::Treesize Percentage;

		PercentStatus() {
			init();
		}

		void init(const std::string &header);

		void init(const std::string &format, Percentage total);

		void next();

		void next(const std::string &append_string);

		void finish(const std::string &append_string);

		void interprint_start();

		void interprint_end() {
			reprint();
		}

	private:
		void init();
		void reprint();

		Percentage m_total, m_current;
		std::string m_format, m_append, m_total_s;
		std::string::size_type m_size;
		bool m_verbose, m_finished;
};



#endif  // SRC_EIXTK_PERCENTAGE_H_
