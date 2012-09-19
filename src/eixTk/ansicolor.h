// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef SRC_EIXTK_ANSICOLOR_H_
#define SRC_EIXTK_ANSICOLOR_H_ 1

#include <ostream>
#include <string>

class AnsiColor;

/** A class for using ANSI color codes and marker
 * Example:
 * @code
 * include <iostream>
 * AnsiColor::init_static(); // must be called exactly once
 * AnsiColor special("yellow,1;blue;underline", 0);
 * std::cout << special << "marked text" << AnsiColor::reset() << "normal text";
 * @endcode
 */
class AnsiColor {
	private:
		/** The parsed string */
		std::string code;

		static const char *reset_string;

	public:
		static unsigned int colorscheme;

		AnsiColor()
		{ }

		explicit AnsiColor(const std::string &str, std::string *errtext)
		{ initcolor(str, errtext); }

		static const char *reset()
		{ return reset_string; }

		const std::string &asString() const
		{ return code; }

		bool initcolor(const std::string &str, std::string *errtext);

		static void init_static();

		static void AnsiPalette();

		enum WhichPalette {
			PALETTE_NONE = 0,
			PALETTE_F0   = 1,
			PALETTE_F1   = 2,
			PALETTE_F    = 3,
			PALETTE_B    = 4,
			PALETTE_ALL  = 7
		};
		static void PrintPalette(enum WhichPalette which);
};

inline static std::ostream&
operator<<(std::ostream& os, const AnsiColor &ac)
{
	os << ac.asString();
	return os;
}

#endif  // SRC_EIXTK_ANSICOLOR_H_
