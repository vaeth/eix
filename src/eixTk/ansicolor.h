// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef __ANSICOLOR_H__
#define __ANSICOLOR_H__

#include <iostream>
#include <map>
#include <vector>

/** A class for using ANSI markers
 * Example:
 * @code
 * AnsiMarker m_inverse(AnsiMarker::amInverse);
 * cout << m_inverse << "inverse text" << m_inverse.end() << "normal text";
 * @endcode
 */
class AnsiMarker {

	public:
		/** The various markers, amNone is no marker */
		enum Marker { amNone=0, amBold=1, amUnderlined=4, amBlink=5, amInverse=7, amIllegal };
		std::vector<Marker> markers;
		bool have_something;
		std::string string_begin;

		AnsiMarker(const Marker m = amNone)
		{
			if(m != amNone) {
				markers.assign(1, m);
				calc_string();
			}
		}

		void calc_string()
		{
			have_something = false;
			string_begin.clear();
			for(std::vector<Marker>::const_iterator it = markers.begin();
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

		static const char *reset()
		{ return "\x1B[0m"; }

		const std::string &asString() const
		{ return string_begin; }

		const char *end() const
		{
			if(have_something)
				return reset();
			return "";
		}

		static std::map<std::string,AnsiMarker::Marker>& init_map_once()
		{
			static std::map<std::string,AnsiMarker::Marker> marker_map;
			marker_map[""]        = AnsiMarker::amNone;
			marker_map["none"]    = AnsiMarker::amNone;
			marker_map["bold"]    = AnsiMarker::amBold;
			marker_map["underline"] = AnsiMarker::amUnderlined;
			marker_map["underlined"]= AnsiMarker::amUnderlined;
			marker_map["blink"]   = AnsiMarker::amBlink;
			marker_map["inverse"] = AnsiMarker::amInverse;
			marker_map["invert"]  = AnsiMarker::amInverse;
			return marker_map;
		}

		static Marker name_to_marker(const std::string &name)
		{
			static std::map<std::string,AnsiMarker::Marker> &marker_map = init_map_once();
			std::map<std::string,AnsiMarker::Marker>::const_iterator f = marker_map.find(name);
			if(f != marker_map.end())
				return f->second;
			return amIllegal;
		}

		/** Prints the current marker to an ostream */
		friend std::ostream& operator<< (std::ostream& os, AnsiMarker am);

		void initmarker(const std::string &markers_string) throw (ExBasic)
		{
			markers.clear();
			std::vector<std::string> v = split_string(markers_string, ",;:- \t\r\n");
			markers.assign(v.size(), amNone);
			std::vector<Marker>::size_type i = 0;
			for(std::vector<std::string>::const_iterator it = v.begin();
				it != v.end(); ++it) {
				Marker mk = name_to_marker(*it);
				if(mk == amIllegal) {
					mk = amNone;
					throw ExBasic("Illega marker name %r") % *it;
				}
				markers[i++] = mk;
			}
			calc_string();
		}

		AnsiMarker(const std::string &markers_string)
		{ initmarker(markers_string); }
};

inline std::ostream&
operator<< (std::ostream& os, AnsiMarker am)
{
	os << am.string_begin;
	return os;
}

/** A class for using ANSI color codes
 * @note Currently only handles foreground color.
 * Example:
 * @code
 * AnsiColor c_red(AnsiColor::acRed, false);
 * cout << "Normal text, " << c_red << "red text" << AnsiColor(AnsiColor::acDefault) << "normal text" << endl;
 * @endcode
 */
class AnsiColor {

	public:
		/** The various colors; acDefault is the default color, acNone is no color change */
		enum Color { acDefault=0, acBlack=30, acRed, acGreen, acYellow, acBlue, acPurple, acCyan, acGray, acNone, acIllegal };

		/** The current color */
		Color fg;

		/** true -> bright text */
		bool light;

		/** The actual string */
		std::string string_begin;

		/** The actual string */
		bool have_something;

		/** reset */
		static const char *reset()
		{ return AnsiMarker::reset(); }

		/** additional markers */
		AnsiMarker mk;

		/** Constructor
		 * @param fg foreground color
		 * @param light brightness (normal, bright)
		 * defaults to no color output */
		AnsiColor(const Color f = acNone, const bool l = false) : fg(f), light(l), mk(AnsiMarker::amNone)
		{ calc_string(); }

		void calc_string()
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

		const std::string &asString() const
		{ return string_begin; }

		static std::map<std::string,AnsiColor::Color>& init_map_once()
		{
			static std::map<std::string,AnsiColor::Color> color_map;
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

		static Color name_to_color(const std::string &name)
		{
			static std::map<std::string,AnsiColor::Color> &color_map = init_map_once();
			std::map<std::string,AnsiColor::Color>::const_iterator f = color_map.find(name);
			if(f != color_map.end())
				return f->second;
			return acIllegal;
		}

		/** Prints the current color to an ostream */
		friend std::ostream& operator<< (std::ostream& os, AnsiColor ac);

		AnsiColor(const std::string &color_name) throw (ExBasic)
		{
			light = false;

			// look for brightness attribute
			std::string::size_type curr = color_name.find_first_of(",;");
			std::string::size_type resize = curr;
			if((curr != std::string::npos) && (color_name[curr] == ','))
			{
				curr++;
				if(color_name[curr] == '1') {
					light = true;
				}
				else if(color_name[curr] == '0') {
					light = false;
				}
				else {
					throw ExBasic("Invalid brightness value %r")
						% color_name[curr];
				}
				curr = color_name.find(';', curr);
			}
			if(curr != std::string::npos)
				mk.initmarker(color_name.substr(curr + 1));
			const std::string *pure_color;
			std::string pure_color_save;
			if(resize != std::string::npos) {
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
};

inline std::ostream&
operator<< (std::ostream& os, AnsiColor ac)
{
	os << ac.string_begin;
	return os;
}

#endif /* __ANSICOLOR_H__ */
