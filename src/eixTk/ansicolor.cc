// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include <config.h>

#include <cstdio>

#include <map>
#include <string>

#include "eixTk/ansicolor.h"
#include "eixTk/assert.h"
#include "eixTk/eixint.h"
#include "eixTk/formated.h"
#include "eixTk/i18n.h"
#include "eixTk/inttypes.h"
#include "eixTk/likely.h"
#include "eixTk/null.h"
#include "eixTk/stringutils.h"

using std::map;
using std::string;

typedef uint8_t ColorType;
typedef unsigned int CalcType;

static CalcType transcalc(CalcType color);

unsigned int AnsiColor::colorscheme = 0;

const char *AnsiColor::reset_string = "\x1B[0m";

static map<string, ColorType> *static_color_map = NULLPTR;

static const ColorType
	acNone       = 0,
	amNone       = 50,
	amBold       = 1,
	amUnderlined = 4,
	amBlink      = 5,
	amInverse    = 7,
	fgFirst      = 30,
	fgBlack      = fgFirst + 0,
	fgRed        = fgFirst + 1,
	fgGreen      = fgFirst + 2,
	fgYellow     = fgFirst + 3,
	fgBlue       = fgFirst + 4,
	fgPurple     = fgFirst + 5,
	fgCyan       = fgFirst + 6,
	fgGray       = fgFirst + 7,
	fg256        = fgFirst + 8,
	bgFirst      = 40;

void
AnsiColor::init_static()
{
	eix_assert_static(static_color_map == NULLPTR);
	static_color_map = new map<string, ColorType>;
	map<string, ColorType> &color_map(*static_color_map);
	color_map[""]           = acNone;
	color_map["none"]       = amNone;
	color_map["default"]    = acNone;
	color_map["bold"]       = amBold;
	color_map["underline"]  = amUnderlined;
	color_map["underlined"] = amUnderlined;
	color_map["blink"]      = amBlink;
	color_map["inverse"]    = amInverse;
	color_map["invert"]     = amInverse;
	color_map["black"]      = fgBlack;
	color_map["red"]        = fgRed;
	color_map["green"]      = fgGreen;
	color_map["brown"]      = fgYellow;
	color_map["yellow"]     = fgYellow;
	color_map["blue"]       = fgBlue;
	color_map["purple"]     = fgPurple;
	color_map["cyan"]       = fgCyan;
	color_map["gray"]       = fgGray;
	color_map["white"]      = fgGray;
}

bool
AnsiColor::initcolor(const string &str, string *errtext)
{
	if(likely(str.empty())) {
		code.assign(reset_string);
		return true;
	}

	eix_assert_static(static_color_map != NULLPTR);
	string fg, bg, markers;
	bool ok(true);
	bool bold(false);
	bool havecol(false), noreset(false);
	string::size_type currpos(0);
	string::size_type endpos, first_endpos(string::npos);
	for(unsigned int i(colorscheme); ; --i) {
		endpos = str.find_first_of("|", currpos);
		if(i == 0) {
			break;
		}
		if(endpos == string::npos) {  // colorscheme is too high
			currpos = 0;
			endpos = first_endpos;
			break;
		}
		if(first_endpos == string::npos) {
			first_endpos = endpos;
		}
		currpos = endpos + 1;
	}
	if((currpos == endpos) || (currpos >= str.size())) {
		code.assign(reset_string);
		return true;
	}
	for(;;) {
		eix::SignedBool iscol(0);
		string::size_type nextpos(str.find_first_of(",;|", currpos));
		string curr(str, currpos, (nextpos == string::npos) ? string::npos : (nextpos - currpos));
		ColorType col;
		map<string, ColorType>::const_iterator f(static_color_map->find(curr));
		if(f != static_color_map->end()) {
			col = f->second;
			if(col == amNone) {
				if(!havecol) {
					noreset = havecol = true;
				}
			} else if(col == acNone) {
				col = amNone;
				iscol = 1;
				havecol = true;
			} else if(col >= fgFirst) {
				iscol = 1;
			} else if(col == amBold) {
				col = amNone;
				bold = true;
			}
		} else if(likely((curr.size() <= 3) && is_numeric(curr.c_str()))) {
			iscol = -1;
			col = fg256;
		} else {
			if(errtext != NULLPTR) {
				*errtext = eix::format(_("Illegal color name %r in %r")) % curr % str;
			}
			ok = false;
			break;
		}
		if(col != amNone) {
			if(iscol != 0) {
				if(havecol) {
					col += (bgFirst - fgFirst);
				}
			}
			static const unsigned int kLen = 10;
			char buf[kLen];
			if(iscol < 0) {
				snprintf(buf, kLen, ";%d;5;%s", static_cast<int>(col), curr.c_str());
			} else {
				snprintf(buf, kLen, ";%d", static_cast<int>(col));
			}
			if(iscol == 0) {
				markers.append(buf);
			} else if(!havecol) {
				fg.assign(buf);
				havecol = true;
			} else if(likely(bg.empty())) {
				bg.assign(buf);
			} else {
				if(errtext != NULLPTR) {
					*errtext = eix::format(_("More than two colors specified in %r")) % str;
				}
				ok = false;
				break;
			}
		}
		if(nextpos == endpos) {
			break;
		}
		currpos = nextpos + 1;
		if(str[nextpos] == ',') {
			nextpos = str.find_first_of(";|", currpos);
			curr.assign(str, currpos,  (nextpos == string::npos) ? string::npos : (nextpos - currpos));
			if(likely(curr == "1")) {
				bold = true;
			} else if(likely(curr == "0")) {
				bold = false;
			} else if(unlikely(curr.empty())) {
				if(errtext != NULLPTR) {
					*errtext = eix::format(_("Invalid brightness value %r")) % curr;
				}
				ok = false;
				break;
			}
			if(nextpos == endpos) {
				break;
			}
			currpos = nextpos + 1;
		}
	}
	code.assign("\x1B[");
	string::size_type skip(1);
	if(havecol && !noreset) {
		code.append(1, '0');
		skip = 0;
	}
	if(bold) {
		if(skip == 0) {
			code.append(1, ';');
		}
		code.append(1, '1');
		skip = 0;
	}
	if(!markers.empty()) {
		code.append(markers, skip, string::npos);
		skip = 0;
	}
	if(!fg.empty()) {
		code.append(fg, skip, string::npos);
		skip = 0;
	}
	if(!bg.empty()) {
		code.append(bg, skip, string::npos);
		skip = 0;
	}
	if(skip == 0) {
		code.append(1, 'm');
	} else {
		code.clear();
	}
	return ok;
}

static CalcType
transcalc(CalcType color)
{
	return ((color != 0) ? ((color * 40) + 55) : 0);
}

void
AnsiColor::AnsiPalette()
{
	for(CalcType red(0); red < 6; ++red) {
		for(CalcType green(0); green < 6; ++green) {
			for(CalcType blue(0); blue < 6; ++blue) {
				printf("\x1B]4;%d;rgb:%2.2x/%2.2x/%2.2x\x1B\\",
					static_cast<int>(16 + (red * 36) + (green * 6) + blue),
					static_cast<int>(transcalc(red)),
					static_cast<int>(transcalc(green)),
					static_cast<int>(transcalc(blue)));
			}
		}
	}
	for(CalcType gray(0); gray < 24; ++gray) {
		int trans(static_cast<int>((gray * 10) + 8));
		printf("\x1B]4;%d;rgb:%2.2x/%2.2x/%2.2x\x1B\\",
			static_cast<int>(232 + gray), trans, trans, trans);
	}
}
