/***************************************************************************
 *   eix is a small utility for searching ebuilds in the                   *
 *   Gentoo Linux portage system. It uses indexing to allow quick searches *
 *   in package descriptions with regular expressions.                     *
 *                                                                         *
 *   https://sourceforge.net/projects/eix                                  *
 *                                                                         *
 *   Copyright (c)                                                         *
 *     Wolfgang Frisch <xororand@users.sourceforge.net>                    *
 *     Emil Beinroth <emilbeinroth@gmx.net>                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef __FORMATSTRING_H__
#define __FORMATSTRING_H__

#include <stack>
#include <iostream>

#include <portage/package.h>
#include <eixTk/exceptions.h>
#include <eixTk/ansicolor.h>

class Node {
	public:
		enum Type { TEXT, VARIABLE, IF } type;
		Node *next;
		Node(Type t) {
			type = t;
			next = NULL;
		}
		~Node() {
			if(next)
				delete next;
		}
};

class Text : public Node {
	public:
		std::string text;

		Text(std::string t = "") : Node(TEXT) {
			text = t;
		}
};

class Property : public Node {
	public:
		std::string name;

		Property(std::string n = "") : Node(VARIABLE) {
			name = n;
		}
};

class ConditionBlock : public Node {
	public:
		bool final;

		Property variable;
		Text     text;
		Node     *if_true, *if_false;
		bool negation;

		ConditionBlock() : Node(IF) {
			final = false;
			if_true = if_false = NULL;
		}

		~ConditionBlock() {
			if(if_true)
				delete if_true;
			if(if_false)
				delete if_false;
		}
};

class FormatParser {
	private:
		typedef enum ParserState {
			ERROR, STOP, START,
			TEXT, COLOR, PROPERTY,
			IF, ELSE, FI
		};

		std::stack<Node*>  keller;
		ParserState   state;
		const char   *band;
		char         *band_position;
		bool          enable_colors;
		std::string        last_error;

		/* Decide what state should be used to parse the current type of token. */
		ParserState state_START();
		/* Parse string, color and property. */
		ParserState state_TEXT();
		ParserState state_COLOR();
		ParserState state_PROPERTY();
		/* Parse if-else-fi constructs. */
		ParserState state_IF();
		ParserState state_ELSE();
		ParserState state_FI();

	public:
		Node *start(const char *fmt, bool colors = true) throw(ExBasic);

		/* Calculate line and column of current position. */
		int getPosition(int *line, int *column) {
			char *x = (char*) band, *y = (char*) band;
			while(x <= band_position && x) {
				y = x;
				x = strchr(x, '\n');
				if(x) {
					x += 1;
					++*line;
					*column = band_position - y;
				}
			}
			return band_position - band;
		}
};

class PrintFormat {

	public:
		typedef void   (*PrintProperty)(PrintFormat *formatstring, void *entity, std::string &property);
		typedef std::string (*GetProperty)  (void *entity, std::string &property);

	protected:
		FormatParser   m_parser;
		PrintProperty  m_print_property;
		GetProperty    m_get_property;
		Node          *m_root;

		void recPrint(void *entity, PrintProperty print_property, GetProperty get_property, Node *root);

	public:
		bool no_color,            /**< Shall we use colors? */
			 style_version_lines; /**< Shall we show versions linewise? */

		std::string color_masked,     /**< Color for masked versions */
			   color_unstable,   /**< Color for unstable versions */
			   color_stable,     /**< Color for stable versions */
			   color_overlaykey; /**< Color for the overlay key */

		PrintFormat(GetProperty get_callback, PrintProperty print_callback)
			: m_print_property(print_callback), m_get_property(get_callback) { }

		void setupColors() {
			color_masked     = AnsiColor(color_masked).asString();
			color_unstable   = AnsiColor(color_unstable).asString();
			color_stable     = AnsiColor(color_stable).asString();
			color_overlaykey = AnsiColor(color_overlaykey).asString();
		}

		void print(void *entity, PrintProperty print_property, GetProperty get_property, Node *root) {
			recPrint(entity, print_property, get_property, root);
			fputc('\n', stdout);
		}

		void print(void *entity, Node *root) {
			recPrint(entity, m_print_property, m_get_property, root);
			fputc('\n', stdout);
		}

		void print(void *entity) {
			recPrint(entity, m_print_property, m_get_property, m_root);
			fputc('\n', stdout);
		}

		void setFormat(const char *fmt) throw(ExBasic) {
			m_root = parseFormat(fmt);
		}

		Node *parseFormat(const char *fmt) throw(ExBasic) {
			return m_parser.start(fmt, !no_color);
		}

	private:
};

#endif /* __FORMATSTRING_H__ */
