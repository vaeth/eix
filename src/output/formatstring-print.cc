// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include <config.h>

// #define EIX_PARANOIC_ASSERT

#include <cstdlib>
#include <cstring>

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "eixTk/ansicolor.h"
#include "eixTk/assert.h"
#include "eixTk/formated.h"
#include "eixTk/i18n.h"
#include "eixTk/likely.h"
#include "eixTk/null.h"
#include "eixTk/sysutils.h"
#include "eixTk/unused.h"
#include "eixrc/eixrc.h"
#include "output/formatstring-print.h"
#include "output/formatstring.h"
#include "portage/conf/portagesettings.h"
#include "portage/extendedversion.h"
#include "portage/instversion.h"
#include "portage/keywords.h"
#include "portage/package.h"
#include "portage/vardbpkg.h"
#include "portage/version.h"

using std::map;
using std::pair;
using std::set;
using std::string;
using std::vector;

using std::cerr;
using std::endl;

#define STRING_TRUE "1"
#define STRING_FALSE ""
#define STRING_EMPTY ""

inline static const char *IS_TRUE(bool a);
inline static const char *IS_FALSE(bool a);
static Package *old_or_new(string *new_name, Package *older, Package *newer, const string &name) ATTRIBUTE_NONNULL_;

inline static
const char *IS_TRUE(bool a)
{
	return (a ? STRING_TRUE : STRING_FALSE);
}

inline static
const char *IS_FALSE(bool a)
{
	return (a ? STRING_FALSE : STRING_TRUE);
}

class VersionVariables {
	private:
		const Version *m_version;
		InstVersion *m_instver;

	public:
		bool first, last, slotfirst, slotlast, oneslot, isinst;
		bool know_restrict;
		string result;

		VersionVariables()
		{
			m_version = NULLPTR;
			m_instver = NULLPTR;
			first = last = slotfirst = slotlast = oneslot = true;
			isinst = false;
		}

		void setinst(InstVersion *inst)
		{
			know_restrict = false;
			m_instver = inst;
		}

		void setversion(const Version *ver)
		{ m_version = ver; }

		const Version *version() const
		{ return m_version; }

		InstVersion *instver() const
		{ return m_instver; }
};

string
PrintFormat::iuse_expand(const IUseSet &iuse, bool coll) const
{
	string ret;
	map<string, string> expvars;
	const set<IUse> &iuse_set(iuse.asSet());
	for(set<IUse>::const_iterator it(iuse_set.begin());
		it != iuse_set.end(); ++it) {
		string var, expval;
		const string &name(it->name());
		if(portagesettings->use_expand(&var, &expval, name)) {
			string &r(expvars[var]);
			if(!r.empty()) {
				r.append(1, ' ');
			}
			const char *p(it->prefix());
			if(p != NULLPTR) {
				r.append(p);
			}
			r.append(expval);
		} else {
			if(!ret.empty()) {
				ret.append(1, ' ');
			}
			ret.append(it->asString());
		}
	}
	for(map<string, string>::const_iterator it(expvars.begin());
		it != expvars.end(); ++it) {
		if(!ret.empty()) {
			ret.append(1, ' ');
		}
		ret.append(coll ? before_coll_start : before_iuse_start);
		ret.append(it->first);
		ret.append(coll ? before_coll_end : before_iuse_end);
		ret.append(it->second);
		ret.append(coll ? after_coll : after_iuse);
	}
	return ret;
}

string
PrintFormat::get_inst_use(const Package &package, InstVersion *i, bool expand) const
{
	if((!(vardb->readUse(package, i))) || i->inst_iuse.empty()) {
		return STRING_EMPTY;
	}
	string ret, add, expval;
	map<string, pair<string, string> > expvars;
	for(vector<string>::iterator it(i->inst_iuse.begin());
		likely(it != i->inst_iuse.end()); ++it) {
		string *value(&(*it));
		bool is_unset(i->usedUse.find(*value) == i->usedUse.end());
		string *curr(&ret);
		bool unset_list(false);
		if(is_unset && !alpha_use) {
			unset_list = true;
			curr = &add;
		}
		if(expand) {
			string var;
			if(portagesettings->use_expand(&var, &expval, *value)) {
				value = &expval;
				pair<string, string> &r(expvars[var]);
				curr = (unset_list ? &(r.second) : &(r.first));
			}
		}
		if(!curr->empty())
			curr->append(1, ' ');
		if(is_unset)
			curr->append(before_unset_use);
		else
			curr->append(before_set_use);
		curr->append(*value);
		if(is_unset)
			curr->append(after_unset_use);
		else
			curr->append(after_set_use);
	}
	if(!add.empty()) {
		if(!ret.empty())
			ret.append(1, ' ');
		ret.append(add);
	}
	for(map<string, pair<string, string> >::const_iterator it(expvars.begin());
		it != expvars.end(); ++it) {
		if(!ret.empty()) {
			ret.append(1, ' ');
		}
		ret.append(before_use_start);
		ret.append(it->first);
		ret.append(before_use_end);
		const string &f(it->second.first);
		ret.append(f);
		const string &s(it->second.second);
		if(!s.empty()) {
			if(!f.empty()) {
				ret.append(1, ' ');
			}
			ret.append(s);
		}
		ret.append(after_use);
	}
	return ret;
}

void
PrintFormat::get_installed(Package *package, Node *root, bool only_marked) const
{
	eix_assert_paranoic(vardb != NULLPTR);
	if(unlikely((unlikely(only_marked)) && (marked_list == NULLPTR)))
		return;
	vector<InstVersion> *vec(vardb->getInstalledVector(*package));
	if(vec == NULLPTR)
		return;
	bool have_prevversion(false);
	for(vector<InstVersion>::iterator it(vec->begin());
		likely(it != vec->end()); ++it) {
		if(unlikely(only_marked)) {
			if(likely(!(marked_list->is_marked(*package, &(*it)))))
				continue;
		}
		if(have_prevversion) {
			version_variables->last = version_variables->slotlast = false;
			recPrint(&(version_variables->result), package, &get_package_property, root);
			version_variables->first = version_variables->slotfirst = false;
		}
		have_prevversion = true;
		version_variables->setinst(&(*it));
	}
	if(have_prevversion) {
		version_variables->last = version_variables->slotlast = true;
		recPrint(&(version_variables->result), package, &get_package_property, root);
	}
}

void
PrintFormat::get_versions_versorted(Package *package, Node *root, vector<Version*> *versions) const
{
	bool have_prevversion(false);
	for(Package::const_iterator vit(package->begin());
		likely(vit != package->end()); ++vit) {
		if(unlikely(versions != NULLPTR)) {
			if(likely(find(versions->begin(), versions->end(), *vit) == versions->end())) {
				continue;
			}
		}
		if(have_prevversion) {
			version_variables->last = version_variables->slotlast = false;
			recPrint(&(version_variables->result), package, &get_package_property, root);
			version_variables->first = version_variables->slotfirst = false;
		}
		have_prevversion = true;
		version_variables->setversion(*vit);
	}
	if(have_prevversion) {
		version_variables->last = version_variables->slotlast = true;
		recPrint(&(version_variables->result), package, &get_package_property, root);
	}
}

void
PrintFormat::get_versions_slotsorted(Package *package, Node *root, vector<Version*> *versions) const
{
	const SlotList *sl(&(package->slotlist()));
	SlotList::size_type slotnum(0);
	if(unlikely(versions != NULLPTR)) {
		for(SlotList::const_iterator it(sl->begin());
			likely(it != sl->end()); ++it) {
			const VersionList *vl(&(it->const_version_list()));
			for(VersionList::const_iterator vit(vl->begin());
				likely(vit != vl->end()); ++vit) {
				if(unlikely(find(versions->begin(), versions->end(), *vit) != versions->end())) {
					++slotnum;
					break;
				}
			}
		}
	} else {
		slotnum = sl->size();
	}
	if(unlikely(slotnum == 0))
		return;
	version_variables->oneslot = (slotnum == 1);

	bool have_prevversion(false);
	SlotList::size_type prevslot(slotnum + 1);
	version_variables->slotfirst = true;
	for(SlotList::const_iterator it(sl->begin()); likely(slotnum != 0); ++it, --slotnum) {
		const VersionList *vl(&(it->const_version_list()));
		for(VersionList::const_iterator vit(vl->begin());
			likely(vit != vl->end()); ++vit) {
			if(unlikely(versions != NULLPTR)) {
				if(likely(find(versions->begin(), versions->end(), *vit) == versions->end()))
					continue;
			}
			if(have_prevversion) {
				version_variables->last = false;
				version_variables->slotlast = (prevslot != slotnum);
				recPrint(&(version_variables->result), package, &get_package_property, root);
				version_variables->first = false;
			}
			have_prevversion = true;
			version_variables->setversion(*vit);
			version_variables->slotfirst = (prevslot != slotnum);
			prevslot = slotnum;
		}
	}
	if(have_prevversion) {
		version_variables->last = true;
		version_variables->slotlast = true;
		recPrint(&(version_variables->result), package, &get_package_property, root);
	}
}

void
PrintFormat::clear_virtual(ExtendedVersion::Overlay count)
{
	delete virtuals;
	virtuals = new vector<bool>(count, false);
}

void
PrintFormat::set_as_virtual(const ExtendedVersion::Overlay overlay, bool on)
{
	if(overlay == 0) {
		return;
	}
	(*virtuals)[overlay-1] = on;
}

bool
PrintFormat::is_virtual(const ExtendedVersion::Overlay overlay) const
{
	if(virtuals == NULLPTR) {
		return false;
	}
	if((overlay == 0) || (overlay >= virtuals->size())) {
		return false;
	}
	return (*virtuals)[overlay - 1];
}

bool
PrintFormat::have_virtual(const Package *p, bool nonvirtual) const
{
	for(Package::const_iterator vit(p->begin());
		likely(vit != p->end()); ++vit) {
		if(is_virtual(vit->overlay_key) ^ nonvirtual) {
			return true;
		}
	}
	return false;
}

class Scanner {
	public:
		enum Diff {
			DIFF_NONE,
			DIFF_BETTER,
			DIFF_BESTBETTER,
			DIFF_WORSE,
			DIFF_BESTWORSE,
			DIFF_DIFFER,
			DIFF_BESTDIFFER
		};
		enum Prop { PKG, VER };
		typedef string (PrintFormat::*Plain)(Package *pkg) const;
		typedef void (PrintFormat::*ColonVar)(Package *pkg, const string &after_colon) const;
		typedef string (PrintFormat::*ColonOther)(Package *pkg, const string &after_colon) const;

	protected:
		map<string, Diff> diff;
		map<string, pair<Plain, Prop> > plain;
		map<string, ColonVar> colon_var;
		map<string, ColonOther> colon_other;

		void prop_diff(const char *s, Diff diffprop) ATTRIBUTE_NONNULL_
		{ diff[s] = diffprop; }

		void prop_colon_pkg(const char *s, ColonVar colfunc) ATTRIBUTE_NONNULL_
		{ colon_var[s] = colfunc; }

		void prop_colon_ver(const char *s, ColonOther colfunc) ATTRIBUTE_NONNULL_
		{ colon_other[s] = colfunc; }

		void prop_pkg(const char *s, Plain plainfunc) ATTRIBUTE_NONNULL_
		{ plain[s] = pair<Plain, Prop>(plainfunc, PKG); }

		void prop_ver(const char *s, Plain plainfunc) ATTRIBUTE_NONNULL_
		{ plain[s] = pair<Plain, Prop>(plainfunc, VER); }

	public:
		Scanner()
		{
			prop_diff("better", DIFF_BETTER);
			prop_diff("bestbetter", DIFF_BESTBETTER);
			prop_diff("worse", DIFF_WORSE);
			prop_diff("bestworse", DIFF_BESTWORSE);
			prop_diff("differ", DIFF_DIFFER);
			prop_diff("bestdiffer", DIFF_BESTDIFFER);
			prop_colon_ver("date", &PrintFormat::COLON_VER_DATE);
			prop_colon_pkg("availableversions", &PrintFormat::COLON_PKG_AVAILABLEVERSIONS);
			prop_colon_pkg("markedversions", &PrintFormat::COLON_PKG_MARKEDVERSIONS);
			prop_colon_pkg("bestversion*", &PrintFormat::COLON_PKG_BESTVERSIONS);
			prop_colon_pkg("bestversion", &PrintFormat::COLON_PKG_BESTVERSION);
			prop_colon_pkg("bestslotversions*", &PrintFormat::COLON_PKG_BESTSLOTVERSIONSS);
			prop_colon_pkg("bestslotversions", &PrintFormat::COLON_PKG_BESTSLOTVERSIONS);
			prop_colon_pkg("bestslotupgradeversions*", &PrintFormat::COLON_PKG_BESTSLOTUPGRADEVERSIONSS);
			prop_colon_pkg("bestslotupgradeversions", &PrintFormat::COLON_PKG_BESTSLOTUPGRADEVERSIONS);
			prop_colon_pkg("installedversions", &PrintFormat::COLON_PKG_INSTALLEDVERSIONS);
			prop_colon_pkg("installedmarkedversions", &PrintFormat::COLON_PKG_INSTALLEDMARKEDVERSIONS);
			prop_pkg("installed", &PrintFormat::PKG_INSTALLED);
			prop_pkg("versionlines", &PrintFormat::PKG_VERSIONLINES);
			prop_pkg("slotsorted", &PrintFormat::PKG_SLOTSORTED);
			prop_pkg("color", &PrintFormat::PKG_COLOR);
			prop_pkg("havebest", &PrintFormat::PKG_HAVEBEST);
			prop_pkg("havebest*", &PrintFormat::PKG_HAVEBESTS);
			prop_pkg("category", &PrintFormat::PKG_CATEGORY);
			prop_pkg("name", &PrintFormat::PKG_NAME);
			prop_pkg("description", &PrintFormat::PKG_DESCRIPTION);
			prop_pkg("homepage", &PrintFormat::PKG_HOMEPAGE);
			prop_pkg("licenses", &PrintFormat::PKG_LICENSES);
			prop_pkg("overlaykey", &PrintFormat::PKG_OVERLAYKEY);
			prop_pkg("binary", &PrintFormat::PKG_BINARY);
			prop_pkg("system", &PrintFormat::PKG_SYSTEM);
			prop_pkg("world", &PrintFormat::PKG_WORLD);
			prop_pkg("world_sets", &PrintFormat::PKG_WORLD_SETS);
			prop_pkg("setnames", &PrintFormat::PKG_SETNAMES);
			prop_pkg("allsetnames", &PrintFormat::PKG_ALLSETNAMES);
			prop_pkg("upgrade", &PrintFormat::PKG_UPGRADE);
			prop_pkg("upgradeorinstall", &PrintFormat::PKG_UPGRADEORINSTALL);
			prop_pkg("bestupgrade", &PrintFormat::PKG_BESTUPGRADE);
			prop_pkg("bestupgradeorinstall", &PrintFormat::PKG_BESTUPGRADEORINSTALL);
			prop_pkg("downgrade", &PrintFormat::PKG_DOWNGRADE);
			prop_pkg("bestdowngrade", &PrintFormat::PKG_BESTDOWNGRADE);
			prop_pkg("recommend", &PrintFormat::PKG_RECOMMEND);
			prop_pkg("recommendorinstall", &PrintFormat::PKG_RECOMMENDORINSTALL);
			prop_pkg("bestrecommend", &PrintFormat::PKG_BESTRECOMMEND);
			prop_pkg("bestrecommendorinstall", &PrintFormat::PKG_BESTRECOMMENDORINSTALL);
			prop_pkg("marked", &PrintFormat::PKG_MARKED);
			prop_pkg("havemarkedversion", &PrintFormat::PKG_HAVEMARKEDVERSION);
			prop_pkg("slots", &PrintFormat::PKG_SLOTS);
			prop_pkg("slotted", &PrintFormat::PKG_SLOTTED);
			prop_pkg("havevirtual", &PrintFormat::PKG_HAVEVIRTUAL);
			prop_pkg("havenonvirtual", &PrintFormat::PKG_HAVENONVIRTUAL);
			prop_pkg("havecolliuse", &PrintFormat::PKG_HAVECOLLIUSE);
			prop_pkg("colliuse*", &PrintFormat::PKG_COLLIUSES);
			prop_pkg("colliuse", &PrintFormat::PKG_COLLIUSE);
			prop_ver("first", &PrintFormat::VER_FIRST);
			prop_ver("last", &PrintFormat::VER_LAST);
			prop_ver("slotfirst", &PrintFormat::VER_SLOTFIRST);
			prop_ver("slotlast", &PrintFormat::VER_SLOTLAST);
			prop_ver("oneslot", &PrintFormat::VER_ONESLOT);
			prop_ver("fullslot", &PrintFormat::VER_FULLSLOT);
			prop_ver("isfullslot", &PrintFormat::VER_ISFULLSLOT);
			prop_ver("slot", &PrintFormat::VER_SLOT);
			prop_ver("isslot", &PrintFormat::VER_ISSLOT);
			prop_ver("subslot", &PrintFormat::VER_SUBSLOT);
			prop_ver("issubslot", &PrintFormat::VER_ISSUBSLOT);
			prop_ver("version", &PrintFormat::VER_VERSION);
			prop_ver("plainversion", &PrintFormat::VER_PLAINVERSION);
			prop_ver("revision", &PrintFormat::VER_REVISION);
			prop_ver("overlaynum", &PrintFormat::VER_OVERLAYNUM);
			prop_ver("overlayver", &PrintFormat::VER_OVERLAYVER);
			prop_ver("versionkeywords*", &PrintFormat::VER_VERSIONKEYWORDSS);
			prop_ver("versionkeywords", &PrintFormat::VER_VERSIONKEYWORDS);
			prop_ver("versionekeywords", &PrintFormat::VER_VERSIONEKEYWORDS);
			prop_ver("isbestupgradeslot*", &PrintFormat::VER_ISBESTUPGRADESLOTS);
			prop_ver("isbestupgradeslot", &PrintFormat::VER_ISBESTUPGRADESLOT);
			prop_ver("isbestupgrade*", &PrintFormat::VER_ISBESTUPGRADES);
			prop_ver("isbestupgrade", &PrintFormat::VER_ISBESTUPGRADE);
			prop_ver("markedversion", &PrintFormat::VER_MARKEDVERSION);
			prop_ver("installedversion", &PrintFormat::VER_INSTALLEDVERSION);
			prop_ver("haveuse", &PrintFormat::VER_HAVEUSE);
			prop_ver("use*", &PrintFormat::VER_USES);
			prop_ver("use", &PrintFormat::VER_USE);
			prop_ver("virtual", &PrintFormat::VER_VIRTUAL);
			prop_ver("isbinary", &PrintFormat::VER_ISBINARY);
			prop_ver("restrict", &PrintFormat::VER_RESTRICT);
			prop_ver("restrictfetch", &PrintFormat::VER_RESTRICTFETCH);
			prop_ver("restrictmirror", &PrintFormat::VER_RESTRICTMIRROR);
			prop_ver("restrictprimaryuri", &PrintFormat::VER_RESTRICTPRIMARYURI);
			prop_ver("restrictbinchecks", &PrintFormat::VER_RESTRICTBINCHECKS);
			prop_ver("restrictstrip", &PrintFormat::VER_RESTRICTSTRIP);
			prop_ver("restricttest", &PrintFormat::VER_RESTRICTTEST);
			prop_ver("restrictuserpriv", &PrintFormat::VER_RESTRICTUSERPRIV);
			prop_ver("restrictinstallsources", &PrintFormat::VER_RESTRICTINSTALLSOURCES);
			prop_ver("restrictbindist", &PrintFormat::VER_RESTRICTBINDIST);
			prop_ver("restrictparallel", &PrintFormat::VER_RESTRICTPARALLEL);
			prop_ver("properties", &PrintFormat::VER_PROPERTIES);
			prop_ver("propertiesinteractive", &PrintFormat::VER_PROPERTIESINTERACTIVE);
			prop_ver("propertieslive", &PrintFormat::VER_PROPERTIESLIVE);
			prop_ver("propertiesvirtual", &PrintFormat::VER_PROPERTIESVIRTUAL);
			prop_ver("propertiesset", &PrintFormat::VER_PROPERTIESSET);
			prop_ver("havedepend", &PrintFormat::VER_HAVEDEPEND);
			prop_ver("haverdepend", &PrintFormat::VER_HAVERDEPEND);
			prop_ver("havepdepend", &PrintFormat::VER_HAVEPDEPEND);
			prop_ver("havehdepend", &PrintFormat::VER_HAVEHDEPEND);
			prop_ver("havedeps", &PrintFormat::VER_HAVEDEPS);
			prop_ver("depend*", &PrintFormat::VER_DEPENDS);
			prop_ver("depend", &PrintFormat::VER_DEPEND);
			prop_ver("rdepend*", &PrintFormat::VER_RDEPENDS);
			prop_ver("rdepend", &PrintFormat::VER_RDEPEND);
			prop_ver("pdepend*", &PrintFormat::VER_PDEPENDS);
			prop_ver("pdepend", &PrintFormat::VER_PDEPEND);
			prop_ver("hdepend*", &PrintFormat::VER_HDEPENDS);
			prop_ver("hdepend", &PrintFormat::VER_HDEPEND);
			prop_ver("ishardmasked", &PrintFormat::VER_ISHARDMASKED);
			prop_ver("isprofilemasked", &PrintFormat::VER_ISPROFILEMASKED);
			prop_ver("ismasked", &PrintFormat::VER_ISMASKED);
			prop_ver("isstable", &PrintFormat::VER_ISSTABLE);
			prop_ver("isunstable", &PrintFormat::VER_ISUNSTABLE);
			prop_ver("isalienstable", &PrintFormat::VER_ISALIENSTABLE);
			prop_ver("isalienunstable", &PrintFormat::VER_ISALIENUNSTABLE);
			prop_ver("ismissingkeyword", &PrintFormat::VER_ISMISSINGKEYWORD);
			prop_ver("isminuskeyword", &PrintFormat::VER_ISMINUSKEYWORD);
			prop_ver("isminusunstable", &PrintFormat::VER_ISMINUSUNSTABLE);
			prop_ver("isminusasterisk", &PrintFormat::VER_ISMINUSASTERISK);
			prop_ver("washardmasked", &PrintFormat::VER_WASHARDMASKED);
			prop_ver("wasprofilemasked", &PrintFormat::VER_WASPROFILEMASKED);
			prop_ver("wasmasked", &PrintFormat::VER_WASMASKED);
			prop_ver("wasstable", &PrintFormat::VER_WASSTABLE);
			prop_ver("wasunstable", &PrintFormat::VER_WASUNSTABLE);
			prop_ver("wasalienstable", &PrintFormat::VER_WASALIENSTABLE);
			prop_ver("wasalienunstable", &PrintFormat::VER_WASALIENUNSTABLE);
			prop_ver("wasmissingkeyword", &PrintFormat::VER_WASMISSINGKEYWORD);
			prop_ver("wasminuskeyword", &PrintFormat::VER_WASMINUSKEYWORD);
			prop_ver("wasminusunstable", &PrintFormat::VER_WASMINUSUNSTABLE);
			prop_ver("wasminusasterisK", &PrintFormat::VER_WASMINUSASTERISK);
			prop_ver("havemaskreasons", &PrintFormat::VER_HAVEMASKREASONS);
			prop_ver("maskreasons", &PrintFormat::VER_MASKREASONS);
			prop_ver("maskreasons*", &PrintFormat::VER_MASKREASONSS);
		}

		Diff get_diff(const string& s) const ATTRIBUTE_PURE
		{
			map<string, Diff>::const_iterator it(diff.find(s));
			return ((it == diff.end()) ? DIFF_NONE : it->second);
		}

		ColonVar get_colon_var(const string& s, Prop *p) const ATTRIBUTE_NONNULL_
		{
			map<string, ColonVar>::const_iterator it(colon_var.find(s));
			if(it == colon_var.end()) {
				return NULLPTR;
			}
			*p = PKG;
			return it->second;
		}

		ColonOther get_colon_other(const string& s, Prop *p) const ATTRIBUTE_NONNULL_
		{
			map<string, ColonOther>::const_iterator it(colon_other.find(s));
			if(it == colon_other.end()) {
				return NULLPTR;
			}
			*p = VER;
			return it->second;
		}

		Plain get_plain(const string& s, Prop *p) const ATTRIBUTE_NONNULL_
		{
			map<string, pair<Plain, Prop> >::const_iterator it(plain.find(s));
			if(it == plain.end()) {
				return NULLPTR;
			}
			*p = it->second.second;
			return it->second.first;
		}
};

static Scanner *scanner = NULLPTR;

void
PrintFormat::init_static()
{
	eix_assert_static(scanner == NULLPTR);
	scanner = new Scanner;
	AnsiColor::init_static();
}

string
PrintFormat::get_pkg_property(Package *package, const string &name) const
{
	eix_assert_static(scanner != NULLPTR);
	Scanner::Prop t;
	Scanner::Plain plain(scanner->get_plain(name, &t));
	Scanner::ColonVar colon_var;
	Scanner::ColonOther colon_other;
	string after_colon;
	if(plain == NULLPTR) {
		string::size_type col(name.find(':'));
		if(likely(col != string::npos)) {
			// we misuse here "after_colon" to mean "before_colon"
			after_colon.assign(name.substr(0, col));
			colon_var = scanner->get_colon_var(after_colon, &t);
			if(unlikely(colon_var == NULLPTR)) {
				colon_other = scanner->get_colon_other(after_colon, &t);
				if(unlikely(colon_other == NULLPTR)) {
					// flag that we failed
					col = string::npos;
				}
			}
		}
		if(unlikely(col == string::npos)) {
			cerr << eix::format(_("Unknown property %r")) % name << endl;
			exit(EXIT_FAILURE);
		}
		after_colon.assign(name, col + 1, string::npos);
	}
	if(unlikely((t == Scanner::VER) && (version_variables == NULLPTR))) {
		cerr << eix::format(_("Property %r used outside version context")) % name << endl;
		exit(EXIT_FAILURE);
	}
	if(plain != NULLPTR) {
		return (this->*plain)(package);
	}
	if(colon_var == NULLPTR) {
		return (this->*colon_other)(package, after_colon);
	}
	// colon_var:
	// It is important that version_variables points to a local object:
	// This allows loops within loops.
	// Recursion is avoided by checking the variable names.
	VersionVariables variables;
	VersionVariables *previous_variables(version_variables);
	version_variables = &variables;
	(this->*colon_var)(package, after_colon);
	version_variables = previous_variables;
	return variables.result;
}

string
PrintFormat::COLON_VER_DATE(Package *package, const string &after_colon) const
{
	if(version_variables->isinst) {
		InstVersion *i(version_variables->instver());
		vardb->readInstDate(*package, i);
		return date_conv((*eix_rc)[after_colon].c_str(), i->instDate);
	}
	return STRING_EMPTY;
}

void
PrintFormat::colon_pkg_availableversions(Package *package, const string &after_colon, bool only_marked) const
{
	vector<Version*> *versions(NULLPTR);
	if(unlikely(only_marked)) {
		versions = new vector<Version*>;
		if(likely(marked_list != NULLPTR)) {
			for(Package::const_iterator it(package->begin());
				likely(it != package->end()); ++it) {
				if(unlikely(marked_list->is_marked(*package, &(**it)))) {
					versions->push_back(*it);
				}
			}
		}
	}
	if(likely((!only_marked) || !(versions->empty()))) {
		string::size_type col(after_colon.find(':'));
		if(col == string::npos) {
			get_versions_versorted(package, parse_variable(after_colon), versions);
			varcache[after_colon].in_use = false;
		} else if(!(package->have_nontrivial_slots())) {
			string var(after_colon, 0, col);
			get_versions_versorted(package, parse_variable(var), versions);
			varcache[var].in_use = false;
		} else {
			string var(after_colon, col + 1, string::npos);
			get_versions_slotsorted(package, parse_variable(var), versions);
			varcache[var].in_use = false;
		}
	}
	delete versions;
}

void
PrintFormat::COLON_PKG_AVAILABLEVERSIONS(Package *package, const string &after_colon) const
{
	colon_pkg_availableversions(package, after_colon, false);
}

void
PrintFormat::COLON_PKG_MARKEDVERSIONS(Package *package, const string &after_colon) const
{
	colon_pkg_availableversions(package, after_colon, true);
}

void
PrintFormat::colon_pkg_bestversion(Package *package, const string &after_colon, bool allow_unstable) const
{
	const Version *ver(package->best(allow_unstable));
	version_variables->setversion(ver);
	if(likely(ver != NULLPTR)) {
		recPrint(&(version_variables->result), package, get_package_property, parse_variable(after_colon));
		varcache[after_colon].in_use = false;
	}
}

void
PrintFormat::COLON_PKG_BESTVERSION(Package *package, const string &after_colon) const
{
	colon_pkg_bestversion(package, after_colon, false);
}

void
PrintFormat::COLON_PKG_BESTVERSIONS(Package *package, const string &after_colon) const
{
	colon_pkg_bestversion(package, after_colon, true);
}

void
PrintFormat::colon_pkg_bestslotversions(Package *package, const string &after_colon, bool allow_unstable) const
{
	const Version *ver(package->best(allow_unstable));
	version_variables->setversion(ver);
	vector<Version*> versions;
	package->best_slots(&versions, allow_unstable);
	if(!versions.empty()) {
		get_versions_versorted(package, parse_variable(after_colon), &versions);
		varcache[after_colon].in_use = false;
	}
}

void
PrintFormat::COLON_PKG_BESTSLOTVERSIONS(Package *package, const string &after_colon) const
{
	colon_pkg_bestslotversions(package, after_colon, false);
}

void
PrintFormat::COLON_PKG_BESTSLOTVERSIONSS(Package *package, const string &after_colon) const
{
	colon_pkg_bestslotversions(package, after_colon, true);
}

void
PrintFormat::colon_pkg_bestslotupgradeversions(Package *package, const string &after_colon, bool allow_unstable) const
{
	vector<Version*> versions;
	package->best_slots_upgrade(&versions, vardb, portagesettings, allow_unstable);
	if(!versions.empty()) {
		get_versions_versorted(package, parse_variable(after_colon), &versions);
		varcache[after_colon].in_use = false;
	}
}

void
PrintFormat::COLON_PKG_BESTSLOTUPGRADEVERSIONS(Package *package, const string &after_colon) const
{
	colon_pkg_bestslotupgradeversions(package, after_colon, false);
}

void
PrintFormat::COLON_PKG_BESTSLOTUPGRADEVERSIONSS(Package *package, const string &after_colon) const
{
	colon_pkg_bestslotupgradeversions(package, after_colon, true);
}

void
PrintFormat::colon_pkg_installedversions(Package *package, const string &after_colon, bool only_marked) const
{
	version_variables->isinst = true;
	get_installed(package, parse_variable(after_colon), only_marked);
	varcache[after_colon].in_use = false;
}

void
PrintFormat::COLON_PKG_INSTALLEDVERSIONS(Package *package, const string &after_colon) const
{
	colon_pkg_installedversions(package, after_colon, false);
}


void
PrintFormat::COLON_PKG_INSTALLEDMARKEDVERSIONS(Package *package, const string &after_colon) const
{
	colon_pkg_installedversions(package, after_colon, true);
}

string
PrintFormat::PKG_INSTALLED(Package *package) const
{
	eix_assert_paranoic(vardb != NULLPTR);
	vector<InstVersion> *vec(vardb->getInstalledVector(*package));
	return IS_TRUE((vec != NULLPTR) && (likely(!(vec->empty()))));
}

string
PrintFormat::PKG_VERSIONLINES(Package *package ATTRIBUTE_UNUSED) const
{
	UNUSED(package);
	return IS_TRUE(style_version_lines);
}

string
PrintFormat::PKG_SLOTSORTED(Package *package ATTRIBUTE_UNUSED) const
{
	UNUSED(package);
	return IS_TRUE(slot_sorted);
}

string
PrintFormat::PKG_COLOR(Package *package ATTRIBUTE_UNUSED) const
{
	UNUSED(package);
	return IS_FALSE(no_color);
}

string
PrintFormat::PKG_HAVEBEST(Package *package) const
{
	return IS_TRUE(package->best(false) != NULLPTR);
}

string
PrintFormat::PKG_HAVEBESTS(Package *package) const
{
	return IS_TRUE(package->best(true) != NULLPTR);
}

string
PrintFormat::PKG_CATEGORY(Package *package) const
{
	return package->category;
}

string
PrintFormat::PKG_NAME(Package *package) const
{
	return package->name;
}

string
PrintFormat::PKG_DESCRIPTION(Package *package) const
{
	return package->desc;
}

string
PrintFormat::PKG_HOMEPAGE(Package *package) const
{
	return package->homepage;
}

string
PrintFormat::PKG_LICENSES(Package *package) const
{
	return package->licenses;
}

string
PrintFormat::PKG_BINARY(Package *package) const
{
	for(Package::const_iterator it(package->begin()); likely(it != package->end()); ++it) {
		if(it->have_bin_pkg(portagesettings, package)) {
			return STRING_TRUE;
		}
	}
	eix_assert_paranoic(vardb != NULLPTR);
	vector<InstVersion> *vec(vardb->getInstalledVector(*package));
	if(vec != NULLPTR) {
		for(vector<InstVersion>::iterator it(vec->begin());
			likely(it != vec->end()); ++it) {
			if(it->have_bin_pkg(portagesettings, package)) {
				return STRING_TRUE;
			}
		}
	}
	return STRING_FALSE;
}

string
PrintFormat::PKG_OVERLAYKEY(Package *package) const
{
	ExtendedVersion::Overlay ov_key(package->largest_overlay);
	if(ov_key && package->have_same_overlay_key()) {
		return overlay_keytext(ov_key, false);
	}
	return STRING_EMPTY;
}

string
PrintFormat::PKG_SYSTEM(Package *package) const
{
	return IS_TRUE(package->is_system_package());
}

string
PrintFormat::PKG_WORLD(Package *package) const
{
	return IS_TRUE(package->is_world_package());
}

string
PrintFormat::PKG_WORLD_SETS(Package *package) const
{
	return IS_TRUE(package->is_world_sets_package());
}

string
PrintFormat::PKG_SETNAMES(Package *package) const
{
	return portagesettings->get_setnames(package);
}

string
PrintFormat::PKG_ALLSETNAMES(Package *package) const
{
	return portagesettings->get_setnames(package, true);
}

string
PrintFormat::pkg_upgrade(Package *package, bool only_installed, bool test_slots) const
{
	LocalCopy localcopy(this, package);
	bool result(package->can_upgrade(vardb, portagesettings, only_installed, test_slots));
	localcopy.restore(package);
	return IS_TRUE(result);
}

string
PrintFormat::PKG_UPGRADE(Package *package) const
{
	return pkg_upgrade(package, true, true);
}

string
PrintFormat::PKG_UPGRADEORINSTALL(Package *package) const
{
	return pkg_upgrade(package, false, true);
}

string
PrintFormat::PKG_BESTUPGRADE(Package *package) const
{
	return pkg_upgrade(package, true, false);
}

string
PrintFormat::PKG_BESTUPGRADEORINSTALL(Package *package) const
{
	return pkg_upgrade(package, false, false);
}

string
PrintFormat::pkg_downgrade(Package *package, bool test_slots) const
{
	LocalCopy locallocalcopy(this, package);
	bool result(package->must_downgrade(vardb, test_slots));
	locallocalcopy.restore(package);
	return IS_TRUE(result);
}

string
PrintFormat::PKG_DOWNGRADE(Package *package) const
{
	return pkg_downgrade(package, true);
}

string
PrintFormat::PKG_BESTDOWNGRADE(Package *package) const
{
	return pkg_downgrade(package, false);
}

string
PrintFormat::pkg_recommend(Package *package, bool only_installed, bool test_slots) const
{
	LocalCopy locallocalcopy(this, package);
	bool result(package->recommend(vardb, portagesettings, only_installed, test_slots));
	locallocalcopy.restore(package);
	return IS_TRUE(result);
}

string
PrintFormat::PKG_RECOMMEND(Package *package) const
{
	return pkg_recommend(package, true, true);
}

string
PrintFormat::PKG_RECOMMENDORINSTALL(Package *package) const
{
	return pkg_recommend(package, false, true);
}

string
PrintFormat::PKG_BESTRECOMMEND(Package *package) const
{
	return pkg_recommend(package, true, false);
}

string
PrintFormat::PKG_BESTRECOMMENDORINSTALL(Package *package) const
{
	return pkg_recommend(package, false, false);
}

string
PrintFormat::PKG_MARKED(Package *package) const
{
	if(likely(marked_list != NULLPTR)) {
		if(unlikely(marked_list->is_marked(*package))) {
			return STRING_TRUE;
		}
	}
	return STRING_FALSE;
}

string
PrintFormat::PKG_HAVEMARKEDVERSION(Package *package) const
{
	if(likely(marked_list != NULLPTR)) {
		for(Package::const_iterator it(package->begin());
			likely(it != package->end()); ++it) {
			if(marked_list->is_marked(*package, &(**it))) {
				return STRING_TRUE;
			}
		}
	}
	return STRING_FALSE;
}

string
PrintFormat::PKG_SLOTS(Package *package) const
{
	return IS_TRUE((package->slotlist()).size() > 1);
}

string
PrintFormat::PKG_SLOTTED(Package *package) const
{
	return IS_TRUE(package->have_nontrivial_slots());
}

string
PrintFormat::PKG_HAVEVIRTUAL(Package *package) const
{
	return IS_TRUE(have_virtual(package, false));
}

string
PrintFormat::PKG_HAVENONVIRTUAL(Package *package) const
{
	return IS_TRUE(have_virtual(package, true));
}

string
PrintFormat::PKG_HAVECOLLIUSE(Package *package) const
{
	return IS_FALSE(package->iuse.empty());
}

string
PrintFormat::PKG_COLLIUSES(Package *package) const
{
	return iuse_expand(package->iuse, true);
}

string
PrintFormat::PKG_COLLIUSE(Package *package) const
{
	return package->iuse.asString();
}

const ExtendedVersion *
PrintFormat::ver_version() const
{
	if(version_variables->isinst) {
		return version_variables->instver();
	}
	return version_variables->version();
}

string
PrintFormat::VER_FIRST(Package *package ATTRIBUTE_UNUSED) const
{
	UNUSED(package);
	return IS_TRUE(version_variables->first);
}

string
PrintFormat::VER_LAST(Package *package ATTRIBUTE_UNUSED) const
{
	UNUSED(package);
	return IS_TRUE(version_variables->last);
}

string
PrintFormat::VER_SLOTFIRST(Package *package ATTRIBUTE_UNUSED) const
{
	UNUSED(package);
	return IS_TRUE(version_variables->slotfirst);
}

string
PrintFormat::VER_SLOTLAST(Package *package ATTRIBUTE_UNUSED) const
{
	UNUSED(package);
	return IS_TRUE(version_variables->slotlast);
}

string
PrintFormat::VER_ONESLOT(Package *package ATTRIBUTE_UNUSED) const
{
	UNUSED(package);
	return IS_TRUE(version_variables->oneslot);
}

const ExtendedVersion *
PrintFormat::ver_versionslot(Package *package) const
{
	if(version_variables->isinst) {
		InstVersion *i(version_variables->instver());
		package->guess_slotname(i, vardb, "?");
		return i;
	} else {
		return version_variables->version();
	}
}

string
PrintFormat::VER_FULLSLOT(Package *package) const
{
	return ver_versionslot(package)->get_longfullslot();
}

string
PrintFormat::VER_ISFULLSLOT(Package *package) const
{
	return IS_FALSE(ver_versionslot(package)->get_shortfullslot().empty());
}

string
PrintFormat::VER_SLOT(Package *package) const
{
	const string &slot(ver_versionslot(package)->slotname);
	return ((slot.empty()) ? "0" : slot);
}

string
PrintFormat::VER_ISSLOT(Package *package) const
{
	const string &slot(ver_versionslot(package)->slotname);
	return IS_FALSE(slot.empty() || (slot == "0"));
}

string
PrintFormat::VER_SUBSLOT(Package *package) const
{
	return ver_versionslot(package)->subslotname;
}

string
PrintFormat::VER_ISSUBSLOT(Package *package) const
{
	return IS_FALSE(ver_versionslot(package)->subslotname.empty());
}

string
PrintFormat::VER_VERSION(Package *package ATTRIBUTE_UNUSED) const
{
	UNUSED(package);
	if(version_variables->isinst) {
		return version_variables->instver()->getFull();
	}
	return version_variables->version()->getFull();
}

string
PrintFormat::VER_PLAINVERSION(Package *package ATTRIBUTE_UNUSED) const
{
	UNUSED(package);
	if(version_variables->isinst) {
		return version_variables->instver()->getPlain();
	}
	return version_variables->version()->getPlain();
}

string
PrintFormat::VER_REVISION(Package *package ATTRIBUTE_UNUSED) const
{
	UNUSED(package);
	if(version_variables->isinst) {
		return version_variables->instver()->getRevision();
	}
	return version_variables->version()->getRevision();
}

string
PrintFormat::ver_overlay(Package *package, bool getnum) const
{
	if(version_variables->isinst) {
		InstVersion *i(version_variables->instver());
		eix_assert_paranoic(header != NULLPTR);
		if(unlikely(!(vardb->readOverlay(*package, i, *header)))) {
			if(getnum || no_color) {
				return "[?]";
			}
			return color_overlaykey + "[?]" + color_keyend;
		}
		if(i->overlay_key > 0) {
			if(getnum || (!package->have_same_overlay_key()) || (package->largest_overlay != i->overlay_key)) {
				return overlay_keytext(i->overlay_key, getnum);
			}
		}
	} else if(getnum || (!package->have_same_overlay_key())) {
		if(version_variables->version()->overlay_key) {
			return overlay_keytext(version_variables->version()->overlay_key, getnum);
		}
	}
	return STRING_EMPTY;
}

string
PrintFormat::VER_OVERLAYNUM(Package *package) const
{
	return ver_overlay(package, true);
}

string
PrintFormat::VER_OVERLAYVER(Package *package) const
{
	return ver_overlay(package, false);
}

string
PrintFormat::VER_VERSIONKEYWORDSS(Package *package) const
{
	if(unlikely(version_variables->isinst)) {
		return STRING_EMPTY;
	}
	portagesettings->get_effective_keywords_userprofile(package);
	return version_variables->version()->get_effective_keywords();
}

string
PrintFormat::VER_VERSIONKEYWORDS(Package *package ATTRIBUTE_UNUSED) const
{
	UNUSED(package);
	if(unlikely(version_variables->isinst)) {
		return STRING_EMPTY;
	}
	return version_variables->version()->get_full_keywords();
}

string
PrintFormat::VER_VERSIONEKEYWORDS(Package *package) const
{
	if(likely(!version_variables->isinst)) {
		portagesettings->get_effective_keywords_userprofile(package);
		const Version *v(version_variables->version());
		string s(v->get_effective_keywords());
		if(s != v->get_full_keywords()) {
			return s;
		}
	}
	return STRING_EMPTY;
}

string
PrintFormat::ver_isbestupgrade(Package *package, bool check_slots, bool allow_unstable) const
{
	return IS_TRUE(likely(!version_variables->isinst) &&
		unlikely(package->is_best_upgrade(check_slots,
				version_variables->version(),
				vardb, portagesettings, allow_unstable)));
}

string
PrintFormat::VER_ISBESTUPGRADESLOT(Package *package) const
{
	return ver_isbestupgrade(package, true, false);
}

string
PrintFormat::VER_ISBESTUPGRADESLOTS(Package *package) const
{
	return ver_isbestupgrade(package, true, true);
}

string
PrintFormat::VER_ISBESTUPGRADE(Package *package) const
{
	return ver_isbestupgrade(package, false, false);
}

string
PrintFormat::VER_ISBESTUPGRADES(Package *package) const
{
	return ver_isbestupgrade(package, false, true);
}

string
PrintFormat::VER_MARKEDVERSION(Package *package) const
{
	if(likely(!version_variables->isinst)) {
		if(unlikely((likely(marked_list != NULLPTR)) && (unlikely(marked_list->is_marked(*package,
			version_variables->version()))))) {
			return STRING_TRUE;
		}
	}
	return STRING_FALSE;
}

string
PrintFormat::VER_INSTALLEDVERSION(Package *package) const
{
	if(unlikely(version_variables->isinst)) {
		return STRING_TRUE;
	}
	eix_assert_paranoic(header != NULLPTR);
	return IS_TRUE(vardb->isInstalledVersion(*package, version_variables->version(), *header));
}

string
PrintFormat::VER_HAVEUSE(Package *package) const
{
	if(version_variables->isinst) {
		InstVersion *i(version_variables->instver());
		return IS_TRUE(likely(vardb->readUse(*package, i))
			&& !(i->inst_iuse.empty()));
	}
	return IS_FALSE(version_variables->version()->iuse.empty());
}

string
PrintFormat::VER_USE(Package *package) const
{
	if(version_variables->isinst) {
		return get_inst_use(*package, version_variables->instver(), false);
	}
	return version_variables->version()->iuse.asString();
}

string
PrintFormat::VER_USES(Package *package) const
{
	if(version_variables->isinst) {
		return get_inst_use(*package, version_variables->instver(), true);
	}
	return iuse_expand(version_variables->version()->iuse, false);
}

string
PrintFormat::VER_VIRTUAL(Package *package) const
{
	ExtendedVersion::Overlay key;
	if(version_variables->isinst) {
		InstVersion *i(version_variables->instver());
		eix_assert_paranoic(header != NULLPTR);
		if(!(vardb->readOverlay(*package, i, *header))) {
			return STRING_FALSE;
		}
		key = i->overlay_key;
	} else {
		key = version_variables->version()->overlay_key;
	}
	return IS_TRUE(is_virtual(key));
}

string
PrintFormat::VER_ISBINARY(Package *package) const
{
	return IS_TRUE(ver_version()->have_bin_pkg(portagesettings, package));
}

const ExtendedVersion *
PrintFormat::ver_restrict(Package *package) const
{
	if(version_variables->isinst) {
		InstVersion *i(version_variables->instver());
		if(!version_variables->know_restrict) {
			eix_assert_paranoic(header != NULLPTR);
			if(vardb->readRestricted(*package, i, *header)) {
					version_variables->know_restrict = true;
			} else {
				return NULLPTR;
			}
		}
		return i;
	}
	return version_variables->version();
}

string
PrintFormat::ver_restrict(Package *package, ExtendedVersion::Restrict r) const
{
	const ExtendedVersion *e(ver_restrict(package));
	return IS_TRUE((e != NULLPTR) && (((e->restrictFlags) & r) != ExtendedVersion::RESTRICT_NONE));
}

string
PrintFormat::VER_RESTRICT(Package *package) const
{
	return ver_restrict(package, ExtendedVersion::RESTRICT_ALL);
}

string
PrintFormat::VER_RESTRICTFETCH(Package *package) const
{
	return ver_restrict(package, ExtendedVersion::RESTRICT_FETCH);
}

string
PrintFormat::VER_RESTRICTMIRROR(Package *package) const
{
	return ver_restrict(package, ExtendedVersion::RESTRICT_MIRROR);
}

string
PrintFormat::VER_RESTRICTPRIMARYURI(Package *package) const
{
	return ver_restrict(package, ExtendedVersion::RESTRICT_PRIMARYURI);
}

string
PrintFormat::VER_RESTRICTBINCHECKS(Package *package) const
{
	return ver_restrict(package, ExtendedVersion::RESTRICT_BINCHECKS);
}
string
PrintFormat::VER_RESTRICTSTRIP(Package *package) const
{
	return ver_restrict(package, ExtendedVersion::RESTRICT_STRIP);
}

string
PrintFormat::VER_RESTRICTTEST(Package *package) const
{
	return ver_restrict(package, ExtendedVersion::RESTRICT_TEST);
}

string
PrintFormat::VER_RESTRICTUSERPRIV(Package *package) const
{
	return ver_restrict(package, ExtendedVersion::RESTRICT_USERPRIV);
}

string
PrintFormat::VER_RESTRICTINSTALLSOURCES(Package *package) const
{
	return ver_restrict(package, ExtendedVersion::RESTRICT_INSTALLSOURCES);
}

string
PrintFormat::VER_RESTRICTBINDIST(Package *package) const
{
	return ver_restrict(package, ExtendedVersion::RESTRICT_BINDIST);
}

string
PrintFormat::VER_RESTRICTPARALLEL(Package *package) const
{
	return ver_restrict(package, ExtendedVersion::RESTRICT_PARALLEL);
}

string
PrintFormat::ver_properties(Package *package, ExtendedVersion::Properties p) const
{
	const ExtendedVersion *e(ver_restrict(package));
	return IS_TRUE((e != NULLPTR) && (((e->propertiesFlags) & p) != ExtendedVersion::RESTRICT_NONE));
}

string
PrintFormat::VER_PROPERTIES(Package *package) const
{
	return ver_properties(package, ExtendedVersion::PROPERTIES_ALL);
}

string
PrintFormat::VER_PROPERTIESINTERACTIVE(Package *package) const
{
	return ver_properties(package, ExtendedVersion::PROPERTIES_INTERACTIVE);
}

string
PrintFormat::VER_PROPERTIESLIVE(Package *package) const
{
	return ver_properties(package, ExtendedVersion::PROPERTIES_LIVE);
}

string
PrintFormat::VER_PROPERTIESVIRTUAL(Package *package) const
{
	return ver_properties(package, ExtendedVersion::PROPERTIES_VIRTUAL);
}

string
PrintFormat::VER_PROPERTIESSET(Package *package) const
{
	return ver_properties(package, ExtendedVersion::PROPERTIES_SET);
}

string
PrintFormat::VER_HAVEDEPEND(Package *package ATTRIBUTE_UNUSED) const
{
	UNUSED(package);
	return IS_FALSE((unlikely(version_variables->isinst)) || version_variables->version()->depend.depend_empty());
}

string
PrintFormat::VER_HAVERDEPEND(Package *package ATTRIBUTE_UNUSED) const
{
	UNUSED(package);
	return IS_FALSE((unlikely(version_variables->isinst)) || version_variables->version()->depend.rdepend_empty());
}

string
PrintFormat::VER_HAVEPDEPEND(Package *package ATTRIBUTE_UNUSED) const
{
	UNUSED(package);
	return IS_FALSE((unlikely(version_variables->isinst)) || version_variables->version()->depend.pdepend_empty());
}

string
PrintFormat::VER_HAVEHDEPEND(Package *package ATTRIBUTE_UNUSED) const
{
	UNUSED(package);
	return IS_FALSE((unlikely(version_variables->isinst)) || version_variables->version()->depend.hdepend_empty());
}

string
PrintFormat::VER_HAVEDEPS(Package *package ATTRIBUTE_UNUSED) const
{
	UNUSED(package);
	return IS_FALSE((unlikely(version_variables->isinst)) || version_variables->version()->depend.empty());
}

string
PrintFormat::VER_DEPENDS(Package *package ATTRIBUTE_UNUSED) const
{
	UNUSED(package);
	if(unlikely(version_variables->isinst)) {
		return STRING_EMPTY;
	}
	return version_variables->version()->depend.get_depend_brief();
}

string
PrintFormat::VER_DEPEND(Package *package ATTRIBUTE_UNUSED) const
{
	UNUSED(package);
	if(unlikely(version_variables->isinst)) {
		return STRING_EMPTY;
	}
	return version_variables->version()->depend.get_depend();
}

string
PrintFormat::VER_RDEPENDS(Package *package ATTRIBUTE_UNUSED) const
{
	UNUSED(package);
	if(unlikely(version_variables->isinst)) {
		return STRING_EMPTY;
	}
	return version_variables->version()->depend.get_rdepend_brief();
}


string
PrintFormat::VER_RDEPEND(Package *package ATTRIBUTE_UNUSED) const
{
	UNUSED(package);
	if(unlikely(version_variables->isinst)) {
		return STRING_EMPTY;
	}
	return version_variables->version()->depend.get_rdepend();
}

string
PrintFormat::VER_PDEPENDS(Package *package ATTRIBUTE_UNUSED) const
{
	UNUSED(package);
	if(unlikely(version_variables->isinst)) {
		return STRING_EMPTY;
	}
	return version_variables->version()->depend.get_pdepend_brief();
}

string
PrintFormat::VER_PDEPEND(Package *package ATTRIBUTE_UNUSED) const
{
	UNUSED(package);
	if(unlikely(version_variables->isinst)) {
		return STRING_EMPTY;
	}
	return version_variables->version()->depend.get_pdepend();
}

string
PrintFormat::VER_HDEPENDS(Package *package ATTRIBUTE_UNUSED) const
{
	UNUSED(package);
	if(unlikely(version_variables->isinst)) {
		return STRING_EMPTY;
	}
	return version_variables->version()->depend.get_hdepend_brief();
}

string
PrintFormat::VER_HDEPEND(Package *package ATTRIBUTE_UNUSED) const
{
	UNUSED(package);
	if(unlikely(version_variables->isinst)) {
		return STRING_EMPTY;
	}
	return version_variables->version()->depend.get_hdepend();
}

const MaskFlags *
PrintFormat::ver_maskflags() const
{
	return ((unlikely(version_variables->isinst)) ? NULLPTR : (&(version_variables->version()->maskflags)));
}

string
PrintFormat::VER_ISHARDMASKED(Package *package ATTRIBUTE_UNUSED) const
{
	UNUSED(package);
	const MaskFlags *maskflags(ver_maskflags());
	return IS_TRUE((maskflags != NULLPTR) && (maskflags->isHardMasked()));
}

string
PrintFormat::VER_ISPROFILEMASKED(Package *package ATTRIBUTE_UNUSED) const
{
	UNUSED(package);
	const MaskFlags *maskflags(ver_maskflags());
	return IS_TRUE((maskflags != NULLPTR) && (maskflags->isProfileMask()));
}

string
PrintFormat::VER_ISMASKED(Package *package ATTRIBUTE_UNUSED) const
{
	UNUSED(package);
	const MaskFlags *maskflags(ver_maskflags());
	return IS_TRUE((maskflags != NULLPTR) && (maskflags->isPackageMask()));
}

const KeywordsFlags *
PrintFormat::ver_keywordsflags() const
{
	return ((unlikely(version_variables->isinst)) ? NULLPTR : (&(version_variables->version()->keyflags)));
}

string
PrintFormat::VER_ISSTABLE(Package *package ATTRIBUTE_UNUSED) const
{
	UNUSED(package);
	const KeywordsFlags *keywordsflags(ver_keywordsflags());
	return IS_TRUE((keywordsflags != NULLPTR) && (keywordsflags->isStable()));
}

string
PrintFormat::VER_ISUNSTABLE(Package *package ATTRIBUTE_UNUSED) const
{
	UNUSED(package);
	const KeywordsFlags *keywordsflags(ver_keywordsflags());
	return IS_TRUE((keywordsflags != NULLPTR) && (keywordsflags->isUnstable()));
}

string
PrintFormat::VER_ISALIENSTABLE(Package *package ATTRIBUTE_UNUSED) const
{
	UNUSED(package);
	const KeywordsFlags *keywordsflags(ver_keywordsflags());
	return IS_TRUE((keywordsflags != NULLPTR) && (keywordsflags->isAlienStable()));
}

string
PrintFormat::VER_ISALIENUNSTABLE(Package *package ATTRIBUTE_UNUSED) const
{
	UNUSED(package);
	const KeywordsFlags *keywordsflags(ver_keywordsflags());
	return IS_TRUE((keywordsflags != NULLPTR) && (keywordsflags->isAlienUnstable()));
}

string
PrintFormat::VER_ISMISSINGKEYWORD(Package *package ATTRIBUTE_UNUSED) const
{
	UNUSED(package);
	const KeywordsFlags *keywordsflags(ver_keywordsflags());
	return IS_TRUE((keywordsflags != NULLPTR) && (keywordsflags->isMissingKeyword()));
}

string
PrintFormat::VER_ISMINUSKEYWORD(Package *package ATTRIBUTE_UNUSED) const
{
	UNUSED(package);
	const KeywordsFlags *keywordsflags(ver_keywordsflags());
	return IS_TRUE((keywordsflags != NULLPTR) && (keywordsflags->isMinusKeyword()));
}

string
PrintFormat::VER_ISMINUSUNSTABLE(Package *package ATTRIBUTE_UNUSED) const
{
	UNUSED(package);
	const KeywordsFlags *keywordsflags(ver_keywordsflags());
	return IS_TRUE((keywordsflags != NULLPTR) && (keywordsflags->isMinusUnstable()));
}

string
PrintFormat::VER_ISMINUSASTERISK(Package *package ATTRIBUTE_UNUSED) const
{
	UNUSED(package);
	const KeywordsFlags *keywordsflags(ver_keywordsflags());
	return IS_TRUE((keywordsflags != NULLPTR) && (keywordsflags->isMinusAsterisk()));
}

bool
PrintFormat::ver_wasflags(Package *package, MaskFlags *maskflags, KeywordsFlags *keyflags) const
{
	if(unlikely(version_variables->isinst)) {
		return false;
	}
	stability->calc_version_flags(false, maskflags, keyflags, version_variables->version(), package);
	return true;
}

string
PrintFormat::VER_WASHARDMASKED(Package *package) const
{
	MaskFlags maskflags;
	return IS_TRUE(ver_wasflags(package, &maskflags, NULLPTR) && maskflags.isHardMasked());
}

string
PrintFormat::VER_WASPROFILEMASKED(Package *package) const
{
	MaskFlags maskflags;
	return IS_TRUE(ver_wasflags(package, &maskflags, NULLPTR) && maskflags.isProfileMask());
}

string
PrintFormat::VER_WASMASKED(Package *package) const
{
	MaskFlags maskflags;
	return IS_TRUE(ver_wasflags(package, &maskflags, NULLPTR) && maskflags.isPackageMask());
}

string
PrintFormat::VER_WASSTABLE(Package *package) const
{
	KeywordsFlags keywordsflags;
	return IS_TRUE(ver_wasflags(package, NULLPTR, &keywordsflags) && keywordsflags.isStable());
}

string
PrintFormat::VER_WASUNSTABLE(Package *package) const
{
	KeywordsFlags keywordsflags;
	return IS_TRUE(ver_wasflags(package, NULLPTR, &keywordsflags) && keywordsflags.isUnstable());
}

string
PrintFormat::VER_WASALIENSTABLE(Package *package) const
{
	KeywordsFlags keywordsflags;
	return IS_TRUE(ver_wasflags(package, NULLPTR, &keywordsflags) && keywordsflags.isAlienStable());
}

string
PrintFormat::VER_WASALIENUNSTABLE(Package *package) const
{
	KeywordsFlags keywordsflags;
	return IS_TRUE(ver_wasflags(package, NULLPTR, &keywordsflags) && keywordsflags.isAlienUnstable());
}

string
PrintFormat::VER_WASMISSINGKEYWORD(Package *package) const
{
	KeywordsFlags keywordsflags;
	return IS_TRUE(ver_wasflags(package, NULLPTR, &keywordsflags) && keywordsflags.isMissingKeyword());
}

string
PrintFormat::VER_WASMINUSKEYWORD(Package *package) const
{
	KeywordsFlags keywordsflags;
	return IS_TRUE(ver_wasflags(package, NULLPTR, &keywordsflags) && keywordsflags.isMinusKeyword());
}

string
PrintFormat::VER_WASMINUSUNSTABLE(Package *package) const
{
	KeywordsFlags keywordsflags;
	return IS_TRUE(ver_wasflags(package, NULLPTR, &keywordsflags) && keywordsflags.isMinusUnstable());
}

string
PrintFormat::VER_WASMINUSASTERISK(Package *package) const
{
	KeywordsFlags keywordsflags;
	return IS_TRUE(ver_wasflags(package, NULLPTR, &keywordsflags) && keywordsflags.isMinusAsterisk());
}

string
PrintFormat::VER_HAVEMASKREASONS(Package *package ATTRIBUTE_UNUSED) const
{
	UNUSED(package);
	if(unlikely(version_variables->isinst)) {
		return STRING_FALSE;
	}
	return IS_TRUE(version_variables->version()->have_reasons());
}

string
PrintFormat::ver_maskreasons(const string &skip, const string &sep) const
{
	if(unlikely(version_variables->isinst)) {
		return STRING_EMPTY;
	}
	string s;
	version_variables->version()->reasons_string(&s, skip, sep);
	return s;
}

string
PrintFormat::VER_MASKREASONS(Package *package ATTRIBUTE_UNUSED) const
{
	UNUSED(package);
	return ver_maskreasons(maskreasons_skip, maskreasons_sep);
}

string
PrintFormat::VER_MASKREASONSS(Package *package ATTRIBUTE_UNUSED) const
{
	UNUSED(package);
	return ver_maskreasons(maskreasonss_skip, maskreasonss_sep);
}

static Package *
old_or_new(string *new_name, Package *older, Package *newer, const string &name)
{
	const char *s(name.c_str());
	if(strncmp(s, "old", 3) == 0) {
		*new_name = s + 3;
		return older;
	}
	if(strncmp(s, "new", 3) == 0) {
		*new_name = s + 3;
		return newer;
	}
	*new_name = name;
	return newer;
}

string
get_package_property(const PrintFormat *fmt, void *entity, const string &name)
{
	return fmt->get_pkg_property(static_cast<Package *>(entity), name);
}

string
get_diff_package_property(const PrintFormat *fmt, void *entity, const string &name)
{
	Package *older((static_cast<Package**>(entity))[0]);
	Package *newer((static_cast<Package**>(entity))[1]);
	Scanner::Diff diff(scanner->get_diff(name));
	if(unlikely(diff != Scanner::DIFF_NONE)) {
		LocalCopy copynewer(fmt, newer);
		LocalCopy copyolder(fmt, older);
		bool result;
		switch(diff) {
			case Scanner::DIFF_BETTER:
				result = newer->have_worse(*older, true);
				break;
			case Scanner::DIFF_BESTBETTER:
				result = newer->have_worse(*older, false);
				break;
			case Scanner::DIFF_WORSE:
				result = older->have_worse(*newer, true);
				break;
			case Scanner::DIFF_BESTWORSE:
				result = older->have_worse(*newer, false);
				break;
			case Scanner::DIFF_DIFFER:
				result = newer->differ(*older, true);
				break;
			default:
			// case Scanner::DIFF_BESTDIFFER:
				result = newer->differ(*older, false);
				break;
		}
		copyolder.restore(older);
		copynewer.restore(newer);
		return IS_TRUE(result);
	}
	string new_name;
	Package *package(old_or_new(&new_name, older, newer, name));
	return fmt->get_pkg_property(package, new_name);
}
