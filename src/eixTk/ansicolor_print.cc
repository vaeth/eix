// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include <config.h>

#include <cstdio>

#include "eixTk/ansicolor.h"
#include "eixTk/i18n.h"

typedef unsigned int CalcType;

class Display {
private:
	bool foreground, bold, printed_palette;

	void output(CalcType color, CalcType fg, CalcType bg);

	void syscol();

	void cube(CalcType red_s, CalcType red_e);

	void cube();

	void ramp();

	static void nl();

	static void resetnl();

public:
	Display()
	{ printed_palette = false; }

	void palette(bool f, bool b, const char *s);
};

void
Display::resetnl()
{
	puts(AnsiColor::reset());
}

void
Display::nl()
{
	puts("");
}

void
Display::output(CalcType color, CalcType fg, CalcType bg)
{
	printf("\x1B[%s38;5;%d;48;5;%dm%3d ",
		(bold ? "1;" : ""),
		static_cast<int>(foreground ? color : fg),
		static_cast<int>(foreground ? bg : color),
		static_cast<int>(color));
}

void
Display::syscol()
{
	CalcType fg(7), bg(238);
	for(CalcType color(0); color < 16; fg = bg = 0, ++color) {
		if(color == 8) {
			resetnl();
		}
		output(color, fg, bg);
	}
	resetnl();
	nl();
}

void
Display::cube(CalcType red_s, CalcType red_e)
{
	for(CalcType green(0); green < 6; ++green) {
		for(CalcType red(red_s);;) {
			for(CalcType blue(0); blue < 6; ++blue) {
				output((16 + (red * 36) + (green * 6) + blue),
					((((green * 4) + (red * 3) + (blue * 1) < 12)) ? 7 : 0),
					(((green != 0) || (red != 0) || (blue > 2)) ? 0 : 238));
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

void
Display::cube()
{
	cube(0, 3);
	cube(3, 6);
}

void
Display::ramp()
{
	CalcType fg(7), bg(238);
	for(CalcType color(232); color < 256; ++color) {
		output(color, fg, bg);
		if(color == 234) {
			bg = 0;
		}
		if(color == 243) {
			fg = 0;
			resetnl();
		}
	}
	resetnl();
}

void
Display::palette(bool f, bool b, const char *s)
{
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

void
AnsiColor::PrintPalette(enum WhichPalette which)
{
	Display display;
	if((which & PALETTE_F0) != PALETTE_NONE) {
		display.palette(true, false, _("foreground, normal"));
	}
	if((which & PALETTE_F1) != PALETTE_NONE) {
		display.palette(true, true, _("foreground, bright"));
	}
	if((which & PALETTE_B) != PALETTE_NONE) {
		display.palette(false, false, _("background"));
	}
}
