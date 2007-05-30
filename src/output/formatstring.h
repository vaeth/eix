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
 *     Martin Väth <vaeth@mathematik.uni-wuerzburg.de>                     *
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
#include <portage/set_stability.h>
#include <eixTk/exceptions.h>
#include <eixTk/ansicolor.h>

class DBHeader;
class VarDbPkg;
class PortageSettings;

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

std::string parse_colors(const std::string &colorstring, bool colors);

class FormatParser {
		friend std::string parse_colors(const std::string &colorstring, bool colors);
	private:
		typedef enum ParserState {
			ERROR, STOP, START,
			TEXT, COLOR, PROPERTY,
			IF, ELSE, FI
		};

		std::stack<Node*>  keller;
		ParserState   state;
		const char   *band;
		const char   *band_position;
		bool          enable_colors;
		bool          only_colors;
		std::string   last_error;

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
		Node *start(const char *fmt, bool colors = true, bool parse_only_colors = false) throw(ExBasic);

		/* Calculate line and column of current position. */
		int getPosition(int *line, int *column) {
			const char *x = band, *y = band;
			while(x <= band_position && x) {
				y = x;
				x = strchr(x, '\n');
				if(x) {
					x ++;
					++*line;
					*column = band_position - y;
				}
			}
			return band_position - band;
		}
};

class MarkedList : std::multimap<std::string, BasicVersion*>
{
	public:
		MarkedList() {}
		~MarkedList()
		{
			for(const_iterator it = begin(); it != end(); ++it)
			{
				if(it->second)
					delete it->second;
			}
		}

		void add(const char *pkg, const char *ver);
		/** Return pointer to (newly allocated) sorted vector of marked versions,
		    or NULL. With nonversion argument, its content will decide whether
		    the package was marked with a non-version argument */
		std::vector<BasicVersion> *get_marked_vector(const Package &pkg, bool *nonversion = NULL) const;
		/** Return true if pkg is marked. If ver is non-NULL also *ver must match */
		bool is_marked(const Package &pkg, const BasicVersion *ver = NULL) const;
		/** Return String of marked versions (sorted) */
		std::string getMarkedString(const Package &pkg) const;
	private:
		typedef const_iterator CI;
		typedef std::pair<CI, CI> CIPair;
		CIPair equal_range_pkg(const Package &pkg) const;
};

class PrintFormat {
	friend std::string get_basic_version(const PrintFormat *fmt, const BasicVersion *version, bool pure_text);
	friend std::string get_inst_use(const Package &p, InstVersion &i, const PrintFormat &fmt, const char **a);
	friend std::string getInstalledString(const Package &p, const PrintFormat &fmt, bool pure_text, char formattype, const std::vector<std::string> &prepend);
	friend void print_version(const PrintFormat *fmt, const Version *version, const Package *p, bool with_slot, bool exclude_overlay);
	friend void print_versions_versions(const PrintFormat *fmt, const Package *p, bool with_slot);
	friend void print_versions_slots(const PrintFormat *fmt, const Package *p);
	friend void print_versions(const PrintFormat *fmt, const Package *p, bool with_slot);
	friend void print_package_property(const PrintFormat *fmt, const void *void_entity, const std::string &name) throw(ExBasic);
	friend std::string get_package_property(const PrintFormat *fmt, const void *entity, const std::string &name) throw(ExBasic);
	friend class LocalCopy;

	public:
		typedef void   (*PrintProperty)(const PrintFormat *formatstring, const void *entity, const std::string &property);
		typedef std::string (*GetProperty)(const PrintFormat *formatstring, const void *entity, const std::string &property);

	protected:
		FormatParser   m_parser;
		PrintProperty  m_print_property;
		GetProperty    m_get_property;
		Node          *m_root;
		std::vector<bool> *virtuals;
		std::vector<Version::Overlay> *overlay_translations;
		std::vector<bool> *overlay_used;
		bool          *some_overlay_used;
		MarkedList    *marked_list;
		/* The following two variables are actually a hack:
		   This is only set temporarily during printing to avoid
		   passing this argument through all sub-functions.
		   We do it that way since we do not want to set it "globally":
		   The pointers might possibly have changed until we use them */
		DBHeader      *header;
		VarDbPkg      *vardb;
		PortageSettings *portagesettings;
		const SetStability *stability_local, *stability_nonlocal;

		/* return true if something was actually printed */
		bool recPrint(void *entity, PrintProperty print_property, GetProperty get_property, Node *root);

	public:
		bool no_color,            /**< Shall we use colors? */
		     color_original,      /**< Color according to non-local settings? */
		     color_local_mask,    /**< Color [m] as masked? */
		     style_version_lines, /**< Shall we show versions linewise? */
		     show_slots,          /**< Shall we show slots at all? */
		     slot_sorted,         /**< Print sorted by slots */
		     colon_slots,         /**< Print slots separated with colons */
		     colored_slots,       /**< Print slots in separate color */
		     print_iuse;          /**< Print iuse data */

		LocalMode recommend_mode;

		std::string color_masked,     /**< Color for masked versions */
			   color_unstable,    /**< Color for unstable versions */
			   color_stable,      /**< Color for stable versions */
			   color_overlaykey,  /**< Color for the overlay key */
			   color_virtualkey,  /**< Color for the virtual key */
			   color_slots;       /**< Color for slots */
		std::string mark_installed,   /**< Marker for installed packages */
			   mark_installed_end,/**< End-Marker for installed packages */
			   mark_version,      /**< Marker for marked versions */
			   mark_version_end;  /**< End-Marker for marked versions */
		std::string dateFormat,       /**< The format of the long  install-date */
		            dateFormatShort;  /**< The format of the short install-date */
		std::string before_iuse, after_iuse,
		            before_coll_iuse, after_coll_iuse,
		            before_slot_iuse, after_slot_iuse;
		std::string tag_for_profile, tag_for_masked,
			tag_for_ex_profile, tag_for_ex_masked,
			tag_for_locally_masked, tag_for_stable,
			tag_for_unstable,
			tag_for_minus_asterisk, tag_for_minus_keyword,
			tag_for_alien_stable, tag_for_alien_unstable,
			tag_for_missing_keyword,
			tag_for_ex_unstable,
			tag_for_ex_minus_asterisk, tag_for_ex_minus_keyword,
			tag_for_ex_alien_stable, tag_for_ex_alien_unstable,
			tag_for_ex_missing_keyword;

		PrintFormat(GetProperty get_callback = NULL, PrintProperty print_callback = NULL)
		{
			m_print_property = print_callback;
			m_get_property = get_callback;
			virtuals = NULL;
			overlay_translations = NULL;
			overlay_used = NULL;
			some_overlay_used = NULL;
			marked_list = NULL;
			vardb = NULL;
			portagesettings = NULL;
			stability_local = NULL;
			stability_nonlocal = NULL;
		}

		~PrintFormat()
		{
			if(stability_local) {
				delete stability_local;
				stability_local = NULL;
			}
			if(stability_nonlocal) {
				delete stability_nonlocal;
				stability_nonlocal = NULL;
			}
		}

		void setupColors() {
			color_masked     = AnsiColor(color_masked).asString();
			color_unstable   = AnsiColor(color_unstable).asString();
			color_stable     = AnsiColor(color_stable).asString();
			color_overlaykey = AnsiColor(color_overlaykey).asString();
			color_virtualkey = AnsiColor(color_virtualkey).asString();
			color_slots      = AnsiColor(color_slots).asString();
			AnsiMarker ins_marker(mark_installed);
			mark_installed     = ins_marker.asString();
			mark_installed_end = ins_marker.end();
			AnsiMarker ver_marker(mark_version);
			mark_version       = ver_marker.asString();
			mark_version_end   = ver_marker.end();
			before_iuse        = parse_colors(before_iuse, true);
			after_iuse         = parse_colors(after_iuse, true);
			before_coll_iuse   = parse_colors(before_coll_iuse, true);
			after_coll_iuse    = parse_colors(after_coll_iuse, true);
			before_slot_iuse   = parse_colors(before_slot_iuse, true);
			after_slot_iuse    = parse_colors(after_slot_iuse, true);
		}

		void clear_virtual(Version::Overlay count)
		{
			if(virtuals)
				delete virtuals;
			virtuals = new std::vector<bool>(count, false);
		}

		void set_as_virtual(const Version::Overlay overlay, bool on = true)
		{
			if(!overlay)
				return;
			(*virtuals)[overlay-1] = on;
		}

		bool is_virtual(const Version::Overlay overlay) const
		{
			if(!virtuals)
				return false;
			if((!overlay) || (overlay >= virtuals->size()))
				return false;
			return (*virtuals)[overlay - 1];
		}

		void set_overlay_translations(std::vector<Version::Overlay> *translations)
		{ overlay_translations = translations; }

		void set_overlay_used(std::vector<bool> *used, bool *some)
		{ overlay_used = used; some_overlay_used = some; }

		void set_marked_list(MarkedList *m_list)
		{ marked_list = m_list; }

		std::string overlay_keytext(Version::Overlay overlay, bool never_color = false) const;

		/* return true if something was actually printed */
		bool print(void *entity, PrintProperty print_property, GetProperty get_property, Node *root, DBHeader *dbheader, VarDbPkg *vardbpkg, PortageSettings *ps) {
			header = dbheader; vardb = vardbpkg; portagesettings = ps;
			bool r = recPrint(entity, print_property, get_property, root);
			vardb=NULL; portagesettings = NULL;
			if(r)
				fputc('\n', stdout);
			return r;
		}

		/* return true if something was actually printed */
		bool print(void *entity, Node *root, DBHeader *dbheader, VarDbPkg *vardbpkg, PortageSettings *ps)
		{ return print(entity, m_print_property, m_get_property, root, dbheader, vardbpkg, ps); }

		/* return true if something was actually printed */
		bool print(void *entity, DBHeader *dbheader, VarDbPkg *vardbpkg, PortageSettings *ps)
		{ return print(entity, m_root, dbheader, vardbpkg, ps); }

		void setFormat(const char *fmt) throw(ExBasic) {
			m_root = parseFormat(fmt);
		}

		Node *parseFormat(const char *fmt) throw(ExBasic) {
			return m_parser.start(fmt, !no_color);
		}

		void StabilityLocal(Package &p) const
		{
			if(!stability_local) {
				(const_cast<PrintFormat*>(this))->stability_local =
					new SetStability(portagesettings, false, true);
			}
			stability_local->set_stability(p);
		}

		void StabilityNonlocal(Package &p) const
		{
			if(!stability_nonlocal) {
				(const_cast<PrintFormat*>(this))->stability_nonlocal =
					new SetStability(portagesettings, true, false);
			}
			stability_nonlocal->set_stability(p);
		}

	private:
};

class LocalCopy {
	private:
		bool is_a_copy;
	public:
		const Package *package;

		LocalCopy(const PrintFormat *fmt, const Package *pkg);

		~LocalCopy()
		{ if(is_a_copy) delete package; }
};



#endif /* __FORMATSTRING_H__ */
