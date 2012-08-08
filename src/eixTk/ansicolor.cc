// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include <cassert>
#include <cstdio>

#include <map>
#include <string>
#include <vector>

#include "eixTk/ansicolor.h"
#include "eixTk/formated.h"
#include "eixTk/i18n.h"
#include "eixTk/likely.h"
#include "eixTk/null.h"
#include "eixTk/stringutils.h"

using std::map;
using std::string;
using std::vector;

const char *AnsiMarker::reset_string("\x1B[0m");

static map<string, AnsiMarker::Marker> *static_marker_map = NULLPTR;

static void
init_marker_map()
{
	assert(static_marker_map == NULLPTR);  // must be called only once
	static_marker_map = new map<string, AnsiMarker::Marker>;
	map<string, AnsiMarker::Marker> &marker_map(*static_marker_map);
	marker_map[""]           = AnsiMarker::amNone;
	marker_map["none"]       = AnsiMarker::amNone;
	marker_map["bold"]       = AnsiMarker::amBold;
	marker_map["underline"]  = AnsiMarker::amUnderlined;
	marker_map["underlined"] = AnsiMarker::amUnderlined;
	marker_map["blink"]      = AnsiMarker::amBlink;
	marker_map["inverse"]    = AnsiMarker::amInverse;
	marker_map["invert"]     = AnsiMarker::amInverse;
}

inline static AnsiMarker::Marker
name_to_marker(const string &name)
{
	assert(static_marker_map != NULLPTR);  // has init_static() been called?
	map<string, AnsiMarker::Marker>::const_iterator f(static_marker_map->find(name));
	if(f != static_marker_map->end())
		return f->second;
	return AnsiMarker::amIllegal;
}

static map<string, AnsiColor::Color> *static_color_map = NULLPTR;

static void
init_color_map()
{
	assert(static_color_map == NULLPTR);  // must be called only once
	static_color_map = new map<string, AnsiColor::Color>;
	map<string, AnsiColor::Color> &color_map(*static_color_map);
	color_map["default"] = AnsiColor::acDefault;
	color_map[""]        = AnsiColor::acDefault;
	color_map["none"]    = AnsiColor::acNone;
	color_map["black"]   = AnsiColor::acBlack;
	color_map["red"]     = AnsiColor::acRed;
	color_map["green"]   = AnsiColor::acGreen;
	color_map["yellow"]  = AnsiColor::acYellow;
	color_map["blue"]    = AnsiColor::acBlue;
	color_map["purple"]  = AnsiColor::acPurple;
	color_map["cyan"]    = AnsiColor::acCyan;
	color_map["gray"]    = AnsiColor::acGray;
}

inline static AnsiColor::Color
name_to_color(const string &name)
{
	assert(static_color_map != NULLPTR);  // has init_static() been called?
	map<string, AnsiColor::Color>::const_iterator f(static_color_map->find(name));
	if(f != static_color_map->end()) {
		return f->second;
	}
	return AnsiColor::acIllegal;
}

bool
AnsiMarker::initmarker(const string &markers_string, string *errtext)
{
	markers.clear();
	have_something = false;
	vector<string> v;
	split_string(&v, markers_string, false, ",;:- \t\r\n");
	markers.assign(v.size(), amNone);
	vector<Marker>::size_type i(0);
	for(vector<string>::const_iterator it(v.begin());
		likely(it != v.end()); ++it) {
		Marker mk(name_to_marker(*it));
		if(mk == amIllegal) {
			mk = amNone;
			if(errtext != NULLPTR) {
				*errtext = eix::format(_("Illegal marker name %r")) % *it;
			}
			return false;
		}
		markers[i++] = mk;
	}
	calc_string();
	return true;
}

void
AnsiMarker::calc_string()
{
	string_begin.clear();
	for(vector<Marker>::const_iterator it(markers.begin());
		unlikely(it != markers.end()); ++it) {
		if(*it == amNone) {
			continue;
		}
		static const int kLen(20);
		char buf[kLen];
		snprintf(buf, kLen, "\x1B[%dm", static_cast<int>(*it));
		string_begin.append(buf);
		have_something = true;
	}
}

void
AnsiColor::calc_string()
{
	have_something = mk.have_something;
	if(fg == acNone) {
		string_begin = mk.asString();
		return;
	}
	have_something = true;
	static const int kLen(20);
	char buf[kLen];
	if(fg == acDefault)
		snprintf(buf, kLen, "\x1B[0m\x1B[%dm", static_cast<int>(light));
	else
		snprintf(buf, kLen, "\x1B[%d;%dm", static_cast<int>(light), static_cast<int>(fg));
	string_begin = buf;
	string_begin.append(mk.asString());
}

bool
AnsiColor::initcolor(const string &color_name, string *errtext)
{
	have_something = false;
	// look for brightness attribute
	string::size_type curr(color_name.find_first_of(",;"));
	string::size_type resize(curr);
	if((curr != string::npos) && (color_name[curr] == ',')) {
		++curr;
		if(color_name[curr] == '1') {
			light = true;
		} else if(color_name[curr] == '0') {
			light = false;
		} else {
			if(errtext != NULLPTR) {
				*errtext = eix::format(_("Invalid brightness value %r")) % color_name[curr];
			}
			return false;
		}
		curr = color_name.find(';', curr);
	} else {
		light = false;
	}
	if(curr != string::npos) {
		if(!mk.initmarker(color_name.substr(curr + 1), errtext)) {
			return false;
		}
	}
	const string *pure_color;
	string pure_color_save;
	if(resize != string::npos) {
		pure_color_save.assign(color_name, 0, resize);
		pure_color = &pure_color_save;
	} else {
		pure_color = &color_name;
	}
	fg = name_to_color(*pure_color);
	if(fg == acIllegal) {
		fg = acNone;
		if(errtext != NULLPTR) {
			*errtext = eix::format(_("Illegal color name %r")) % (*pure_color);
		}
		return false;
	}
	calc_string();
	return true;
}

void
AnsiColor::init_static()
{
	init_marker_map();
	init_color_map();
}
