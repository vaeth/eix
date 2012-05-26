// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef EIX__FORMATSTRING_H__
#define EIX__FORMATSTRING_H__ 1

#include <eixTk/exceptions.h>
#include <eixTk/null.h>
#include <portage/basicversion.h>
#include <portage/extendedversion.h>
#include <portage/package.h>
#include <portage/set_stability.h>

#include <map>
#include <set>
#include <stack>
#include <string>
#include <vector>

typedef signed char FullFlag;
typedef unsigned char FormatTypeFlags;

class DBHeader;
class EixRc;
class Package;
class PortageSettings;
class VarDbPkg;
class Version;

class Node {
	public:
		enum Type { TEXT, OUTPUT, SET, IF } type;
		Node *next;
		Node(Type t) {
			type = t;
			next = NULLPTR;
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
		bool user_variable;

		Property(std::string n = "", bool user_var = false) : Node(OUTPUT) {
			name = n;
			user_variable = user_var;
		}
};

class ConditionBlock : public Node {
	public:
		bool final;

		Property variable;
		Text     text;
		enum Rhs { RHS_STRING, RHS_PROPERTY, RHS_VAR } rhs;
		Node     *if_true, *if_false;
		bool user_variable, negation;

		ConditionBlock() : Node(IF) {
			final = false;
			if_true = if_false = NULLPTR;
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
		Node         *root_node;
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

		Node *rootnode()
		{ return root_node; }

		/** Calculate line and column of current position. */
		int getPosition(int *line, int *column);
};

class MarkedList : public std::multimap<std::string, BasicVersion*>
{
	public:
		MarkedList() {}
		~MarkedList()
		{
			for(const_iterator it(begin()); it != end(); ++it)
			{
				if(it->second)
					delete it->second;
			}
		}

		void add(const char *pkg, const char *ver);
		/** Return pointer to (newly allocated) sorted vector of marked versions,
		    or NULLPTR. With nonversion argument, its content will decide whether
		    the package was marked with a non-version argument */
		std::set<BasicVersion> *get_marked_versions(const Package &pkg, bool *nonversion = NULLPTR) const;
		/** Return true if pkg is marked. If ver is non-NULLPTR also *ver must match */
		bool is_marked(const Package &pkg, const BasicVersion *ver = NULLPTR) const;
		/** Return String of marked versions (sorted) */
		std::string getMarkedString(const Package &pkg) const;
	private:
		typedef const_iterator CI;
		typedef std::pair<CI, CI> CIPair;
		CIPair equal_range_pkg(const Package &pkg) const;
};

class VarParserCacheNode {
	public:
		FormatParser m_parser;
		bool in_use;

		Node *init(const char *fmt, bool colors, bool use) throw(ExBasic)
		{
			in_use = use;
			return m_parser.start(fmt, colors);
		}
};

class VarParserCache : public std::map<std::string,VarParserCacheNode>
{
	public:
		void clear_use()
		{
			for(std::map<std::string,VarParserCacheNode>::iterator it = begin();
				it != end(); ++it)
				it->second.in_use = false;
		}
};

class VersionVariables;

class PrintFormat {
	friend class LocalCopy;
	friend std::string get_package_property(const PrintFormat *fmt, void *entity, const std::string &name);
	friend std::string get_diff_package_property(const PrintFormat *fmt, void *void_entity, const std::string &name);

	public:
		typedef std::string (*GetProperty)(const PrintFormat *fmt, void *entity, const std::string &property);

	protected:
		mutable std::map<std::string,std::string> user_variables;
		/* Looping over variables is a bit tricky:
		   We store the parsed thing in VarParserCache.
		   Additionally, we store there whether we currently loop
		   over the variable to avoid recursion. */
		mutable VarParserCache varcache;
		mutable VersionVariables *version_variables;
		FormatParser   m_parser;
		GetProperty    m_get_property;
		std::vector<bool> *virtuals;
		std::vector<ExtendedVersion::Overlay> *overlay_translations;
		std::vector<bool> *overlay_used;
		bool          *some_overlay_used;
		MarkedList    *marked_list;
		EixRc         *eix_rc;
		/* The following four variables are actually a hack:
		   This is only set temporarily during printing to avoid
		   passing this argument through all sub-functions.
		   We do it that way since we do not want to set it "globally":
		   The pointers might possibly have changed until we use them */
		const DBHeader *header;
		VarDbPkg       *vardb;
		const PortageSettings *portagesettings;
		const SetStability *stability;

		/* return true if something was actually printed */
		bool recPrint(std::string *result, void *entity, GetProperty get_property, Node *root) const;

		Node *parse_variable(const std::string &varname) const throw(ExBasic);

		std::string get_inst_use(const Package &package, InstVersion &i) const;
		void get_installed(Package *package, Node *root, bool mark) const;
		void get_versions_versorted(Package *package, Node *root, std::vector<Version*> *versions) const;
		void get_versions_slotsorted(Package *package, Node *root, std::vector<Version*> *versions) const;
		std::string get_pkg_property(Package *package, const std::string &name) const throw(ExBasic);
	public:
		bool	no_color,            /**< Shall we use colors? */
			style_version_lines, /**< Shall we show versions linewise? */
			slot_sorted,         /**< Print sorted by slots */
			alpha_use;           /**< Print use in alphabetical order (not by set/unset) */

		LocalMode recommend_mode;

		std::string
			color_overlaykey,  /**< Color for the overlay key */
			color_virtualkey,  /**< Color for the virtual key */
			before_set_use, after_set_use,
			before_unset_use, after_unset_use;

		PrintFormat(GetProperty get_callback = NULLPTR)
		{
			m_get_property = get_callback;
			virtuals = NULLPTR;
			overlay_translations = NULLPTR;
			overlay_used = NULLPTR;
			some_overlay_used = NULLPTR;
			marked_list = NULLPTR;
			eix_rc = NULLPTR;
			vardb = NULLPTR;
			portagesettings = NULLPTR;
			stability = NULLPTR;
		}

		// Initialize those variables common to eix and eix-diff:
		void setupResources(EixRc &eixrc);

		void setupColors();

		void clear_virtual(ExtendedVersion::Overlay count)
		{
			if(virtuals)
				delete virtuals;
			virtuals = new std::vector<bool>(count, false);
		}

		void set_as_virtual(const ExtendedVersion::Overlay overlay, bool on = true)
		{
			if(!overlay)
				return;
			(*virtuals)[overlay-1] = on;
		}

		bool is_virtual(const ExtendedVersion::Overlay overlay) const
		{
			if(!virtuals)
				return false;
			if((!overlay) || (overlay >= virtuals->size()))
				return false;
			return (*virtuals)[overlay - 1];
		}

		void set_overlay_translations(std::vector<ExtendedVersion::Overlay> *translations)
		{ overlay_translations = translations; }

		void set_overlay_used(std::vector<bool> *used, bool *some)
		{ overlay_used = used; some_overlay_used = some; }

		void set_marked_list(MarkedList *m_list)
		{ marked_list = m_list; }

		std::string overlay_keytext(ExtendedVersion::Overlay overlay, bool plain = false) const;

		/* return true if something was actually printed */
		bool print(void *entity, GetProperty get_property, Node *root, const DBHeader *dbheader, VarDbPkg *vardbpkg, const PortageSettings *ps, const SetStability *s);

		/* return true if something was actually printed */
		bool print(void *entity, Node *root, const DBHeader *dbheader, VarDbPkg *vardbpkg, const PortageSettings *ps, const SetStability *s)
		{ return print(entity, m_get_property, root, dbheader, vardbpkg, ps, s); }

		/* return true if something was actually printed */
		bool print(void *entity, const DBHeader *dbheader, VarDbPkg *vardbpkg, const PortageSettings *ps, const SetStability *s)
		{ return print(entity, m_parser.rootnode(), dbheader, vardbpkg, ps, s); }

		Node *parseFormat(const char *fmt) throw(ExBasic)
		{ return m_parser.start(fmt, !no_color); }

		void setFormat(const char *fmt) throw(ExBasic)
		{ parseFormat(fmt); }

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



#endif /* EIX__FORMATSTRING_H__ */
