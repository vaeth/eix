// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#ifndef SRC_EIXTK_ANSICOLOR_H_
#define SRC_EIXTK_ANSICOLOR_H_ 1

#include <config.h>

#include <ostream>
#include <string>

#include "eixTk/formated.h"
#include "eixTk/inttypes.h"

class AnsiColor;

/**
A class for using ANSI color codes and marker
Example:
@code
  #include <iostream>
  AnsiColor::init_static(); // must be called exactly once
  AnsiColor special("yellow,1;blue;underline", 0);
  std::cout << special << "marked text" << AnsiColor::reset() << "normal text";
@endcode
**/
class AnsiColor {
	private:
		/**
		The parsed string
		**/
		std::string code;

		static const char *reset_string;

	public:
		static unsigned int colorscheme;

		AnsiColor() {
		}

		AnsiColor(const std::string& str, std::string *errtext) {
			initcolor(str, errtext);
		}

		static const char *reset() {
			return reset_string;
		}

		const std::string& asString() const {
			return code;
		}

		bool initcolor(const std::string& str, std::string *errtext);

		static void init_static();

		static void AnsiPalette();

		typedef uint8_t WhichPalette;

		static const WhichPalette
			PALETTE_NONE = 0,
			PALETTE_D0   = 1,
			PALETTE_D1   = 2,
			PALETTE_D    = (PALETTE_D0|PALETTE_D1),
			PALETTE_L0   = 4,
			PALETTE_L1   = 8,
			PALETTE_L    = (PALETTE_L0|PALETTE_L1),
			PALETTE_B    = 16,
			PALETTE_ALL  = (PALETTE_D|PALETTE_L|PALETTE_B);

		static void PrintPalette(WhichPalette which);
};

inline static std::ostream& operator<<(std::ostream& os, const AnsiColor& ac) {
	os << ac.asString();
	return os;
}

inline static eix::format& operator%(eix::format& format, const AnsiColor& ac) {  // NOLINT(runtime/references)
	format % ac.asString();
	return format;
}

#endif  // SRC_EIXTK_ANSICOLOR_H_
