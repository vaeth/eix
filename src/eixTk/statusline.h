// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef SRC_EIXTK_STATUSLINE_H_
#define SRC_EIXTK_STATUSLINE_H_ 1

#include <string>

class Statusline {
	private:
		bool use, soft;
		std::string header, m_program, m_exit;

		void print_force(const std::string& str);
		void user_statusline();
	public:
		Statusline(bool active, bool softstatus, const std::string& program_name, const std::string& exit_statusline)
			: use(active), soft(softstatus), m_program(program_name), m_exit(exit_statusline) {
		}

		void print(const std::string& str);

		void success();

		void failure();
};


#endif  // SRC_EIXTK_STATUSLINE_H_
