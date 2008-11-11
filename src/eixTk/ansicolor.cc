// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include "ansicolor.h"

#include <cstdio>

using namespace std;

const char *AnsiMarker::reset_string = "\x1B[0m";

inline static const map<string,AnsiMarker::Marker>&
static_marker_map()
{
	static map<string,AnsiMarker::Marker> marker_map;
	marker_map[""]           = AnsiMarker::amNone;
	marker_map["none"]       = AnsiMarker::amNone;
	marker_map["bold"]       = AnsiMarker::amBold;
	marker_map["underline"]  = AnsiMarker::amUnderlined;
	marker_map["underlined"] = AnsiMarker::amUnderlined;
	marker_map["blink"]      = AnsiMarker::amBlink;
	marker_map["inverse"]    = AnsiMarker::amInverse;
	marker_map["invert"]     = AnsiMarker::amInverse;
	return marker_map;
}

inline static AnsiMarker::Marker
name_to_marker(const string &name)
{
	static const map<string,AnsiMarker::Marker> &marker_map = static_marker_map();
	map<string,AnsiMarker::Marker>::const_iterator f = marker_map.find(name);
	if(f != marker_map.end())
		return f->second;
	return AnsiMarker::amIllegal;
}

inline static const map<string,AnsiColor::Color>&
static_color_map()
{
	static map<string,AnsiColor::Color> color_map;
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
	return color_map;
}

inline static AnsiColor::Color
name_to_color(const string &name)
{
	static const map<string,AnsiColor::Color> &color_map = static_color_map();
	map<string,AnsiColor::Color>::const_iterator f = color_map.find(name);
	if(f != color_map.end())
		return f->second;
	return AnsiColor::acIllegal;
}

void
AnsiMarker::initmarker(const string &markers_string) throw (ExBasic)
{
	markers.clear(); have_something = false;
	vector<string> v = split_string(markers_string, false, ",;:- \t\r\n");
	markers.assign(v.size(), amNone);
	vector<Marker>::size_type i = 0;
	for(vector<string>::const_iterator it = v.begin();
		it != v.end(); ++it) {
		Marker mk = name_to_marker(*it);
		if(mk == amIllegal) {
			mk = amNone;
			throw ExBasic("Illegal marker name %r") % *it;
		}
		markers[i++] = mk;
	}
	calc_string();
}

void
AnsiMarker::calc_string()
{
	string_begin.clear();
	for(vector<Marker>::const_iterator it = markers.begin();
		it != markers.end(); ++it) {
		if(*it == amNone)
			continue;
		static const int len = 20;
		char buf[len];
		snprintf(buf, len, "\x1B[%dm", int(*it));
		string_begin.append(buf);
		have_something = true;
	}
}

void AnsiColor::calc_string()
{
	have_something = mk.have_something;
	if(fg == acNone) {
		string_begin = mk.asString();
		return;
	}
	have_something = true;
	static const int len = 20;
	char buf[len];
	if(fg == acDefault)
		snprintf(buf, len, "\x1B[0m\x1B[%dm", int(light));
	else
		snprintf(buf, len, "\x1B[%d;%dm", int(light), int(fg));
	string_begin = buf;
	string_begin.append(mk.asString());
}

AnsiColor::AnsiColor(const string &color_name) throw (ExBasic)
{
	// look for brightness attribute
	string::size_type curr = color_name.find_first_of(",;");
	string::size_type resize = curr;
	if((curr != string::npos) && (color_name[curr] == ','))
	{
		curr++;
		if(color_name[curr] == '1')
			light = true;
		else if(color_name[curr] == '0')
			light = false;
		else {
			throw ExBasic("Invalid brightness value %r") % color_name[curr];
		}
		curr = color_name.find(';', curr);
	}
	else
		light = false;
	if(curr != string::npos)
		mk.initmarker(color_name.substr(curr + 1));
	const string *pure_color;
	string pure_color_save;
	if(resize != string::npos) {
		pure_color_save = color_name.substr(0,resize);
		pure_color = &pure_color_save;
	}
	else
		pure_color = &color_name;
	fg = name_to_color(*pure_color);
	if(fg == acIllegal) {
		fg = acNone;
		throw ExBasic("Illegal color name %r %r") % (*pure_color);
	}
	calc_string();
}
