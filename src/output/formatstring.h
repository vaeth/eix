// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef __FORMATSTRING_H__
#define __FORMATSTRING_H__

#include <stack>
#include <iostream>
#include <cstdlib>

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
		enum ParserState {
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

class MarkedList : public std::multimap<std::string, BasicVersion*>
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
	friend std::string get_extended_version(const PrintFormat *fmt, const ExtendedVersion *version, bool pure_text);
	friend std::string get_inst_use(const Package &p, InstVersion &i, const PrintFormat &fmt, const char **a);
	friend std::string getFullInstalled(const Package &p, const PrintFormat &fmt, bool with_slots, char full);
	friend std::string getInstalledString(const Package &p, const PrintFormat &fmt, bool pure_text, char formattype, const std::vector<std::string> &prepend);
	friend void print_version(const PrintFormat *fmt, const Version *version, const Package *package, bool with_slots, char full);
	friend void print_versions_versions(const PrintFormat *fmt, const Package *p, bool with_slots, char full, const std::vector<Version*> *versions);
	friend void print_versions_slots(const PrintFormat *fmt, const Package *p, bool with_slots, char full, const std::vector<Version*> *versions);
	friend void print_versions(const PrintFormat *fmt, const Package *p, bool with_slots, char full);
	friend bool print_package_property(const PrintFormat *fmt, const void *void_entity, const std::string &name) throw(ExBasic);
	friend std::string get_package_property(const PrintFormat *fmt, const void *entity, const std::string &name) throw(ExBasic);
	friend class LocalCopy;

	public:
		typedef bool      (*PrintProperty)(const PrintFormat *formatstring, const void *entity, const std::string &property);
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
		/* The following four variables are actually a hack:
		   This is only set temporarily during printing to avoid
		   passing this argument through all sub-functions.
		   We do it that way since we do not want to set it "globally":
		   The pointers might possibly have changed until we use them */
		DBHeader      *header;
		VarDbPkg      *vardb;
		PortageSettings *portagesettings;
		const SetStability  *stability;

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
		     alpha_use,           /**< Print use in alphabetical order (not by set/unset) */
		     print_iuse,          /**< Print iuse data */
		     print_restrictions;  /**< Print mirror/fetch restrictions */
		short print_keywords;     /**< Print keywords before/after (</>0) iuse data */

		LocalMode recommend_mode;

		std::string color_masked,     /**< Color for masked versions */
			   color_unstable,    /**< Color for unstable versions */
			   color_stable,      /**< Color for stable versions */
			   color_overlaykey,  /**< Color for the overlay key */
			   color_virtualkey,  /**< Color for the virtual key */
			   color_slots,       /**< Color for slots */
			   color_restrict_fetch,          /**< Color for RESTRICT=fetch */
			   color_restrict_mirror,         /**< Color for RESTRICT=mirror */
			   color_restrict_primaryuri,     /**< Color for RESTRICT=primaryuri */
			   color_restrict_binchecks,      /**< Color for RESTRICT=binchecks */
			   color_restrict_strip,          /**< Color for RESTRICT=strip */
			   color_restrict_test,           /**< Color for RESTRICT=test */
			   color_restrict_userpriv,       /**< Color for RESTRICT=userpriv */
			   color_restrict_installsources, /**< Color for RESTRICT=installsources */
			   color_restrict_bindist,        /**< Color for RESTRICT=bindist */
			   color_properties_interactive,  /**< Color for PROPERTIES=interactive */
			   color_properties_live,         /**< Color for PROPERTIES=live */
			   color_properties_virtual,      /**< Color for PROPERTIES=virtual */
			   color_properties_set;          /**< Color for PROPERTIES=set */
		std::string mark_installed,   /**< Marker for installed packages */
			   mark_installed_end,/**< End-Marker for installed packages */
			   mark_upgrade,      /**< Marker for upgrade candidate versions */
			   mark_upgrade_end,  /**< End-Marker for upgrade candidate versions */
			   mark_version,      /**< Marker for marked versions */
			   mark_version_end;  /**< End-Marker for marked versions */
		std::string dateFormat,       /**< The format of the long  install-date */
		            dateFormatShort;  /**< The format of the short install-date */
		std::string before_keywords, after_keywords,
		            before_iuse, after_iuse,
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
			tag_for_ex_missing_keyword,
			tag_restrict_fetch,
			tag_restrict_mirror,
			tag_restrict_primaryuri,
			tag_restrict_binchecks,
			tag_restrict_strip,
			tag_restrict_test,
			tag_restrict_userpriv,
			tag_restrict_installsources,
			tag_restrict_bindist,
			tag_properties_interactive,
			tag_properties_live,
			tag_properties_virtual,
			tag_properties_set;

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
			stability = NULL;
		}

		void setupColors() {
			bool use_color = !no_color;
			if(use_color) {
				color_masked     = AnsiColor(color_masked).asString();
				color_unstable   = AnsiColor(color_unstable).asString();
				color_stable     = AnsiColor(color_stable).asString();
				color_overlaykey = AnsiColor(color_overlaykey).asString();
				color_virtualkey = AnsiColor(color_virtualkey).asString();
				color_slots      = AnsiColor(color_slots).asString();
				color_restrict_fetch          = AnsiColor(color_restrict_fetch).asString();
				color_restrict_mirror         = AnsiColor(color_restrict_mirror).asString();
				color_restrict_primaryuri     = AnsiColor(color_restrict_primaryuri).asString();
				color_restrict_binchecks      = AnsiColor(color_restrict_binchecks).asString();
				color_restrict_strip          = AnsiColor(color_restrict_strip).asString();
				color_restrict_test           = AnsiColor(color_restrict_test).asString();
				color_restrict_userpriv       = AnsiColor(color_restrict_userpriv).asString();
				color_restrict_installsources = AnsiColor(color_restrict_installsources).asString();
				color_restrict_bindist        = AnsiColor(color_restrict_bindist).asString();
				color_properties_interactive  = AnsiColor(color_properties_interactive).asString();
				color_properties_live         = AnsiColor(color_properties_live).asString();
				color_properties_virtual      = AnsiColor(color_properties_virtual).asString();
				color_properties_set          = AnsiColor(color_properties_set).asString();
				AnsiMarker ins_marker(mark_installed);
				mark_installed     = ins_marker.asString();
				mark_installed_end = ins_marker.end();
				AnsiMarker upgrade_marker(mark_upgrade);
				mark_upgrade       = upgrade_marker.asString();
				mark_upgrade_end   = upgrade_marker.end();
				AnsiMarker ver_marker(mark_version);
				mark_version       = ver_marker.asString();
				mark_version_end   = ver_marker.end();
			}
			before_keywords    = parse_colors(before_keywords, use_color);
			after_keywords     = parse_colors(after_keywords, use_color);
			before_iuse        = parse_colors(before_iuse, use_color);
			after_iuse         = parse_colors(after_iuse, use_color);
			before_coll_iuse   = parse_colors(before_coll_iuse, use_color);
			after_coll_iuse    = parse_colors(after_coll_iuse, use_color);
			before_slot_iuse   = parse_colors(before_slot_iuse, use_color);
			after_slot_iuse    = parse_colors(after_slot_iuse, use_color);
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
		bool print(void *entity, PrintProperty print_property, GetProperty get_property, Node *root, DBHeader *dbheader, VarDbPkg *vardbpkg, PortageSettings *ps, const SetStability *s) {
			header = dbheader; vardb = vardbpkg; portagesettings = ps; stability = s;
			bool r = recPrint(entity, print_property, get_property, root);
			vardb=NULL; portagesettings = NULL; s = NULL;
			if(r)
				fputc('\n', stdout);
			return r;
		}

		/* return true if something was actually printed */
		bool print(void *entity, Node *root, DBHeader *dbheader, VarDbPkg *vardbpkg, PortageSettings *ps, const SetStability *s)
		{ return print(entity, m_print_property, m_get_property, root, dbheader, vardbpkg, ps, s); }

		/* return true if something was actually printed */
		bool print(void *entity, DBHeader *dbheader, VarDbPkg *vardbpkg, PortageSettings *ps, const SetStability *s)
		{ return print(entity, m_root, dbheader, vardbpkg, ps, s); }

		void setFormat(const char *fmt) throw(ExBasic) {
			m_root = parseFormat(fmt);
		}

		Node *parseFormat(const char *fmt) throw(ExBasic) {
			return m_parser.start(fmt, !no_color);
		}

		void StabilityLocal(Package &p) const
		{ stability->set_stability(true, p); }

		void StabilityNonlocal(Package &p) const
		{ stability->set_stability(false, p); }

	private:
};

class LocalCopy : public PackageSave
{
	public:
		LocalCopy(const PrintFormat *fmt, Package *pkg);
};



#endif /* __FORMATSTRING_H__ */
