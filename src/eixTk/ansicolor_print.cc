// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>

#include <config.h>

#include <cstdio>

#include "eixTk/ansicolor.h"
#include "eixTk/eixint.h"
#include "eixTk/i18n.h"

typedef unsigned int CalcType;

const AnsiColor::WhichPalette
	AnsiColor::PALETTE_NONE,
	AnsiColor::PALETTE_D0,
	AnsiColor::PALETTE_D1,
	AnsiColor::PALETTE_D,
	AnsiColor::PALETTE_L0,
	AnsiColor::PALETTE_L1,
	AnsiColor::PALETTE_L,
	AnsiColor::PALETTE_B,
	AnsiColor::PALETTE_ALL;

class Display {
	private:
		bool bold, printed_palette;
		eix::SignedBool foreground;

		void output(CalcType color, CalcType fg, CalcType dark, CalcType light);

		void syscol();

		void cube(CalcType red_s, CalcType red_e);

		void cube();

		void ramp();

		static void nl();

		static void resetnl();

	public:
		Display() {
			printed_palette = false;
		}

		void palette(eix::SignedBool f, bool b, const char *s);
};

void Display::resetnl() {
	puts(AnsiColor::reset());
}

void Display::nl() {
	puts("");
}

void Display::output(CalcType color, CalcType fg, CalcType dark, CalcType light) {
	printf("\x1B[%s38;5;%d;48;5;%dm%3d ",
		(bold ? "1;" : ""),
		static_cast<int>(foreground ? color : fg),
		static_cast<int>(foreground ? ((foreground > 0) ? dark : light) : color),
		static_cast<int>(color));
}

void Display::syscol() {
	CalcType fg(7), dark(238), light;
	for(CalcType color(0); color < 16; fg = dark = 0, ++color) {
		switch(color) {
			case 3:
			case 7:
			case 11:
			case 15:
				light = 248;
				break;
			case 8:
				resetnl();
			default:
				light = 7;
		}
		output(color, fg, dark, light);
	}
	resetnl();
	nl();
}

void Display::cube(CalcType red_s, CalcType red_e) {
	for(CalcType green(0); green < 6; ++green) {
		for(CalcType red(red_s);;) {
			for(CalcType blue(0); blue < 6; ++blue) {
				output((16 + (red * 36) + (green * 6) + blue),
					((((green * 4) + (red * 3) + (blue * 1) < 12)) ? 7 : 0),
					(((green != 0) || (red != 0) || (blue > 2)) ? 0 : 238),
					((((green * 4) + (red * 3) + (blue * 1) > 26)) ? 244 : 7));
			}
			fputs(AnsiColor::reset(), stdout);
			if(++red == red_e) {
				break;
			}
			fputc(' ', stdout);
		}
		nl();
	}
	nl();
}

void Display::cube() {
	cube(0, 3);
	cube(3, 6);
}

void Display::ramp() {
	CalcType fg(7), dark(238), light(7);
	for(CalcType color(232); color < 256; ++color) {
		output(color, fg, dark, light);
		switch(color) {
			case 234:
				dark = 0;
				break;
			case 243:
				fg = 0;
				resetnl();
				break;
			case 247:
				light = 244;
				break;
			default:
				break;
		}
	}
	resetnl();
}

void Display::palette(eix::SignedBool f, bool b, const char *s) {
	foreground = f;
	bold = b;
	if(printed_palette) {
		nl();
	} else {
		printed_palette = true;
	}
	printf(_("System Colors (%s):\n"), s);
	syscol();
	printf(_("6x6x6 Color Cube (%s):\n"), s);
	cube();
	printf(_("Grayscale Ramp (%s):\n"), s);
	ramp();
}

void AnsiColor::PrintPalette(WhichPalette which) {
	Display display;
	if((which & PALETTE_L0) != PALETTE_NONE) {
		display.palette(-1, false, _("light, normal"));
	}
	if((which & PALETTE_L1) != PALETTE_NONE) {
		display.palette(-1, true, _("light, bright"));
	}
	if((which & PALETTE_D0) != PALETTE_NONE) {
		display.palette(1, false, _("dark, normal"));
	}
	if((which & PALETTE_D1) != PALETTE_NONE) {
		display.palette(1, true, _("dark, bright"));
	}
	if((which & PALETTE_B) != PALETTE_NONE) {
		display.palette(0, false, _("background"));
	}
}
