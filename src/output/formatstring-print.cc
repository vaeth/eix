// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include <config.h>
#include "formatstring-print.h"
#include <eixTk/ansicolor.h>
#include <eixTk/exceptions.h>
#include <eixTk/i18n.h>
#include <eixTk/likely.h>
#include <eixTk/stringutils.h>
#include <eixTk/sysutils.h>
#include <eixrc/eixrc.h>
#include <output/formatstring.h>
#include <portage/conf/portagesettings.h>
#include <portage/extendedversion.h>
#include <portage/instversion.h>
#include <portage/keywords.h>
#include <portage/package.h>
#include <portage/vardbpkg.h>
#include <portage/version.h>

#include <string>
#include <vector>
#include <map>

#include <cstddef>
#include <cstring>

using namespace std;

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
			m_version = NULL;
			m_instver = NULL;
			first = last = slotfirst = slotlast = oneslot = true;
			isinst = false;
		}

		void setinst(InstVersion *inst)
		{ know_restrict = false; m_instver = inst; }

		void setversion(const Version *ver)
		{ m_version = ver; }

		const Version *version() const
		{ return m_version; }

		InstVersion *instver() const
		{ return m_instver; }
};

string
PrintFormat::get_inst_use(const Package &package, InstVersion &i) const
{
	if((unlikely(vardb == NULL)) || !(vardb->readUse(package, i)))
		return emptystring;
	if(i.inst_iuse.empty())
		return emptystring;
	string ret, add;
	for(vector<string>::iterator it(i.inst_iuse.begin());
		likely(it != i.inst_iuse.end()); ++it) {
		bool is_unset(false);
		string *curr(&ret);
		if(i.usedUse.find(*it) == i.usedUse.end()) {
			is_unset = true;
			if(!alpha_use)
				curr = &add;
		}
		if(!curr->empty())
			curr->append(1, ' ');
		if(is_unset)
			curr->append(before_unset_use);
		else
			curr->append(before_set_use);
		curr->append(*it);
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
	return ret;
}

string
PrintFormat::get_version_keywords(Package *package, const Version *version) const
{
	if(print_effective) {
		portagesettings->get_effective_keywords_userprofile(package);
	}
	string keywords(version->get_full_keywords());
	string effective(version->get_effective_keywords());
	if(keywords.empty()) {
		if(effective.empty() || !print_effective)
			return emptystring;
	}
	string result(before_keywords);
	result.append(keywords);
	result.append(after_keywords);
	if(print_effective && (keywords != effective)) {
		result.append(before_ekeywords);
		result.append(effective);
		result.append(after_ekeywords);
	}
	return result;
}

void
PrintFormat::get_installed(Package *package, Node *root, bool only_marked) const
{
	if(unlikely(vardb == NULL))
		return;
	if(unlikely((unlikely(only_marked)) && (marked_list == NULL)))
		return;
	vector<InstVersion> *vec(vardb->getInstalledVector(*package));
	if(vec == NULL)
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
		if(unlikely(versions != NULL)) {
			if(likely(find(versions->begin(), versions->end(), *vit) == versions->end()))
				continue;
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
	if(unlikely(versions != NULL)) {
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
	}
	else
		slotnum = sl->size();
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
			if(unlikely(versions != NULL)) {
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

class Scanner {
	public:
		enum Prop {
			// No scanning result:
			PROP_NONE,

			// Used only in get_diff_package_property():
			DIFF_BETTER,
			DIFF_BESTBETTER,
			DIFF_WORSE,
			DIFF_BESTWORSE,
			DIFF_DIFFER,
			DIFF_BESTDIFFER,

			// Used only after colon:
			COLON_VER_DATE,
			COLON_PKG_AVAILABLEVERSIONS,
			COLON_PKG_MARKEDVERSIONS,
			COLON_PKG_BESTVERSIONS,
			COLON_PKG_BESTVERSION,
			COLON_PKG_BESTSLOTVERSIONSS,
			COLON_PKG_BESTSLOTVERSIONS,
			COLON_PKG_BESTSLOTUPGRADEVERSIONSS,
			COLON_PKG_BESTSLOTUPGRADEVERSIONS,
			COLON_PKG_INSTALLEDMARKEDVERSIONS,
			COLON_PKG_INSTALLEDVERSIONS,

			// General package properties:
			PKG_INSTALLED,
			PKG_VERSIONLINES,
			PKG_SLOTSORTED,
			PKG_COLOR,
			PKG_HAVEBEST,
			PKG_HAVEBESTS,
			PKG_CATEGORY,
			PKG_NAME,
			PKG_DESCRIPTION,
			PKG_HOMEPAGE,
			PKG_LICENSES,
			PKG_OVERLAYKEY,
			PKG_BINARY,
			PKG_SYSTEM,
			PKG_WORLD,
			PKG_WORLD_SETS,
			PKG_SETNAMES,
			PKG_ALLSETNAMES,
			PKG_UPGRADE,
			PKG_UPGRADEORINSTALL,
			PKG_BESTUPGRADE,
			PKG_BESTUPGRADEORINSTALL,
			PKG_DOWNGRADE,
			PKG_BESTDOWNGRADE,
			PKG_RECOMMEND,
			PKG_RECOMMENDORINSTALL,
			PKG_BESTRECOMMEND,
			PKG_BESTRECOMMENDORINSTALL,
			PKG_MARKED,
			PKG_HAVEMARKEDVERSION,
			PKG_SLOTS,
			PKG_SLOTTED,
			PKG_HAVECOLLIUSE,
			PKG_COLLIUSE,
			PKG_HAVEVERSIONUSE,

			// Used only in version context:
			VER_FIRST,
			VER_LAST,
			VER_SLOTFIRST,
			VER_SLOTLAST,
			VER_ONESLOT,
			VER_SLOT,
			VER_ISSLOT,
			VER_VERSION,
			VER_OVERLAYNUM,
			VER_OVERLAYVER,
			VER_VERSIONKEYWORDS,
			VER_ISBESTUPGRADESLOTS,
			VER_ISBESTUPGRADESLOT,
			VER_ISBESTUPGRADES,
			VER_ISBESTUPGRADE,
			VER_MARKEDVERSION,
			VER_INSTALLEDVERSION,
			VER_HAVEUSE,
			VER_USE,
			VER_ISBINARY,
			VER_RESTRICT,
			VER_RESTRICTFETCH,
			VER_RESTRICTMIRROR,
			VER_RESTRICTPRIMARYURI,
			VER_RESTRICTBINCHECKS,
			VER_RESTRICTSTRIP,
			VER_RESTRICTTEST,
			VER_RESTRICTUSERPRIV,
			VER_RESTRICTINSTALLSOURCES,
			VER_RESTRICTBINDIST,
			VER_RESTRICTPARALLEL,
			VER_PROPERTIES,
			VER_PROPERTIESINTERACTIVE,
			VER_PROPERTIESLIVE,
			VER_PROPERTIESVIRTUAL,
			VER_PROPERTIESSET,
			VER_ISHARDMASKED,
			VER_ISPROFILEMASKED,
			VER_ISMASKED,
			VER_ISSTABLE,
			VER_ISUNSTABLE,
			VER_ISALIENSTABLE,
			VER_ISALIENUNSTABLE,
			VER_ISMISSINGKEYWORD,
			VER_ISMINUSKEYWORD,
			VER_ISMINUSUNSTABLE,
			VER_ISMINUSASTERISK,
			VER_WASHARDMASKED,
			VER_WASPROFILEMASKED,
			VER_WASMASKED,
			VER_WASSTABLE,
			VER_WASUNSTABLE,
			VER_WASALIENSTABLE,
			VER_WASALIENUNSTABLE,
			VER_WASMISSINGKEYWORD,
			VER_WASMINUSKEYWORD,
			VER_WASMINUSUNSTABLE,
			VER_WASMINUSASTERISK
		};
		enum PropType { PKG, VER };

	protected:
		map<string, Prop> diff;
		map<string, pair<Prop,PropType> > prop, colon;

		void prop_colon_pkg(const char *s, Prop p)
		{ colon[s] = pair<Prop,PropType>(p, PKG); }

		void prop_colon_ver(const char *s, Prop p)
		{ colon[s] = pair<Prop,PropType>(p, VER); }

		void prop_pkg(const char *s, Prop p)
		{ prop[s] = pair<Prop,PropType>(p, PKG); }

		void prop_ver(const char *s, Prop p)
		{ prop[s] = pair<Prop,PropType>(p, VER); }
	public:
		Scanner()
		{
			diff["better"] = DIFF_BETTER;
			diff["bestbetter"] = DIFF_BESTBETTER;
			diff["worse"] = DIFF_WORSE;
			diff["bestworse"] = DIFF_BESTWORSE;
			diff["differ"] = DIFF_DIFFER;
			diff["bestdiffer"] = DIFF_BESTDIFFER;
			prop_colon_ver("date", COLON_VER_DATE);
			prop_colon_pkg("availableversions", COLON_PKG_AVAILABLEVERSIONS);
			prop_colon_pkg("markedversions", COLON_PKG_MARKEDVERSIONS);
			prop_colon_pkg("bestversion*", COLON_PKG_BESTVERSIONS);
			prop_colon_pkg("bestversion", COLON_PKG_BESTVERSION);
			prop_colon_pkg("bestslotversions*", COLON_PKG_BESTSLOTVERSIONSS);
			prop_colon_pkg("bestslotversions", COLON_PKG_BESTSLOTVERSIONS);
			prop_colon_pkg("bestslotupgradeversions*", COLON_PKG_BESTSLOTUPGRADEVERSIONSS);
			prop_colon_pkg("bestslotupgradeversions", COLON_PKG_BESTSLOTUPGRADEVERSIONS);
			prop_colon_pkg("installedmarkedversions", COLON_PKG_INSTALLEDMARKEDVERSIONS);
			prop_colon_pkg("installedversions", COLON_PKG_INSTALLEDVERSIONS);
			prop_pkg("installed", PKG_INSTALLED);
			prop_pkg("versionlines", PKG_VERSIONLINES);
			prop_pkg("slotsorted", PKG_SLOTSORTED);
			prop_pkg("color", PKG_COLOR);
			prop_pkg("havebest", PKG_HAVEBEST);
			prop_pkg("havebest*", PKG_HAVEBESTS);
			prop_pkg("category", PKG_CATEGORY);
			prop_pkg("name", PKG_NAME);
			prop_pkg("description", PKG_DESCRIPTION);
			prop_pkg("homepage", PKG_HOMEPAGE);
			prop_pkg("licenses", PKG_LICENSES);
			prop_pkg("overlaykey", PKG_OVERLAYKEY);
			prop_pkg("binary", PKG_BINARY);
			prop_pkg("system", PKG_SYSTEM);
			prop_pkg("world", PKG_WORLD);
			prop_pkg("world_sets", PKG_WORLD_SETS);
			prop_pkg("setnames", PKG_SETNAMES);
			prop_pkg("allsetnames", PKG_ALLSETNAMES);
			prop_pkg("upgrade", PKG_UPGRADE);
			prop_pkg("upgradeorinstall", PKG_UPGRADEORINSTALL);
			prop_pkg("bestupgrade", PKG_BESTUPGRADE);
			prop_pkg("bestupgradeorinstall", PKG_BESTUPGRADEORINSTALL);
			prop_pkg("downgrade", PKG_DOWNGRADE);
			prop_pkg("bestdowngrade", PKG_BESTDOWNGRADE);
			prop_pkg("recommend", PKG_RECOMMEND);
			prop_pkg("recommendorinstall", PKG_RECOMMENDORINSTALL);
			prop_pkg("bestrecommend", PKG_BESTRECOMMEND);
			prop_pkg("bestrecommendorinstall", PKG_BESTRECOMMENDORINSTALL);
			prop_pkg("marked", PKG_MARKED);
			prop_pkg("havemarkedversion", PKG_HAVEMARKEDVERSION);
			prop_pkg("slots", PKG_SLOTS);
			prop_pkg("slotted", PKG_SLOTTED);
			prop_pkg("havecolliuse", PKG_HAVECOLLIUSE);
			prop_pkg("colliuse", PKG_COLLIUSE);
			prop_pkg("haveversionuse", PKG_HAVEVERSIONUSE);
			prop_ver("first", VER_FIRST);
			prop_ver("last", VER_LAST);
			prop_ver("slotfirst", VER_SLOTFIRST);
			prop_ver("slotlast", VER_SLOTLAST);
			prop_ver("oneslot", VER_ONESLOT);
			prop_ver("slot", VER_SLOT);
			prop_ver("isslot", VER_ISSLOT);
			prop_ver("version", VER_VERSION);
			prop_ver("overlaynum", VER_OVERLAYNUM);
			prop_ver("overlayver", VER_OVERLAYVER);
			prop_ver("versionkeywords", VER_VERSIONKEYWORDS);
			prop_ver("isbestupgradeslot*", VER_ISBESTUPGRADESLOTS);
			prop_ver("isbestupgradeslot", VER_ISBESTUPGRADESLOT);
			prop_ver("isbestupgrade*", VER_ISBESTUPGRADES);
			prop_ver("isbestupgrade", VER_ISBESTUPGRADE);
			prop_ver("markedversion", VER_MARKEDVERSION);
			prop_ver("installedversion", VER_INSTALLEDVERSION);
			prop_ver("haveuse", VER_HAVEUSE);
			prop_ver("use", VER_USE);
			prop_ver("isbinary", VER_ISBINARY);
			prop_ver("restrict", VER_RESTRICT);
			prop_ver("restrictfetch", VER_RESTRICTFETCH);
			prop_ver("restrictmirror", VER_RESTRICTMIRROR);
			prop_ver("restrictprimaryuri", VER_RESTRICTPRIMARYURI);
			prop_ver("restrictbinchecks", VER_RESTRICTBINCHECKS);
			prop_ver("restrictstrip", VER_RESTRICTSTRIP);
			prop_ver("restricttest", VER_RESTRICTTEST);
			prop_ver("restrictuserpriv", VER_RESTRICTUSERPRIV);
			prop_ver("restrictinstallsources", VER_RESTRICTINSTALLSOURCES);
			prop_ver("restrictbindist", VER_RESTRICTBINDIST);
			prop_ver("restrictparallel", VER_RESTRICTPARALLEL);
			prop_ver("properties", VER_PROPERTIES);
			prop_ver("propertiesinteractive", VER_PROPERTIESINTERACTIVE);
			prop_ver("propertieslive", VER_PROPERTIESLIVE);
			prop_ver("propertiesvirtual", VER_PROPERTIESVIRTUAL);
			prop_ver("propertiesset", VER_PROPERTIESSET);
			prop_ver("ishardmasked", VER_ISHARDMASKED);
			prop_ver("isprofilemasked", VER_ISPROFILEMASKED);
			prop_ver("ismasked", VER_ISMASKED);
			prop_ver("isstable", VER_ISSTABLE);
			prop_ver("isunstable", VER_ISUNSTABLE);
			prop_ver("isalienstable", VER_ISALIENSTABLE);
			prop_ver("isalienunstable", VER_ISALIENUNSTABLE);
			prop_ver("ismissingkeyword", VER_ISMISSINGKEYWORD);
			prop_ver("isminuskeyword", VER_ISMINUSKEYWORD);
			prop_ver("isminusunstable", VER_ISMINUSUNSTABLE);
			prop_ver("isminusasterisk", VER_ISMINUSASTERISK);
			prop_ver("washardmasked", VER_WASHARDMASKED);
			prop_ver("wasprofilemasked", VER_WASPROFILEMASKED);
			prop_ver("wasmasked", VER_WASMASKED);
			prop_ver("wasstable", VER_WASSTABLE);
			prop_ver("wasunstable", VER_WASUNSTABLE);
			prop_ver("wasalienstable", VER_WASALIENSTABLE);
			prop_ver("wasalienunstable", VER_WASALIENUNSTABLE);
			prop_ver("wasmissingkeyword", VER_WASMISSINGKEYWORD);
			prop_ver("wasminuskeyword", VER_WASMINUSKEYWORD);
			prop_ver("wasminusunstable", VER_WASMINUSUNSTABLE);
			prop_ver("wasminusasterisK", VER_WASMINUSASTERISK);
		}

		Prop get_diff(const string& s) const ATTRIBUTE_PURE
		{
			map<string, Prop>::const_iterator it(diff.find(s));
			if(it == diff.end())
				return PROP_NONE;
			return it->second;
		}

		Prop get_colon(const string& s, PropType *p) const
		{
			map<string, pair<Prop,PropType> >::const_iterator it(colon.find(s));
			if(it == colon.end())
				return PROP_NONE;
			*p = it->second.second;
			return it->second.first;
		}

		Prop get_prop(const string& s, PropType *p) const
		{
			map<string, pair<Prop,PropType> >::const_iterator it(prop.find(s));
			if(it == prop.end())
				return PROP_NONE;
			*p = it->second.second;
			return it->second.first;
		}
};
static Scanner scanner;

string
PrintFormat::get_pkg_property(Package *package, const string &name) const throw(ExBasic)
{
	Scanner::PropType t(Scanner::PKG);
	Scanner::Prop prop(scanner.get_prop(name, &t));
	string after_colon;
	if(prop == Scanner::PROP_NONE) {
		string::size_type col(name.find(':'));
		if(likely(col != string::npos))
			prop = scanner.get_colon(name.substr(0, col), &t);
		if(unlikely(prop == Scanner::PROP_NONE)) {
			throw ExBasic(_("Unknown property %r")) % name;
		}
		after_colon.assign(name, col + 1, string::npos);
	}
	if(unlikely((t == Scanner::VER) && (version_variables == NULL))) {
		throw ExBasic(_("Property %r used outside version context")) % name;
	}
	bool a(false);
	switch(prop) {
		case Scanner::COLON_VER_DATE:
			if(version_variables->isinst) {
				InstVersion *i(version_variables->instver());
				if(likely(vardb != NULL)) {
					vardb->readInstDate(*package, *i);
				}
				return date_conv((*eix_rc)[after_colon].c_str(), i->instDate);
			}
			break;
		case Scanner::COLON_PKG_AVAILABLEVERSIONS:
		case Scanner::COLON_PKG_MARKEDVERSIONS:
		case Scanner::COLON_PKG_BESTVERSIONS:
		case Scanner::COLON_PKG_BESTVERSION:
		case Scanner::COLON_PKG_BESTSLOTVERSIONSS:
		case Scanner::COLON_PKG_BESTSLOTVERSIONS:
		case Scanner::COLON_PKG_BESTSLOTUPGRADEVERSIONSS:
		case Scanner::COLON_PKG_BESTSLOTUPGRADEVERSIONS:
		case Scanner::COLON_PKG_INSTALLEDMARKEDVERSIONS:
		case Scanner::COLON_PKG_INSTALLEDVERSIONS:
			{
				// It is important that version_variables points to a local object:
				// This allows loops within loops.
				// Recursion is avoided by checking the variable names.
				VersionVariables variables;
				VersionVariables *previous_variables(version_variables);
				version_variables = &variables;
				string varsortname;
				string *parsed(NULL);
				switch(prop) {
					case Scanner::COLON_PKG_AVAILABLEVERSIONS:
						a = true;
					case Scanner::COLON_PKG_MARKEDVERSIONS:
						{
							vector<Version*> *versions(NULL);
							if(unlikely(!a)) {
								versions = new vector<Version*>;
								if(likely(marked_list != NULL)) {
									for(Package::const_iterator it(package->begin());
										likely(it != package->end()); ++it) {
										if(unlikely(marked_list->is_marked(*package, &(**it)))) {
											versions->push_back(*it);
										}
									}
								}
							}
							if(likely(a || !(versions->empty()))) {
								string::size_type col(after_colon.find(':'));
								if((col == string::npos) || !(package->have_nontrivial_slots())) {
									if(col != string::npos)
										after_colon.erase(col);
									parsed = &after_colon;
									get_versions_versorted(package, parse_variable(after_colon), versions);
								}
								else {
									varsortname.assign(after_colon, col + 1, string::npos);
									parsed = &varsortname;
									get_versions_slotsorted(package, parse_variable(varsortname), versions);
								}
							}
							if(versions != NULL)
								delete versions;
						}
						break;
					case Scanner::COLON_PKG_BESTVERSIONS:
						a = true;
					case Scanner::COLON_PKG_BESTVERSION:
						{
							const Version *ver(package->best(a));
							variables.setversion(ver);
							if(likely(ver != NULL)) {
								parsed = &after_colon;
								recPrint(&(variables.result), package, get_package_property, parse_variable(after_colon));
							}
						}
						break;
					case Scanner::COLON_PKG_BESTSLOTVERSIONSS:
						a = true;
					case Scanner::COLON_PKG_BESTSLOTVERSIONS:
						{
							vector<Version*> versions;
							package->best_slots(versions, a);
							if(!versions.empty()) {
								parsed = &after_colon;
								get_versions_versorted(package, parse_variable(after_colon), &versions);
							}
						}
						break;
					case Scanner::COLON_PKG_BESTSLOTUPGRADEVERSIONSS:
						a = true;
					case Scanner::COLON_PKG_BESTSLOTUPGRADEVERSIONS:
						{
							vector<Version*> versions;
							package->best_slots_upgrade(versions, vardb, portagesettings, a);
							if(!versions.empty()) {
								parsed = &after_colon;
								get_versions_versorted(package, parse_variable(after_colon), &versions);
							}
						}
						break;
					case Scanner::COLON_PKG_INSTALLEDMARKEDVERSIONS:
						a = true;
					default:
					//case Scanner::COLON_PKG_INSTALLEDVERSIONS:
						{
							variables.isinst = true;
							parsed = &after_colon;
							get_installed(package, parse_variable(after_colon), a);
						}
						break;
				}
				if(parsed)
					varcache[*parsed].in_use = false;
				version_variables = previous_variables;
				return variables.result;
			}
			break;
		case Scanner::PKG_INSTALLED:
			if(vardb) {
				vector<InstVersion> *vec(vardb->getInstalledVector(*package));
				if((vec != NULL) && (likely(!(vec->empty()))))
					return one;
			}
			break;
		case Scanner::PKG_VERSIONLINES:
			if(style_version_lines)
				return one;
			break;
		case Scanner::PKG_SLOTSORTED:
			if(slot_sorted)
				return one;
			break;
		case Scanner::PKG_COLOR:
			if(no_color)
				break;
			return one;
		case Scanner::PKG_HAVEBEST:
			if(package->best(false))
				return one;
			break;
		case Scanner::PKG_HAVEBESTS:
			if(package->best(true))
				return one;
			break;
		case Scanner::PKG_CATEGORY:
			return package->category;
		case Scanner::PKG_NAME:
			return package->name;
		case Scanner::PKG_DESCRIPTION:
			return package->desc;
		case Scanner::PKG_HOMEPAGE:
			return package->homepage;
		case Scanner::PKG_LICENSES:
			return package->licenses;
		case Scanner::PKG_BINARY:
			{
				for(Package::const_iterator it(package->begin()); likely(it != package->end()); ++it) {
					if(it->have_bin_pkg(portagesettings, package))
						return one;
				}
				if(unlikely(vardb == NULL))
					break;
				vector<InstVersion> *vec(vardb->getInstalledVector(*package));
				if(vec == NULL)
					break;
				for(vector<InstVersion>::iterator it(vec->begin());
					likely(it != vec->end()); ++it) {
					if(it->have_bin_pkg(portagesettings, package))
						return one;
				}
			}
			break;
		case Scanner::PKG_OVERLAYKEY:
			{
				ExtendedVersion::Overlay ov_key(package->largest_overlay);
				if(ov_key && package->have_same_overlay_key())
					return overlay_keytext(ov_key, false);
			}
			break;
		case Scanner::PKG_SYSTEM:
			if(package->is_system_package())
				return one;
			break;
		case Scanner::PKG_WORLD:
			if(package->is_world_package())
				return one;
			break;
		case Scanner::PKG_WORLD_SETS:
			if(package->is_world_sets_package())
				return one;
			break;
		case Scanner::PKG_SETNAMES:
			return portagesettings->get_setnames(package);
		case Scanner::PKG_ALLSETNAMES:
			return portagesettings->get_setnames(package, true);
		case Scanner::PKG_UPGRADE:
		case Scanner::PKG_UPGRADEORINSTALL:
			a = true;
		case Scanner::PKG_BESTUPGRADE:
		case Scanner::PKG_BESTUPGRADEORINSTALL:
			{
				LocalCopy copy(this, package);
				bool result(package->can_upgrade(vardb, portagesettings,
					((prop == Scanner::PKG_UPGRADE) ||
					 (prop == Scanner::PKG_BESTUPGRADE)),
					a));
				copy.restore(package);
				if(result)
					return one;
			}
			break;
		case Scanner::PKG_DOWNGRADE:
			a = true;
		case Scanner::PKG_BESTDOWNGRADE:
			{
				LocalCopy copy(this, package);
				bool result(package->must_downgrade(vardb, a));
				copy.restore(package);
				if(result)
					return one;
			}
			break;
		case Scanner::PKG_RECOMMEND:
		case Scanner::PKG_RECOMMENDORINSTALL:
			a = true;
		case Scanner::PKG_BESTRECOMMEND:
		case Scanner::PKG_BESTRECOMMENDORINSTALL:
			{
				LocalCopy copy(this, package);
				bool result(package->recommend(vardb, portagesettings,
					((prop == Scanner::PKG_RECOMMEND) ||
					 (prop == Scanner::PKG_BESTRECOMMEND)),
					a));
				copy.restore(package);
				if(result)
					return one;
			}
			break;
		case Scanner::PKG_MARKED:
			if(likely(marked_list != NULL)) {
				if(unlikely(marked_list->is_marked(*package)))
					return one;
			}
			break;
		case Scanner::PKG_HAVEMARKEDVERSION:
			if(marked_list) {
				for(Package::const_iterator it(package->begin());
					likely(it != package->end()); ++it) {
					if(marked_list->is_marked(*package, &(**it)))
						return one;
				}
			}
			break;
		case Scanner::PKG_SLOTS:
			if((package->slotlist()).size() > 1)
				return one;
			break;
		case Scanner::PKG_SLOTTED:
			if(package->have_nontrivial_slots())
				return one;
			break;
		case Scanner::PKG_HAVECOLLIUSE:
			if(package->iuse.empty())
				break;
			return one;
		case Scanner::PKG_COLLIUSE:
			return package->iuse.asString();
		case Scanner::PKG_HAVEVERSIONUSE:
#ifndef NOT_FULL_USE
			if(package->versions_have_full_use)
				return one;
#endif
			break;
		case Scanner::VER_FIRST:
			if(version_variables->first)
				return one;
			break;
		case Scanner::VER_LAST:
			if(version_variables->last)
				return one;
			break;
		case Scanner::VER_SLOTFIRST:
			if(version_variables->slotfirst)
				return one;
			break;
		case Scanner::VER_SLOTLAST:
			if(version_variables->slotlast)
				return one;
			break;
		case Scanner::VER_ONESLOT:
			if(version_variables->oneslot)
				return one;
			break;
		case Scanner::VER_SLOT:
			a = true;
		case Scanner::VER_ISSLOT:
			{
				const string *slot;
				if(version_variables->isinst) {
					InstVersion *i(version_variables->instver());
					if(unlikely((vardb == NULL) || !(package->guess_slotname(*i, vardb))))
						i->slotname = "?";
					slot = &(i->slotname);
				}
				else
					slot = &(version_variables->version()->slotname);
				if(a) {
					if(slot->empty())
						return "0";
					return *slot;
				}
				if((!(slot->empty())) && (*slot != "0"))
					return one;
			}
			break;
		case Scanner::VER_VERSION:
			if(version_variables->isinst)
				return version_variables->instver()->getFull();
			return version_variables->version()->getFull();
		case Scanner::VER_OVERLAYNUM:
			a = true;
		case Scanner::VER_OVERLAYVER:
			if(version_variables->isinst) {
				InstVersion *i(version_variables->instver());
				if(unlikely((unlikely(vardb == NULL)) ||
					(unlikely(header == NULL)) ||
					(unlikely(!(vardb->readOverlay(*package, *i, *header)))))) {
					if(a || no_color)
						return "[?]";
					return color_overlaykey + "[?]" +
						AnsiColor(AnsiColor::acDefault).asString();
				}
				if(i->overlay_key > 0) {
					if(a || (!package->have_same_overlay_key()) || (package->largest_overlay != i->overlay_key))
						return overlay_keytext(i->overlay_key, a);
				}
			}
			else if(a || (!package->have_same_overlay_key())) {
				if(version_variables->version()->overlay_key)
					return overlay_keytext(version_variables->version()->overlay_key, a);
			}
			break;
		case Scanner::VER_VERSIONKEYWORDS:
			if(likely(!version_variables->isinst))
				return get_version_keywords(package, version_variables->version());
			break;
		case Scanner::VER_ISBESTUPGRADESLOT:
		case Scanner::VER_ISBESTUPGRADESLOTS:
			a = true;
		case Scanner::VER_ISBESTUPGRADE:
		case Scanner::VER_ISBESTUPGRADES:
			if(unlikely(version_variables->isinst))
				break;
			if(unlikely((likely(vardb != NULL)) && (likely(portagesettings != NULL)) &&
				unlikely(package->is_best_upgrade(
					a,
					version_variables->version(),
					vardb, portagesettings,
					((prop == Scanner::VER_ISBESTUPGRADES) ||
					 (prop == Scanner::VER_ISBESTUPGRADESLOTS))))))
				return one;
			break;
		case Scanner::VER_MARKEDVERSION:
			if(unlikely(version_variables->isinst))
				break;
			if(unlikely((likely(marked_list != NULL)) && (unlikely(marked_list->is_marked(*package,
				version_variables->version())))))
				return one;
			break;
		case Scanner::VER_INSTALLEDVERSION:
			if(unlikely(version_variables->isinst))
				return one;
			if(unlikely((likely(vardb != NULL)) && (likely(header != NULL)) &&
				(unlikely(vardb->isInstalledVersion(*package,
					version_variables->version(), *header))))) {
				return one;
			}
			break;
		case Scanner::VER_HAVEUSE:
			if(version_variables->isinst) {
				InstVersion &i(*(version_variables->instver()));
				if((likely(vardb != NULL)) &&
					(likely(vardb->readUse(*package, i)))
					&& !(i.inst_iuse.empty())) {
					return one;
				}
				break;
			}
			if(!(version_variables->version()->iuse.empty()))
				return one;
			break;
		case Scanner::VER_USE:
			if(version_variables->isinst)
				return get_inst_use(*package, *(version_variables->instver()));
			return version_variables->version()->iuse.asString();
		case Scanner::VER_ISBINARY:
			if(version_variables->isinst) {
				if(version_variables->instver()->have_bin_pkg(portagesettings, package))
					return one;
				break;
			}
			if(version_variables->version()->have_bin_pkg(portagesettings, package))
				return one;
			break;
		case Scanner::VER_RESTRICT:
		case Scanner::VER_RESTRICTFETCH:
		case Scanner::VER_RESTRICTMIRROR:
		case Scanner::VER_RESTRICTPRIMARYURI:
		case Scanner::VER_RESTRICTBINCHECKS:
		case Scanner::VER_RESTRICTSTRIP:
		case Scanner::VER_RESTRICTTEST:
		case Scanner::VER_RESTRICTUSERPRIV:
		case Scanner::VER_RESTRICTINSTALLSOURCES:
		case Scanner::VER_RESTRICTBINDIST:
		case Scanner::VER_RESTRICTPARALLEL:
			{
				ExtendedVersion::Restrict restrict;
				if(version_variables->isinst) {
					if(!version_variables->know_restrict) {
						if(vardb && header &&
							(vardb->readRestricted(*package, *(version_variables->instver()), *header)))
							version_variables->know_restrict = true;
						else
							break;
					}
					restrict = version_variables->instver()->restrictFlags;
				}
				else
					restrict = version_variables->version()->restrictFlags;
				switch(prop) {
					case Scanner::VER_RESTRICTFETCH:
						if(unlikely(restrict & ExtendedVersion::RESTRICT_FETCH))
							return one;
						break;
					case Scanner::VER_RESTRICTMIRROR:
						if(unlikely(restrict & ExtendedVersion::RESTRICT_MIRROR))
							return one;
						break;
					case Scanner::VER_RESTRICTPRIMARYURI:
						if(unlikely(restrict & ExtendedVersion::RESTRICT_PRIMARYURI))
							return one;
						break;
					case Scanner::VER_RESTRICTBINCHECKS:
						if(unlikely(restrict & ExtendedVersion::RESTRICT_BINCHECKS))
							return one;
						break;
					case Scanner::VER_RESTRICTSTRIP:
						if(unlikely(restrict & ExtendedVersion::RESTRICT_STRIP))
							return one;
						break;
					case Scanner::VER_RESTRICTTEST:
						if(unlikely(restrict & ExtendedVersion::RESTRICT_TEST))
							return one;
						break;
					case Scanner::VER_RESTRICTUSERPRIV:
						if(unlikely(restrict & ExtendedVersion::RESTRICT_USERPRIV))
							return one;
						break;
					case Scanner::VER_RESTRICTINSTALLSOURCES:
						if(unlikely(restrict & ExtendedVersion::RESTRICT_INSTALLSOURCES))
							return one;
						break;
					case Scanner::VER_RESTRICTBINDIST:
						if(unlikely(restrict & ExtendedVersion::RESTRICT_BINDIST))
							return one;
						break;
					case Scanner::VER_RESTRICTPARALLEL:
						if(unlikely(restrict & ExtendedVersion::RESTRICT_PARALLEL))
							return one;
						break;
					default:
					//case Scanner::VER_RESTRICT:
						if(unlikely(restrict != ExtendedVersion::RESTRICT_NONE))
							return one;
						break;
				}
			}
			break;
		case Scanner::VER_PROPERTIES:
		case Scanner::VER_PROPERTIESINTERACTIVE:
		case Scanner::VER_PROPERTIESLIVE:
		case Scanner::VER_PROPERTIESVIRTUAL:
		case Scanner::VER_PROPERTIESSET:
			{
				ExtendedVersion::Properties properties;
				if(version_variables->isinst) {
					if(!version_variables->know_restrict) {
						if(vardb && header &&
							(vardb->readRestricted(*package, *(version_variables->instver()), *header)))
							version_variables->know_restrict = true;
						else
							break;
					}
					properties = version_variables->instver()->propertiesFlags;
				}
				else
					properties = version_variables->version()->propertiesFlags;
				switch(prop) {
					case Scanner::VER_PROPERTIESINTERACTIVE:
						if(unlikely(properties & ExtendedVersion::PROPERTIES_INTERACTIVE))
							return one;
						break;
					case Scanner::VER_PROPERTIESLIVE:
						if(unlikely(properties & ExtendedVersion::PROPERTIES_LIVE))
							return one;
						break;
					case Scanner::VER_PROPERTIESVIRTUAL:
						if(unlikely(properties & ExtendedVersion::PROPERTIES_VIRTUAL))
							return one;
						break;
					case Scanner::VER_PROPERTIESSET:
						if(unlikely(properties & ExtendedVersion::PROPERTIES_SET))
							return one;
						break;
					default:
					//case Scanner::VER_PROPERTIESSET:
						if(unlikely(properties != ExtendedVersion::PROPERTIES_NONE))
							return one;
						break;
				}
			}
			break;
		case Scanner::VER_ISHARDMASKED:
		case Scanner::VER_ISPROFILEMASKED:
		case Scanner::VER_ISMASKED:
		case Scanner::VER_ISSTABLE:
		case Scanner::VER_ISUNSTABLE:
		case Scanner::VER_ISALIENSTABLE:
		case Scanner::VER_ISALIENUNSTABLE:
		case Scanner::VER_ISMISSINGKEYWORD:
		case Scanner::VER_ISMINUSKEYWORD:
		case Scanner::VER_ISMINUSUNSTABLE:
		case Scanner::VER_ISMINUSASTERISK:
			a = true;
		default:
		//case Scanner::VER_WAS....
			if(likely(!(version_variables->isinst))) {
				const Version *version = version_variables->version();
				MaskFlags mymask(version->maskflags);
				KeywordsFlags mykey(version->keyflags);
				if(!a)
					stability->calc_version_flags(false, mymask, mykey, version, package);
				switch(prop) {
					case Scanner::VER_ISHARDMASKED:
					case Scanner::VER_WASHARDMASKED:
						if(mymask.isHardMasked())
							return one;
						break;
					case Scanner::VER_ISPROFILEMASKED:
					case Scanner::VER_WASPROFILEMASKED:
						if(mymask.isProfileMask())
							return one;
						break;
					case Scanner::VER_ISMASKED:
					case Scanner::VER_WASMASKED:
						if(mymask.isPackageMask())
							return one;
						break;
					case Scanner::VER_ISSTABLE:
					case Scanner::VER_WASSTABLE:
						if(mykey.isStable())
							return one;
						break;
					case Scanner::VER_ISUNSTABLE:
					case Scanner::VER_WASUNSTABLE:
						if(mykey.isUnstable())
							return one;
						break;
					case Scanner::VER_ISALIENSTABLE:
					case Scanner::VER_WASALIENSTABLE:
						if(mykey.isAlienStable())
							return one;
						break;
					case Scanner::VER_ISALIENUNSTABLE:
					case Scanner::VER_WASALIENUNSTABLE:
						if(mykey.isAlienUnstable())
							return one;
						break;
					case Scanner::VER_ISMISSINGKEYWORD:
					case Scanner::VER_WASMISSINGKEYWORD:
						if(mykey.isMissingKeyword())
							return one;
						break;
					case Scanner::VER_ISMINUSKEYWORD:
					case Scanner::VER_WASMINUSKEYWORD:
						if(mykey.isMinusKeyword())
							return one;
						break;
					case Scanner::VER_ISMINUSUNSTABLE:
					case Scanner::VER_WASMINUSUNSTABLE:
						if(mykey.isMinusUnstable())
							return one;
						break;
					default:
					//case Scanner::VER_ISMINUSASTERISK:
					//case Scanner::VER_WASMINUSASTERISK:
						if(mykey.isMinusAsterisk())
							return one;
						break;
				}
			}
			break;
	}
	return emptystring;
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
	Scanner::Prop diff(scanner.get_diff(name));
	if(unlikely(diff != Scanner::PROP_NONE)) {
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
			//case Scanner::DIFF_BESTDIFFER:
				result = newer->differ(*older, false);
				break;
		}
		copyolder.restore(older);
		copynewer.restore(newer);
		if(result)
			return one;
		return emptystring;
	}
	string new_name;
	Package *package(old_or_new(&new_name, older, newer, name));
	return fmt->get_pkg_property(package, new_name);
}
