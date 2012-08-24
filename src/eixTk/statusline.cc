// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include <config.h>

#include <iostream>
#include <string>

#include "eixTk/i18n.h"
#include "eixTk/statusline.h"

using std::string;

using std::cout;

void
Statusline::print_force(const string &str)
{
	if(soft) {
		cout << "\033k" << header << str << "\033\\";
	}
	cout << "\033]0;" << header << str << '\007';
	flush(cout);
}

void
Statusline::print(const string &str)
{
	if(use) {
		if(header.empty()) {
			header = m_program;
			header.append(": ");
		}
		print_force(str);
	}
}

void
Statusline::user_statusline()
{
	header.clear();
	if(m_exit[0] == ' ') {
		print_force(m_exit.substr(1));
	} else {
		print_force(m_exit);
	}
}

void
Statusline::success()
{
	if(header.empty()) {
		return;
	}
	if(m_exit.empty()) {
		print_force(_("Finished"));
	} else {
		user_statusline();
	}
}

void
Statusline::failure()
{
	if(header.empty()) {
		return;
	}
	if(m_exit.empty()) {
		print_force(_("Failure"));
	} else {
		user_statusline();
	}
}
