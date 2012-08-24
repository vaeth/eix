// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef SRC_OUTPUT_FORMATSTRING_H_
#define SRC_OUTPUT_FORMATSTRING_H_ 1

#include <sys/types.h>

#include <map>
#include <set>
#include <stack>
#include <string>
#include <utility>
#include <vector>

#include "eixTk/likely.h"
#include "eixTk/null.h"
#include "portage/basicversion.h"
#include "portage/extendedversion.h"
#include "portage/package.h"
#include "portage/set_stability.h"

class DBHeader;
class EixRc;
class MaskFlags;
class KeywordsFlags;
class Package;
class PortageSettings;
class VarDbPkg;
class Version;

class Node {
	public:
		enum Type { TEXT, OUTPUT, SET, IF } type;
		Node *next;
		explicit Node(Type t) {
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

		explicit Text(std::string t = "") : Node(TEXT) {
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

bool parse_colors(std::string *res, const std::string &colorstring, bool colors, std::string *errtext);

class FormatParser {
		friend bool parse_colors(std::string &res, const std::string &colorstring, bool colors, std::string *errtext);

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
		bool start(const char *fmt, bool colors, bool parse_only_colors, std::string *errtext);

		Node *rootnode()
		{ return root_node; }

		/** Calculate line and column of current position. */
		void getPosition(size_t *line, size_t *column);
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

		bool init(Node *&rootnode, const char *fmt, bool colors, bool use, std::string *errtext)
		{
			in_use = use;
			if(likely(m_parser.start(fmt, colors, false, errtext))) {
				rootnode = m_parser.rootnode();
				return true;
			}
			return false;
		}
};

class VarParserCache : public std::map<std::string, VarParserCacheNode>
{
	public:
		void clear_use()
		{
			for(std::map<std::string, VarParserCacheNode>::iterator it = begin();
				it != end(); ++it)
				it->second.in_use = false;
		}
};

class VersionVariables;

class PrintFormat {
	friend class LocalCopy;
	friend class Scanner;
	friend std::string get_package_property(const PrintFormat *fmt, void *entity, const std::string &name);
	friend std::string get_diff_package_property(const PrintFormat *fmt, void *void_entity, const std::string &name);

	public:
		typedef std::string (*GetProperty)(const PrintFormat *fmt, void *entity, const std::string &property);

	protected:
		mutable std::map<std::string, std::string> user_variables;
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

		bool parse_variable(Node **rootnode, const std::string &varname, std::string *errtext) const;
		Node *parse_variable(const std::string &varname) const;

		std::string get_inst_use(const Package &package, InstVersion *i) const;
		void get_installed(Package *package, Node *root, bool mark) const;
		void get_versions_versorted(Package *package, Node *root, std::vector<Version*> *versions) const;
		void get_versions_slotsorted(Package *package, Node *root, std::vector<Version*> *versions) const;
		std::string get_pkg_property(Package *package, const std::string &name) const;

		// It follows a list of indirect functions called in get_pkg_property():
		// Functions with capital letters are parser destinations; other functions
		// here are sort of "macros" used by several other "capital letter" functions.

		std::string COLON_VER_DATE(Package *package, const std::string &after_colon) const;
		void colon_pkg_availableversions(Package *package, const std::string &after_colon, bool only_marked) const;
		void COLON_PKG_AVAILABLEVERSIONS(Package *package, const std::string &after_colon) const;
		void COLON_PKG_MARKEDVERSIONS(Package *package, const std::string &after_colon) const;
		void colon_pkg_bestversion(Package *package, const std::string &after_colon, bool allow_unstable) const;
		void COLON_PKG_BESTVERSION(Package *package, const std::string &after_colon) const;
		void COLON_PKG_BESTVERSIONS(Package *package, const std::string &after_colon) const;
		void colon_pkg_bestslotversions(Package *package, const std::string &after_colon, bool allow_unstable) const;
		void COLON_PKG_BESTSLOTVERSIONS(Package *package, const std::string &after_colon) const;
		void COLON_PKG_BESTSLOTVERSIONSS(Package *package, const std::string &after_colon) const;
		void colon_pkg_bestslotupgradeversions(Package *package, const std::string &after_colon, bool allow_unstable) const;
		void COLON_PKG_BESTSLOTUPGRADEVERSIONS(Package *package, const std::string &after_colon) const;
		void COLON_PKG_BESTSLOTUPGRADEVERSIONSS(Package *package, const std::string &after_colon) const;
		void colon_pkg_installedversions(Package *package, const std::string &after_colon, bool only_marked) const;
		void COLON_PKG_INSTALLEDVERSIONS(Package *package, const std::string &after_colon) const;
		void COLON_PKG_INSTALLEDMARKEDVERSIONS(Package *package, const std::string &after_colon) const;
		std::string PKG_INSTALLED(Package *package) const;
		std::string PKG_VERSIONLINES(Package *package) const;
		std::string PKG_SLOTSORTED(Package *package) const;
		std::string PKG_COLOR(Package *package) const;
		std::string PKG_HAVEBEST(Package *package) const;
		std::string PKG_HAVEBESTS(Package *package) const;
		std::string PKG_CATEGORY(Package *package) const;
		std::string PKG_NAME(Package *package) const;
		std::string PKG_DESCRIPTION(Package *package) const;
		std::string PKG_HOMEPAGE(Package *package) const;
		std::string PKG_LICENSES(Package *package) const;
		std::string PKG_BINARY(Package *package) const;
		std::string PKG_OVERLAYKEY(Package *package) const;
		std::string PKG_SYSTEM(Package *package) const;
		std::string PKG_WORLD(Package *package) const;
		std::string PKG_WORLD_SETS(Package *package) const;
		std::string PKG_SETNAMES(Package *package) const;
		std::string PKG_ALLSETNAMES(Package *package) const;
		std::string pkg_upgrade(Package *package, bool only_installed, bool test_slots) const;
		std::string PKG_UPGRADE(Package *package) const;
		std::string PKG_UPGRADEORINSTALL(Package *package) const;
		std::string PKG_BESTUPGRADE(Package *package) const;
		std::string PKG_BESTUPGRADEORINSTALL(Package *package) const;
		std::string pkg_downgrade(Package *package, bool test_slots) const;
		std::string PKG_DOWNGRADE(Package *package) const;
		std::string PKG_BESTDOWNGRADE(Package *package) const;
		std::string pkg_recommend(Package *package, bool only_installed, bool test_slots) const;
		std::string PKG_RECOMMEND(Package *package) const;
		std::string PKG_RECOMMENDORINSTALL(Package *package) const;
		std::string PKG_BESTRECOMMEND(Package *package) const;
		std::string PKG_BESTRECOMMENDORINSTALL(Package *package) const;
		std::string PKG_MARKED(Package *package) const;
		std::string PKG_HAVEMARKEDVERSION(Package *package) const;
		std::string PKG_SLOTS(Package *package) const;
		std::string PKG_SLOTTED(Package *package) const;
		std::string PKG_HAVEVIRTUAL(Package *package) const;
		std::string PKG_HAVENONVIRTUAL(Package *package) const;
		std::string PKG_HAVECOLLIUSE(Package *package) const;
		std::string PKG_COLLIUSE(Package *package) const;
		const ExtendedVersion *ver_version() const ATTRIBUTE_PURE;
		std::string VER_FIRST(Package *package) const;
		std::string VER_LAST(Package *package) const;
		std::string VER_SLOTFIRST(Package *package) const;
		std::string VER_SLOTLAST(Package *package) const;
		std::string VER_ONESLOT(Package *package) const;
		const ExtendedVersion *ver_versionslot(Package *package) const;
		std::string VER_FULLSLOT(Package *package) const;
		std::string VER_ISFULLSLOT(Package *package) const;
		std::string VER_SLOT(Package *package) const;
		std::string VER_ISSLOT(Package *package) const;
		std::string VER_SUBSLOT(Package *package) const;
		std::string VER_ISSUBSLOT(Package *package) const;
		std::string VER_VERSION(Package *package) const;
		std::string VER_PLAINVERSION(Package *package) const;
		std::string VER_REVISION(Package *package) const;
		std::string ver_overlay(Package *package, bool getnum) const;
		std::string VER_OVERLAYNUM(Package *package) const;
		std::string VER_OVERLAYVER(Package *package) const;
		std::string VER_VERSIONKEYWORDSS(Package *package) const;
		std::string VER_VERSIONKEYWORDS(Package *package) const;
		std::string VER_VERSIONEKEYWORDS(Package *package) const;
		std::string ver_isbestupgrade(Package *package, bool check_slots, bool allow_unstable) const;
		std::string VER_ISBESTUPGRADESLOT(Package *package) const;
		std::string VER_ISBESTUPGRADESLOTS(Package *package) const;
		std::string VER_ISBESTUPGRADE(Package *package) const;
		std::string VER_ISBESTUPGRADES(Package *package) const;
		std::string VER_MARKEDVERSION(Package *package) const;
		std::string VER_INSTALLEDVERSION(Package *package) const;
		std::string VER_HAVEUSE(Package *package) const;
		std::string VER_USE(Package *package) const;
		std::string VER_VIRTUAL(Package *package) const;
		std::string VER_ISBINARY(Package *package) const;
		const ExtendedVersion *ver_restrict(Package *package) const;
		std::string ver_restrict(Package *package, ExtendedVersion::Restrict r) const;
		std::string VER_RESTRICT(Package *package) const;
		std::string VER_RESTRICTFETCH(Package *package) const;
		std::string VER_RESTRICTMIRROR(Package *package) const;
		std::string VER_RESTRICTPRIMARYURI(Package *package) const;
		std::string VER_RESTRICTBINCHECKS(Package *package) const;
		std::string VER_RESTRICTSTRIP(Package *package) const;
		std::string VER_RESTRICTTEST(Package *package) const;
		std::string VER_RESTRICTUSERPRIV(Package *package) const;
		std::string VER_RESTRICTINSTALLSOURCES(Package *package) const;
		std::string VER_RESTRICTBINDIST(Package *package) const;
		std::string VER_RESTRICTPARALLEL(Package *package) const;
		std::string ver_properties(Package *package, ExtendedVersion::Properties p) const;
		std::string VER_PROPERTIES(Package *package) const;
		std::string VER_PROPERTIESINTERACTIVE(Package *package) const;
		std::string VER_PROPERTIESLIVE(Package *package) const;
		std::string VER_PROPERTIESVIRTUAL(Package *package) const;
		std::string VER_PROPERTIESSET(Package *package) const;
		std::string VER_HAVEDEPEND(Package *package) const;
		std::string VER_HAVERDEPEND(Package *package) const;
		std::string VER_HAVEPDEPEND(Package *package) const;
		std::string VER_HAVEDEPS(Package *package) const;
		std::string VER_DEPENDS(Package *package) const;
		std::string VER_DEPEND(Package *package) const;
		std::string VER_RDEPENDS(Package *package) const;
		std::string VER_RDEPEND(Package *package) const;
		std::string VER_PDEPENDS(Package *package) const;
		std::string VER_PDEPEND(Package *package) const;
		const MaskFlags *ver_maskflags() const ATTRIBUTE_PURE;
		std::string VER_ISHARDMASKED(Package *package) const;
		std::string VER_ISPROFILEMASKED(Package *package) const;
		std::string VER_ISMASKED(Package *package) const;
		const KeywordsFlags *ver_keywordsflags() const ATTRIBUTE_PURE;
		std::string VER_ISSTABLE(Package *package) const;
		std::string VER_ISUNSTABLE(Package *package) const;
		std::string VER_ISALIENSTABLE(Package *package) const;
		std::string VER_ISALIENUNSTABLE(Package *package) const;
		std::string VER_ISMISSINGKEYWORD(Package *package) const;
		std::string VER_ISMINUSKEYWORD(Package *package) const;
		std::string VER_ISMINUSUNSTABLE(Package *package) const;
		std::string VER_ISMINUSASTERISK(Package *package) const;
		bool ver_wasflags(Package *package, MaskFlags *maskflags, KeywordsFlags *keyflags) const;
		std::string VER_WASHARDMASKED(Package *package) const;
		std::string VER_WASPROFILEMASKED(Package *package) const;
		std::string VER_WASMASKED(Package *package) const;
		std::string VER_WASSTABLE(Package *package) const;
		std::string VER_WASUNSTABLE(Package *package) const;
		std::string VER_WASALIENSTABLE(Package *package) const;
		std::string VER_WASALIENUNSTABLE(Package *package) const;
		std::string VER_WASMISSINGKEYWORD(Package *package) const;
		std::string VER_WASMINUSKEYWORD(Package *package) const;
		std::string VER_WASMINUSUNSTABLE(Package *package) const;
		std::string VER_WASMINUSASTERISK(Package *package) const;

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

		explicit PrintFormat(GetProperty get_callback = NULLPTR)
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
		void setupResources(EixRc *eixrc);

		void setupColors();

		void clear_virtual(ExtendedVersion::Overlay count);

		void set_as_virtual(const ExtendedVersion::Overlay overlay, bool on = true);

		bool is_virtual(const ExtendedVersion::Overlay overlay) const ATTRIBUTE_PURE;

		bool have_virtual(const Package *p, bool nonvirtual) const ATTRIBUTE_PURE;

		void set_overlay_translations(std::vector<ExtendedVersion::Overlay> *translations)
		{ overlay_translations = translations; }

		void set_overlay_used(std::vector<bool> *used, bool *some)
		{
			overlay_used = used;
			some_overlay_used = some;
		}

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

		bool parseFormat(const char *fmt, std::string *errtext)
		{ return m_parser.start(fmt, !no_color, false, errtext); }

		bool parseFormat(Node **rootnode, const char *fmt, std::string *errtext)
		{
			if(likely(m_parser.start(fmt, !no_color, false, errtext))) {
				*rootnode = m_parser.rootnode();
				return true;
			}
			return false;
		}

		void StabilityLocal(Package *p) const
		{ stability->set_stability(true, p); }

		void StabilityNonlocal(Package *p) const
		{ stability->set_stability(false, p); }

		static void init_static();
};

class LocalCopy : public PackageSave
{
	public:
		LocalCopy(const PrintFormat *fmt, Package *pkg);
};



#endif  // SRC_OUTPUT_FORMATSTRING_H_
