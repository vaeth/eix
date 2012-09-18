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
	bool foreground, bold;

public:
	void set_foreground(bool f)
	{ foreground = f; }

	void set_bold(bool b)
	{ bold = b; }

	void output(CalcType color, CalcType fg, CalcType bg);

	void syscol();

	void cube(CalcType red_s, CalcType red_e);

	void cube();

	void ramp();

	static void nl();

	static void resetnl();
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
	CalcType fg(7);
	for(CalcType color(0); color < 16; fg = 0, ++color) {
		if(color == 8) {
			resetnl();
		}
		output(color, fg, fg);
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
					(((green != 0) || (red != 0) || (blue > 2)) ? 0 : 7));
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
	CalcType fg(7);
	CalcType bg(7);
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
AnsiColor::PrintPalette(enum WhichPalette which)
{
	Display display;
	if((which & PALETTE_F) != PALETTE_NONE) {
		display.set_foreground(true);
		display.set_bold(false);
		puts(_("System Colors (normal):"));
		display.syscol();
		puts(_("6x6x6 Color Cube (normal):"));
		display.cube();
		puts(_("Grayscale Ramp (normal):"));
		display.ramp();
		Display::nl();
		display.set_bold(true);
		puts(_("System Colors (bright):"));
		display.syscol();
		puts(_("6x6x6 Color Cube (bright):"));
		display.cube();
		puts(_("Grayscale Ramp (bright):"));
		display.ramp();
		if(which != PALETTE_F) {
			Display::nl();
		}
	}
	if((which & PALETTE_B) != PALETTE_NONE) {
		display.set_foreground(false);
		display.set_bold(false);
		puts(_("System Colors (background):"));
		display.syscol();
		puts(_("6x6x6 Color Cube (background):"));
		display.cube();
		puts(_("Grayscale Ramp (background):"));
		display.ramp();
	}
}
