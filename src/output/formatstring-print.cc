// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

// #define EIX_PARANOIC_ASSERT

#include "output/formatstring-print.h"
#include <config.h>  // IWYU pragma: keep

#include <cstdlib>
#include <cstring>

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "eixTk/ansicolor.h"
#include "eixTk/assert.h"
#include "eixTk/attribute.h"
#include "eixTk/formated.h"
#include "eixTk/i18n.h"
#include "eixTk/likely.h"
#include "eixTk/null.h"
#include "eixTk/outputstring.h"
#include "eixTk/stringtypes.h"
#include "eixTk/sysutils.h"
#include "eixTk/unordered_map.h"
#include "eixrc/eixrc.h"
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
using std::string;
using std::vector;

ATTRIBUTE_NONNULL_ static Package *old_or_new(string *new_name, Package *older, Package *newer, const string& name);

class VersionVariables {
	private:
		const Version *m_version;
		InstVersion *m_instver;

	public:
		bool first, last, slotfirst, slotlast, oneslot, isinst;
		OutputString result;

		VersionVariables() {
			m_version = NULLPTR;
			m_instver = NULLPTR;
			first = last = slotfirst = slotlast = oneslot = true;
			isinst = false;
		}

		void setinst(InstVersion *inst) {
			m_instver = inst;
		}

		void setversion(const Version *ver) {
			m_version = ver;
		}

		const Version *version() const {
			return m_version;
		}

		InstVersion *instver() const {
			return m_instver;
		}
};

void
PrintFormat::iuse_expand(OutputString *s, const IUseSet& iuse, bool coll, HandleExpand expand) const {
	if(expand == EXPAND_NO) {
		s->assign_smart(iuse.asString());
		return;
	}
	UNORDERED_MAP<string, OutputString> expvars;
	typedef vector<string> ExpVarNames;
	ExpVarNames expvarnames;
	const IUseSet::IUseNaturalOrder iuse_ordered(iuse.asNaturalOrder());
	for(IUseSet::IUseNaturalOrder::const_iterator it(iuse_ordered.begin());
		it != iuse_ordered.end(); ++it) {
		string var, expval;
		const string& name(it->name());
		if(portagesettings->use_expand(&var, &expval, name)) {
			if(expand == EXPAND_OMIT) {
				continue;
			}
			OutputString& r(expvars[var]);
			if(r.empty()) {
				expvarnames.push_back(var);
			} else {
				r.append_fast(' ');
			}
			const char *p(it->iuse().prefix());
			if(p != NULLPTR) {
				r.append_smart(p);
			}
			r.append_smart(expval);
		} else {
			if(!s->empty()) {
				s->append_fast(' ');
			}
			s->append_smart(it->asString());
		}
	}
	for(ExpVarNames::const_iterator it(expvarnames.begin()); it != expvarnames.end(); ++it) {
		if(!s->empty()) {
			s->append_fast(' ');
		}
		s->append(coll ? before_coll_start : before_iuse_start);
		s->append_smart(*it);
		s->append(coll ? before_coll_end : before_iuse_end);
		s->append(expvars[*it]);
		s->append(coll ? after_coll : after_iuse);
	}
}

void PrintFormat::get_inst_use(OutputString *s, const Package& package, InstVersion *i, HandleExpand expand) const {
	if((!(vardb->readUse(package, i))) || i->inst_iuse.empty()) {
		return;
	}
	OutputString add;
	string expval;
	map<string, pair<OutputString, OutputString> > expvars;
	typedef vector<string> ExpVarNames;
	ExpVarNames expvarnames;
	for(WordVec::iterator it(i->inst_iuse.begin());
		likely(it != i->inst_iuse.end()); ++it) {
		string *value(&(*it));
		bool is_unset(i->usedUse.count(*value) == 0);
		OutputString *curr(s);
		bool unset_list(false);
		if(is_unset && !alpha_use) {
			unset_list = true;
			curr = &add;
		}
		if(expand != EXPAND_NO) {
			string var;
			if(portagesettings->use_expand(&var, &expval, *value)) {
				if(expand == EXPAND_OMIT) {
					continue;
				}
				value = &expval;
				pair<OutputString, OutputString>& r(expvars[var]);
				if(r.first.empty() && r.second.empty()) {
					expvarnames.push_back(var);
				}
				curr = (unset_list ? &(r.second) : &(r.first));
			}
		}
		if(!curr->empty()) {
			curr->append_fast(' ');
		}
		if(is_unset) {
			curr->append(before_unset_use);
		} else {
			curr->append(before_set_use);
		}
		curr->append_smart(*value);
		if(is_unset) {
			curr->append(after_unset_use);
		} else {
			curr->append(after_set_use);
		}
	}
	if(!add.empty()) {
		if(!s->empty()) {
			s->append_fast(' ');
		}
		s->append(add);
	}
	for(ExpVarNames::const_iterator it(expvarnames.begin()); it != expvarnames.end(); ++it) {
		if(!s->empty()) {
			s->append_fast(' ');
		}
		s->append(before_use_start);
		s->append_smart(*it);
		s->append(before_use_end);
		pair<OutputString, OutputString> &r(expvars[*it]);
		const OutputString& f(r.first);
		s->append(f);
		const OutputString& t(r.second);
		if(!t.empty()) {
			if(!f.empty()) {
				s->append_fast(' ');
			}
			s->append(t);
		}
		s->append(after_use);
	}
}

void PrintFormat::get_installed(Package *package, Node *root) const {
	eix_assert_paranoic(vardb != NULLPTR);
	InstVec *vec(vardb->getInstalledVector(*package));
	if(vec == NULLPTR) {
		return;
	}
	bool have_prevversion(false);
	for(InstVec::iterator it(vec->begin());
		likely(it != vec->end()); ++it) {
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

void PrintFormat::get_versions_versorted(Package *package, Node *root, PrintFormat::VerVec *versions) const {
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

void PrintFormat::get_versions_slotsorted(Package *package, Node *root, PrintFormat::VerVec *versions) const {
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
	if(unlikely(slotnum == 0)) {
		return;
	}
	version_variables->oneslot = (slotnum == 1);

	bool have_prevversion(false);
	SlotList::size_type prevslot(slotnum + 1);
	version_variables->slotfirst = true;
	for(SlotList::const_iterator it(sl->begin()); likely(slotnum != 0); ++it, --slotnum) {
		const VersionList *vl(&(it->const_version_list()));
		for(VersionList::const_iterator vit(vl->begin());
			likely(vit != vl->end()); ++vit) {
			if(unlikely(versions != NULLPTR)) {
				if(likely(find(versions->begin(), versions->end(), *vit) == versions->end())) {
					continue;
				}
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

void PrintFormat::clear_virtual(ExtendedVersion::Overlay count) {
	delete virtuals;
	virtuals = new Virtuals(count, false);
}

void PrintFormat::set_as_virtual(const ExtendedVersion::Overlay overlay, bool on) {
	if(overlay == 0) {
		return;
	}
	(*virtuals)[overlay-1] = on;
}

bool PrintFormat::is_virtual(const ExtendedVersion::Overlay overlay) const {
	if(virtuals == NULLPTR) {
		return false;
	}
	if((overlay == 0) || (overlay >= virtuals->size())) {
		return false;
	}
	return (*virtuals)[overlay - 1];
}

bool PrintFormat::have_virtual(const Package *p, bool nonvirtual) const {
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
		typedef void (PrintFormat::*Plain)(OutputString *s, Package *pkg) const;
		typedef void (PrintFormat::*ColonVar)(Package *pkg, const string& after_colon) const;
		typedef void (PrintFormat::*ColonOther)(OutputString *s, Package *pkg, const string& after_colon) const;

	protected:
		UNORDERED_MAP<string, Diff> diff;
		UNORDERED_MAP<string, pair<Plain, Prop> > plain;
		UNORDERED_MAP<string, ColonVar> colon_var;
		UNORDERED_MAP<string, ColonOther> colon_other;

		ATTRIBUTE_NONNULL_ void prop_diff(const char *s, Diff diffprop) {
			diff[s] = diffprop;
		}

		ATTRIBUTE_NONNULL_ void prop_colon_pkg(const char *s, ColonVar colfunc) {
			colon_var[s] = colfunc;
		}

		ATTRIBUTE_NONNULL_ void prop_colon_ver(const char *s, ColonOther colfunc) {
			colon_other[s] = colfunc;
		}

		ATTRIBUTE_NONNULL_ void prop_pkg(const char *s, Plain plainfunc) {
			plain[s] = pair<Plain, Prop>(plainfunc, PKG);
		}

		ATTRIBUTE_NONNULL_ void prop_ver(const char *s, Plain plainfunc) {
			plain[s] = pair<Plain, Prop>(plainfunc, VER);
		}

	public:
		Scanner() {
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
			prop_pkg("mainrepo", &PrintFormat::PKG_MAINREPO);
			prop_pkg("overlaykey", &PrintFormat::PKG_OVERLAYKEY);
			prop_pkg("overlayname", &PrintFormat::PKG_OVERLAYNAME);
			prop_pkg("binary", &PrintFormat::PKG_BINARY);
			prop_pkg("system", &PrintFormat::PKG_SYSTEM);
			prop_pkg("profile", &PrintFormat::PKG_PROFILE);
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
			prop_pkg("colliuse0", &PrintFormat::PKG_COLLIUSE0);
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
			prop_ver("srcuri", &PrintFormat::VER_SRCURI);
			prop_ver("havesrcuri", &PrintFormat::VER_HAVESRCURI);
			prop_ver("eapi", &PrintFormat::VER_EAPI);
			prop_ver("version", &PrintFormat::VER_VERSION);
			prop_ver("plainversion", &PrintFormat::VER_PLAINVERSION);
			prop_ver("revision", &PrintFormat::VER_REVISION);
			prop_ver("overlaynum", &PrintFormat::VER_OVERLAYNUM);
			prop_ver("overlayver", &PrintFormat::VER_OVERLAYVER);
			prop_ver("overlayplainname*", &PrintFormat::VER_OVERLAYPLAINNAMES);
			prop_ver("overlayplainname", &PrintFormat::VER_OVERLAYPLAINNAME);
			prop_ver("overlayvername*", &PrintFormat::VER_OVERLAYVERNAMES);
			prop_ver("overlayvername", &PrintFormat::VER_OVERLAYVERNAME);
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
			prop_ver("use0", &PrintFormat::VER_USE0);
			prop_ver("use*", &PrintFormat::VER_USES);
			prop_ver("use", &PrintFormat::VER_USE);
			prop_ver("requireduse", &PrintFormat::VER_REQUIREDUSE);
			prop_ver("haverequireduse", &PrintFormat::VER_HAVEREQUIREDUSE);
			prop_ver("virtual", &PrintFormat::VER_VIRTUAL);
			prop_ver("isbinary", &PrintFormat::VER_ISBINARY);
			prop_ver("istbz", &PrintFormat::VER_ISTBZ);
			prop_ver("isgpkg", &PrintFormat::VER_ISGPKG);
			prop_ver("ispak", &PrintFormat::VER_ISPAK);
			prop_ver("ismultigpkg", &PrintFormat::VER_ISMULTIGPKG);
			prop_ver("ismultipak", &PrintFormat::VER_ISMULTIPAK);
			prop_ver("gpkgcount", &PrintFormat::VER_GPKGCOUNT);
			prop_ver("pakcount", &PrintFormat::VER_PAKCOUNT);
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
			prop_ver("havebdepend", &PrintFormat::VER_HAVEBDEPEND);
			prop_ver("haveidepend", &PrintFormat::VER_HAVEIDEPEND);
			prop_ver("havedeps", &PrintFormat::VER_HAVEDEPS);
			prop_ver("depend*", &PrintFormat::VER_DEPENDS);
			prop_ver("depend", &PrintFormat::VER_DEPEND);
			prop_ver("rdepend*", &PrintFormat::VER_RDEPENDS);
			prop_ver("rdepend", &PrintFormat::VER_RDEPEND);
			prop_ver("pdepend*", &PrintFormat::VER_PDEPENDS);
			prop_ver("pdepend", &PrintFormat::VER_PDEPEND);
			prop_ver("bdepend*", &PrintFormat::VER_BDEPENDS);
			prop_ver("bdepend", &PrintFormat::VER_BDEPEND);
			prop_ver("idepend*", &PrintFormat::VER_IDEPENDS);
			prop_ver("idepend", &PrintFormat::VER_IDEPEND);
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

		ATTRIBUTE_PURE Diff get_diff(const string& s) const {
			UNORDERED_MAP<string, Diff>::const_iterator it(diff.find(s));
			return ((it == diff.end()) ? DIFF_NONE : it->second);
		}

		ATTRIBUTE_NONNULL_ ColonVar get_colon_var(const string& s, Prop *p) const {
			UNORDERED_MAP<string, ColonVar>::const_iterator it(colon_var.find(s));
			if(it == colon_var.end()) {
				return NULLPTR;
			}
			*p = PKG;
			return it->second;
		}

		ATTRIBUTE_NONNULL_ ColonOther get_colon_other(const string& s, Prop *p) const {
			UNORDERED_MAP<string, ColonOther>::const_iterator it(colon_other.find(s));
			if(it == colon_other.end()) {
				return NULLPTR;
			}
			*p = VER;
			return it->second;
		}

		ATTRIBUTE_NONNULL_ Plain get_plain(const string& s, Prop *p) const {
			UNORDERED_MAP<string, pair<Plain, Prop> >::const_iterator it(plain.find(s));
			if(it == plain.end()) {
				return NULLPTR;
			}
			*p = it->second.second;
			return it->second.first;
		}
};

static Scanner *scanner = NULLPTR;

void PrintFormat::init_static() {
	eix_assert_static(scanner == NULLPTR);
	scanner = new Scanner;
	AnsiColor::init_static();
}

void PrintFormat::get_pkg_property(OutputString *s, Package *package, const string& name) const {
	eix_assert_static(scanner != NULLPTR);
	Scanner::Prop t;
	Scanner::Plain plain(scanner->get_plain(name, &t));
	// Unnecessary initializations, but silence warning of stupid compiler:
	Scanner::ColonVar colon_var(NULLPTR);
	Scanner::ColonOther colon_other(NULLPTR);
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
			eix::say_error(_("unknown property \"%s\"")) % name;
			std::exit(EXIT_FAILURE);
		}
		after_colon.assign(name, col + 1, string::npos);
	}
	if(unlikely((t == Scanner::VER) && (version_variables == NULLPTR))) {
		eix::say_error(_("property \"%s\" used outside version context")) % name;
		std::exit(EXIT_FAILURE);
	}
	if(plain != NULLPTR) {
		(this->*plain)(s, package);
		return;
	}
	if(colon_var == NULLPTR) {
		(this->*colon_other)(s, package, after_colon);
		return;
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
	s->assign(variables.result);
}

void PrintFormat::COLON_VER_DATE(OutputString *s, Package *package, const string& after_colon) const {
	if(version_variables->isinst) {
		InstVersion *i(version_variables->instver());
		vardb->readInstDate(*package, i);
		s->assign_smart(date_conv((*eix_rc)[after_colon].c_str(), i->instDate));
		return;
	}
}

void PrintFormat::colon_pkg_availableversions(Package *package, const string& after_colon, bool only_marked) const {
	VerVec *versions(NULLPTR);
	if(unlikely(only_marked)) {
		if(unlikely(marked_list == NULLPTR) ||
			likely(!marked_list->applyMasks(package))) {
			return;
		}
		for(Package::const_iterator it(package->begin());
			likely(it != package->end()); ++it) {
			if(unlikely(it->maskflags.isMarked())) {
				if(versions == NULLPTR) {
					versions = new VerVec;
				}
				versions->push_back(*it);
			}
		}
		if(versions == NULLPTR) {
			return;
		}
	}
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
	delete versions;
}

void PrintFormat::COLON_PKG_AVAILABLEVERSIONS(Package *package, const string& after_colon) const {
	colon_pkg_availableversions(package, after_colon, false);
}

void PrintFormat::COLON_PKG_MARKEDVERSIONS(Package *package, const string& after_colon) const {
	colon_pkg_availableversions(package, after_colon, true);
}

void PrintFormat::colon_pkg_bestversion(Package *package, const string& after_colon, bool allow_unstable) const {
	const Version *ver(package->best(allow_unstable));
	version_variables->setversion(ver);
	if(likely(ver != NULLPTR)) {
		recPrint(&(version_variables->result), package, get_package_property, parse_variable(after_colon));
		varcache[after_colon].in_use = false;
	}
}

void PrintFormat::COLON_PKG_BESTVERSION(Package *package, const string& after_colon) const {
	colon_pkg_bestversion(package, after_colon, false);
}

void PrintFormat::COLON_PKG_BESTVERSIONS(Package *package, const string& after_colon) const {
	colon_pkg_bestversion(package, after_colon, true);
}

void PrintFormat::colon_pkg_bestslotversions(Package *package, const string& after_colon, bool allow_unstable) const {
	const Version *ver(package->best(allow_unstable));
	version_variables->setversion(ver);
	VerVec versions;
	package->best_slots(&versions, allow_unstable);
	if(!versions.empty()) {
		get_versions_versorted(package, parse_variable(after_colon), &versions);
		varcache[after_colon].in_use = false;
	}
}

void PrintFormat::COLON_PKG_BESTSLOTVERSIONS(Package *package, const string& after_colon) const {
	colon_pkg_bestslotversions(package, after_colon, false);
}

void PrintFormat::COLON_PKG_BESTSLOTVERSIONSS(Package *package, const string& after_colon) const {
	colon_pkg_bestslotversions(package, after_colon, true);
}

void PrintFormat::colon_pkg_bestslotupgradeversions(Package *package, const string& after_colon, bool allow_unstable) const {
	VerVec versions;
	package->best_slots_upgrade(&versions, vardb, portagesettings, allow_unstable);
	if(!versions.empty()) {
		get_versions_versorted(package, parse_variable(after_colon), &versions);
		varcache[after_colon].in_use = false;
	}
}

void PrintFormat::COLON_PKG_BESTSLOTUPGRADEVERSIONS(Package *package, const string& after_colon) const {
	colon_pkg_bestslotupgradeversions(package, after_colon, false);
}

void PrintFormat::COLON_PKG_BESTSLOTUPGRADEVERSIONSS(Package *package, const string& after_colon) const {
	colon_pkg_bestslotupgradeversions(package, after_colon, true);
}

void PrintFormat::COLON_PKG_INSTALLEDVERSIONS(Package *package, const string& after_colon) const {
	version_variables->isinst = true;
	get_installed(package, parse_variable(after_colon));
	varcache[after_colon].in_use = false;
}

void PrintFormat::PKG_INSTALLED(OutputString *s, Package *package) const {
	eix_assert_paranoic(vardb != NULLPTR);
	InstVec *vec(vardb->getInstalledVector(*package));
	if((vec != NULLPTR) && (likely(!(vec->empty())))) {
		s->set_one();
	}
}

void PrintFormat::PKG_VERSIONLINES(OutputString *s, Package * /* package */) const {
	if(style_version_lines) {
		s->set_one();
	}
}

void PrintFormat::PKG_SLOTSORTED(OutputString *s, Package * /* package */) const {
	if(slot_sorted) {
		s->set_one();
	}
}

void PrintFormat::PKG_COLOR(OutputString *s, Package * /* package */) const {
	if(!no_color) {
		s->set_one();
	}
}

void PrintFormat::PKG_HAVEBEST(OutputString *s, Package *package) const {
	if(package->best(false) != NULLPTR) {
		s->set_one();
	}
}

void PrintFormat::PKG_HAVEBESTS(OutputString *s, Package *package) const {
	if(package->best(true) != NULLPTR) {
		s->set_one();
	}
}

void PrintFormat::PKG_CATEGORY(OutputString *s, Package *package) const {
	s->assign_smart(package->category);
}

void PrintFormat::PKG_NAME(OutputString *s, Package *package) const {
	s->assign_smart(package->name);
}

void PrintFormat::PKG_DESCRIPTION(OutputString *s, Package *package) const {
	s->assign_smart(package->desc);
}

void PrintFormat::PKG_HOMEPAGE(OutputString *s, Package *package) const {
	s->assign_smart(package->homepage);
}

void PrintFormat::PKG_LICENSES(OutputString *s, Package *package) const {
	s->assign_smart(package->licenses);
}

void PrintFormat::PKG_BINARY(OutputString *s, Package *package) const {
	for(Package::const_iterator it(package->begin()); likely(it != package->end()); ++it) {
		if(it->have_bin_pkg(portagesettings, package)) {
			s->set_one();
			return;
		}
	}
	eix_assert_paranoic(vardb != NULLPTR);
	InstVec *vec(vardb->getInstalledVector(*package));
	if(vec != NULLPTR) {
		for(InstVec::iterator it(vec->begin());
			likely(it != vec->end()); ++it) {
			if(it->have_bin_pkg(portagesettings, package)) {
				s->set_one();
				return;
			}
		}
	}
}

void PrintFormat::PKG_OVERLAYKEY(OutputString *s, Package *package) const {
	ExtendedVersion::Overlay ov_key(package->largest_overlay);
	if((ov_key != 0) && package->have_same_overlay_key()) {
		overlay_keytext(s, ov_key, false);
	}
}

void PrintFormat::PKG_OVERLAYNAME(OutputString *s, Package *package) const {
	ExtendedVersion::Overlay ov_key(package->largest_overlay);
	if((ov_key != 0) && package->have_same_overlay_key()) {
		s->assign_smart(header->getOverlay(ov_key).name());
	}
}

void PrintFormat::PKG_MAINREPO(OutputString *s, Package *package) const {
	if(package->have_main_repo_key()) {
		s->set_one();
	}
}

void PrintFormat::PKG_SYSTEM(OutputString *s, Package *package) const {
	if(package->is_system_package()) {
		s->set_one();
	}
}

void PrintFormat::PKG_PROFILE(OutputString *s, Package *package) const {
	if(package->is_profile_package()) {
		s->set_one();
	}
}

void PrintFormat::PKG_WORLD(OutputString *s, Package *package) const {
	if(package->is_world_package()) {
		s->set_one();
	}
}

void PrintFormat::PKG_WORLD_SETS(OutputString *s, Package *package) const {
	if(package->is_world_sets_package()) {
		s->set_one();
	}
}

void PrintFormat::PKG_SETNAMES(OutputString *s, Package *package) const {
	s->assign_smart(portagesettings->get_setnames(package));
}

void PrintFormat::PKG_ALLSETNAMES(OutputString *s, Package *package) const {
	s->assign_smart(portagesettings->get_setnames(package, true));
}

void PrintFormat::pkg_upgrade(OutputString *s, Package *package, bool only_installed, bool test_slots) const {
	LocalCopy localcopy(this, package);
	if(package->can_upgrade(vardb, portagesettings, only_installed, test_slots)) {
		s->set_one();
	}
	localcopy.restore(package);
}

void PrintFormat::PKG_UPGRADE(OutputString *s, Package *package) const {
	pkg_upgrade(s, package, true, true);
}

void PrintFormat::PKG_UPGRADEORINSTALL(OutputString *s, Package *package) const {
	pkg_upgrade(s, package, false, true);
}

void PrintFormat::PKG_BESTUPGRADE(OutputString *s, Package *package) const {
	pkg_upgrade(s, package, true, false);
}

void PrintFormat::PKG_BESTUPGRADEORINSTALL(OutputString *s, Package *package) const {
	pkg_upgrade(s, package, false, false);
}

void PrintFormat::pkg_downgrade(OutputString *s, Package *package, bool test_slots) const {
	LocalCopy locallocalcopy(this, package);
	if(package->must_downgrade(vardb, test_slots)) {
		s->set_one();
	}
	locallocalcopy.restore(package);
}

void PrintFormat::PKG_DOWNGRADE(OutputString *s, Package *package) const {
	pkg_downgrade(s, package, true);
}

void PrintFormat::PKG_BESTDOWNGRADE(OutputString *s, Package *package) const {
	pkg_downgrade(s, package, false);
}

void PrintFormat::pkg_recommend(OutputString *s, Package *package, bool only_installed, bool test_slots) const {
	LocalCopy locallocalcopy(this, package);
	if(package->recommend(vardb, portagesettings, only_installed, test_slots)) {
		s->set_one();
	}
	locallocalcopy.restore(package);
}

void PrintFormat::PKG_RECOMMEND(OutputString *s, Package *package) const {
	pkg_recommend(s, package, true, true);
}

void PrintFormat::PKG_RECOMMENDORINSTALL(OutputString *s, Package *package) const {
	pkg_recommend(s, package, false, true);
}

void PrintFormat::PKG_BESTRECOMMEND(OutputString *s, Package *package) const {
	pkg_recommend(s, package, true, false);
}

void PrintFormat::PKG_BESTRECOMMENDORINSTALL(OutputString *s, Package *package) const {
	pkg_recommend(s, package, false, false);
}

void PrintFormat::PKG_MARKED(OutputString *s, Package *package) const {
	if(unlikely(marked_list != NULLPTR) &&
		unlikely(marked_list->applyMasks(package))) {
		s->set_one();
	}
}

void PrintFormat::PKG_HAVEMARKEDVERSION(OutputString *s, Package *package) const {
	if(unlikely(marked_list != NULLPTR) &&
		unlikely(marked_list->MaskMatches(package))) {
		s->set_one();
	}
}

void PrintFormat::PKG_SLOTS(OutputString *s, Package *package) const {
	if((package->slotlist()).size() > 1) {
		s->set_one();
	}
}

void PrintFormat::PKG_SLOTTED(OutputString *s, Package *package) const {
	if(package->have_nontrivial_slots()) {
		s->set_one();
	}
}

void PrintFormat::PKG_HAVEVIRTUAL(OutputString *s, Package *package) const {
	if(have_virtual(package, false)) {
		s->set_one();
	}
}

void PrintFormat::PKG_HAVENONVIRTUAL(OutputString *s, Package *package) const {
	if(have_virtual(package, true)) {
		s->set_one();
	}
}

void PrintFormat::PKG_HAVECOLLIUSE(OutputString *s, Package *package) const {
	if(!(package->iuse.empty())) {
		s->set_one();
	}
}

void PrintFormat::PKG_COLLIUSE(OutputString *s, Package *package) const {
	iuse_expand(s, package->iuse, true, EXPAND_NO);
}

void PrintFormat::PKG_COLLIUSES(OutputString *s, Package *package) const {
	iuse_expand(s, package->iuse, true, EXPAND_YES);
}

void PrintFormat::PKG_COLLIUSE0(OutputString *s, Package *package) const {
	iuse_expand(s, package->iuse, true, EXPAND_OMIT);
}

const ExtendedVersion *PrintFormat::ver_version() const {
	if(version_variables->isinst) {
		return version_variables->instver();
	}
	return version_variables->version();
}

void PrintFormat::VER_FIRST(OutputString *s, Package * /* package */) const {
	if(version_variables->first) {
		s->set_one();
	}
}

void PrintFormat::VER_LAST(OutputString *s, Package * /* package */) const {
	if(version_variables->last) {
		s->set_one();
	}
}

void PrintFormat::VER_SLOTFIRST(OutputString *s, Package * /* package */) const {
	if(version_variables->slotfirst) {
		s->set_one();
	}
}

void PrintFormat::VER_SLOTLAST(OutputString *s, Package * /* package */) const {
	if(version_variables->slotlast) {
		s->set_one();
	}
}

void PrintFormat::VER_ONESLOT(OutputString *s, Package * /* package */) const {
	if(version_variables->oneslot) {
		s->set_one();
	}
}

const ExtendedVersion *PrintFormat::ver_versionslot(Package *package) const {
	if(version_variables->isinst) {
		InstVersion *i(version_variables->instver());
		package->guess_slotname(i, vardb, "?");
		return i;
	} else {
		return version_variables->version();
	}
}

void PrintFormat::VER_FULLSLOT(OutputString *s, Package *package) const {
	s->assign_smart(ver_versionslot(package)->get_longfullslot());
}

void PrintFormat::VER_ISFULLSLOT(OutputString *s, Package *package) const {
	if(!(ver_versionslot(package)->get_shortfullslot().empty())) {
		s->set_one();
	}
}

void PrintFormat::VER_SLOT(OutputString *s, Package *package) const {
	const string& slot(ver_versionslot(package)->slotname);
	if(likely(slot.empty())) {
		s->assign_fast('0');
	} else {
		s->assign_smart(slot);
	}
}

void PrintFormat::VER_ISSLOT(OutputString *s, Package *package) const {
	const string& slot(ver_versionslot(package)->slotname);
	if((!slot.empty()) && (slot != "0")) {
		s->set_one();
	}
}

void PrintFormat::VER_SUBSLOT(OutputString *s, Package *package) const {
	s->assign_smart(ver_versionslot(package)->subslotname);
}

void PrintFormat::VER_ISSUBSLOT(OutputString *s, Package *package) const {
	if(!(ver_versionslot(package)->subslotname.empty())) {
		s->set_one();
	}
}

void PrintFormat::VER_HAVESRCURI(OutputString *s, Package * /* package */) const {
	if(unlikely(version_variables->isinst)) {
		return;
	}
	if(!version_variables->version()->src_uri.empty()) {
		s->set_one();
	}
}

void PrintFormat::VER_SRCURI(OutputString *s, Package * /* package */) const {
	if(unlikely(version_variables->isinst)) {
		return;
	}
	s->assign_smart(version_variables->version()->src_uri);
}

void PrintFormat::VER_EAPI(OutputString *s, Package *package) const {
	if(version_variables->isinst) {
		InstVersion *i(version_variables->instver());
		vardb->readEapi(*package, i);
		s->assign_smart(i->eapi.get());
	} else {
		s->assign_smart(version_variables->version()->eapi.get());
	}
}

void PrintFormat::VER_VERSION(OutputString *s, Package * /* package */) const {
	if(version_variables->isinst) {
		s->assign_smart(version_variables->instver()->getFull());
	} else {
		s->assign_smart(version_variables->version()->getFull());
	}
}

void PrintFormat::VER_PLAINVERSION(OutputString *s, Package * /* package */) const {
	if(version_variables->isinst) {
		s->assign_smart(version_variables->instver()->getPlain());
	} else {
		s->assign_smart(version_variables->version()->getPlain());
	}
}

void PrintFormat::VER_REVISION(OutputString *s, Package * /* package */) const {
	if(version_variables->isinst) {
		s->assign_smart(version_variables->instver()->getRevision());
	} else {
		s->assign_smart(version_variables->version()->getRevision());
	}
}

void PrintFormat::ver_overlay(OutputString *s, Package *package, bool numeric, bool only_noncommon, bool only_nonzero) const {
	eix_assert_paranoic(!numeric || only_nonzero);
	ExtendedVersion::Overlay ov_key;
	if(version_variables->isinst) {
		InstVersion *i(version_variables->instver());
		eix_assert_paranoic(header != NULLPTR);
		if(unlikely(!(vardb->readOverlay(*package, i, *header)))) {
			if(unlikely(!numeric)) {
				s->assign_fast("?");
				return;
			}
			if(no_color || !only_noncommon) {
				s->assign_fast("[?]");
				return;
			}
			s->assign(color_overlaykey, 0);
			s->append_fast("[?]");
			s->append(color_keyend, 0);
			return;
		}
		ov_key = i->overlay_key;
		if(likely(only_noncommon) &&
			likely(package->have_same_overlay_key()) &&
			likely(ov_key == package->largest_overlay)) {
			return;
		}
	} else if(likely(only_noncommon) &&
		likely(package->have_same_overlay_key())) {
		return;
	} else {
		ov_key = version_variables->version()->overlay_key;
	}
	if(likely(only_nonzero) && likely(ov_key == 0)) {
		return;
	}
	if(likely(numeric)) {
		overlay_keytext(s, ov_key, !only_noncommon);
	} else {
		eix_assert_paranoic(header != NULLPTR);
		s->assign_smart(header->getOverlay(ov_key).name());
	}
}

void PrintFormat::VER_OVERLAYNUM(OutputString *s, Package *package) const {
	ver_overlay(s, package, true, false, true);
}

void PrintFormat::VER_OVERLAYVER(OutputString *s, Package *package) const {
	ver_overlay(s, package, true, true, true);
}

void PrintFormat::VER_OVERLAYPLAINNAMES(OutputString *s, Package *package) const {
	ver_overlay(s, package, false, false, false);
}

void PrintFormat::VER_OVERLAYPLAINNAME(OutputString *s, Package *package) const {
	ver_overlay(s, package, false, false, true);
}

void PrintFormat::VER_OVERLAYVERNAMES(OutputString *s, Package *package) const {
	ver_overlay(s, package, false, true, false);
}

void PrintFormat::VER_OVERLAYVERNAME(OutputString *s, Package *package) const {
	ver_overlay(s, package, false, true, true);
}

void PrintFormat::VER_VERSIONKEYWORDSS(OutputString *s, Package *package) const {
	if(unlikely(version_variables->isinst)) {
		return;
	}
	portagesettings->get_effective_keywords_userprofile(package);
	s->assign_smart(version_variables->version()->get_effective_keywords());
}

void PrintFormat::VER_VERSIONKEYWORDS(OutputString *s, Package * /* package */) const {
	if(unlikely(version_variables->isinst)) {
		return;
	}
	s->assign_smart(version_variables->version()->get_full_keywords());
}

void PrintFormat::VER_VERSIONEKEYWORDS(OutputString *s, Package *package) const {
	if(likely(!version_variables->isinst)) {
		portagesettings->get_effective_keywords_userprofile(package);
		const Version *v(version_variables->version());
		string t(v->get_effective_keywords());
		if(t != v->get_full_keywords()) {
			s->assign_smart(t);
		}
	}
}

void PrintFormat::ver_isbestupgrade(OutputString *s, Package *package, bool check_slots, bool allow_unstable) const {
	if(likely(!version_variables->isinst) &&
		unlikely(package->is_best_upgrade(check_slots,
				version_variables->version(),
				vardb, portagesettings, allow_unstable))) {
		s->set_one();
	}
}

void PrintFormat::VER_ISBESTUPGRADESLOT(OutputString *s, Package *package) const {
	ver_isbestupgrade(s, package, true, false);
}

void PrintFormat::VER_ISBESTUPGRADESLOTS(OutputString *s, Package *package) const {
	ver_isbestupgrade(s, package, true, true);
}

void PrintFormat::VER_ISBESTUPGRADE(OutputString *s, Package *package) const {
	ver_isbestupgrade(s, package, false, false);
}

void PrintFormat::VER_ISBESTUPGRADES(OutputString *s, Package *package) const {
	ver_isbestupgrade(s, package, false, true);
}

void PrintFormat::VER_MARKEDVERSION(OutputString *s, Package *package) const {
	if(likely(!version_variables->isinst)) {
		if(unlikely(marked_list != NULLPTR) &&
			likely(marked_list->applyMasks(package))) {
			if(version_variables->version()->maskflags.isMarked()) {
				s->set_one();
			}
		}
	}
}

void PrintFormat::VER_INSTALLEDVERSION(OutputString *s, Package *package) const {
	if(unlikely(version_variables->isinst)) {
		s->set_one();
		return;
	}
	eix_assert_paranoic(header != NULLPTR);
	if(vardb->isInstalledVersion(*package, version_variables->version(), *header)) {
		s->set_one();
	}
}

void PrintFormat::VER_HAVEUSE(OutputString *s, Package *package) const {
	if(version_variables->isinst) {
		InstVersion *i(version_variables->instver());
		if(likely(vardb->readUse(*package, i)) && !(i->inst_iuse.empty())) {
			s->set_one();
		}
		return;
	}
	if(!(version_variables->version()->iuse.empty())) {
		s->set_one();
	}
}

void PrintFormat::VER_USE(OutputString *s, Package *package) const {
	if(version_variables->isinst) {
		get_inst_use(s, *package, version_variables->instver(), EXPAND_NO);
	} else {
		iuse_expand(s, version_variables->version()->iuse, false, EXPAND_YES);
	}
}

void PrintFormat::VER_USES(OutputString *s, Package *package) const {
	if(version_variables->isinst) {
		get_inst_use(s, *package, version_variables->instver(), EXPAND_YES);
	} else {
		iuse_expand(s, version_variables->version()->iuse, false, EXPAND_YES);
	}
}

void PrintFormat::VER_USE0(OutputString *s, Package *package) const {
	if(version_variables->isinst) {
		get_inst_use(s, *package, version_variables->instver(), EXPAND_OMIT);
	} else {
		iuse_expand(s, version_variables->version()->iuse, false, EXPAND_OMIT);
	}
}

void PrintFormat::VER_REQUIREDUSE(OutputString *s, Package * /* package */) const {
	if(version_variables->isinst) {
		return;
	}
	if(Version::use_required_use) {
		s->assign_smart(version_variables->version()->required_use);
	}
}

void PrintFormat::VER_HAVEREQUIREDUSE(OutputString *s, Package * /* package */) const {
	if(version_variables->isinst) {
		return;
	}
	if(Version::use_required_use && !version_variables->version()->required_use.empty()) {
			s->set_one();
	}
}

void PrintFormat::VER_VIRTUAL(OutputString *s, Package *package) const {
	ExtendedVersion::Overlay key;
	if(version_variables->isinst) {
		InstVersion *i(version_variables->instver());
		eix_assert_paranoic(header != NULLPTR);
		if(!(vardb->readOverlay(*package, i, *header))) {
			return;
		}
		key = i->overlay_key;
	} else {
		key = version_variables->version()->overlay_key;
	}
	if(is_virtual(key)) {
		s->set_one();
	}
}

void PrintFormat::VER_ISBINARY(OutputString *s, Package *package) const {
	if(ver_version()->have_bin_pkg(portagesettings, package)) {
		s->set_one();
	}
}

void PrintFormat::VER_ISTBZ(OutputString *s, Package *package) const {
	if(ver_version()->have_tbz_pkg(portagesettings, package)) {
		s->set_one();
	}
}

void PrintFormat::VER_ISGPKG(OutputString *s, Package *package) const {
	if(ver_version()->num_gpkg_pkg(portagesettings, package) != 0) {
		s->set_one();
	}
}

void PrintFormat::VER_ISPAK(OutputString *s, Package *package) const {
	if(ver_version()->num_pak_pkg(portagesettings, package) != 0) {
		s->set_one();
	}
}

void PrintFormat::VER_ISMULTIGPKG(OutputString *s, Package *package) const {
	if(ver_version()->num_gpkg_pkg(portagesettings, package) > 1) {
		s->set_one();
	}
}

void PrintFormat::VER_ISMULTIPAK(OutputString *s, Package *package) const {
	if(ver_version()->num_pak_pkg(portagesettings, package) > 1) {
		s->set_one();
	}
}

void PrintFormat::VER_GPKGCOUNT(OutputString *s, Package *package) const {
	ExtendedVersion::CountBinPkg count(ver_version()->num_gpkg_pkg(portagesettings, package));
	if(count != 0) {
		s->assign_fast(eix::format() % count);
	}
}

void PrintFormat::VER_PAKCOUNT(OutputString *s, Package *package) const {
	ExtendedVersion::CountBinPkg count(ver_version()->num_pak_pkg(portagesettings, package));
	if(count != 0) {
		s->assign_fast(eix::format() % count);
	}
}

const ExtendedVersion *PrintFormat::ver_restrict(Package *package) const {
	if(version_variables->isinst) {
		InstVersion *i(version_variables->instver());
		eix_assert_paranoic(header != NULLPTR);
		vardb->readRestricted(*package, i, *header);
		return i;
	}
	return version_variables->version();
}

void PrintFormat::ver_restrict(OutputString *s, Package *package, ExtendedVersion::Restrict r) const {
	const ExtendedVersion *e(ver_restrict(package));
	if((e != NULLPTR) && (((e->restrictFlags) & r) != ExtendedVersion::RESTRICT_NONE)) {
		s->set_one();
	}
}

void PrintFormat::VER_RESTRICT(OutputString *s, Package *package) const {
	ver_restrict(s, package, ExtendedVersion::RESTRICT_ALL);
}

void PrintFormat::VER_RESTRICTFETCH(OutputString *s, Package *package) const {
	ver_restrict(s, package, ExtendedVersion::RESTRICT_FETCH);
}

void PrintFormat::VER_RESTRICTMIRROR(OutputString *s, Package *package) const {
	ver_restrict(s, package, ExtendedVersion::RESTRICT_MIRROR);
}

void PrintFormat::VER_RESTRICTPRIMARYURI(OutputString *s, Package *package) const {
	ver_restrict(s, package, ExtendedVersion::RESTRICT_PRIMARYURI);
}

void PrintFormat::VER_RESTRICTBINCHECKS(OutputString *s, Package *package) const {
	ver_restrict(s, package, ExtendedVersion::RESTRICT_BINCHECKS);
}

void PrintFormat::VER_RESTRICTSTRIP(OutputString *s, Package *package) const {
	ver_restrict(s, package, ExtendedVersion::RESTRICT_STRIP);
}

void PrintFormat::VER_RESTRICTTEST(OutputString *s, Package *package) const {
	ver_restrict(s, package, ExtendedVersion::RESTRICT_TEST);
}

void PrintFormat::VER_RESTRICTUSERPRIV(OutputString *s, Package *package) const {
	ver_restrict(s, package, ExtendedVersion::RESTRICT_USERPRIV);
}

void PrintFormat::VER_RESTRICTINSTALLSOURCES(OutputString *s, Package *package) const {
	ver_restrict(s, package, ExtendedVersion::RESTRICT_INSTALLSOURCES);
}

void PrintFormat::VER_RESTRICTBINDIST(OutputString *s, Package *package) const {
	ver_restrict(s, package, ExtendedVersion::RESTRICT_BINDIST);
}

void PrintFormat::VER_RESTRICTPARALLEL(OutputString *s, Package *package) const {
	ver_restrict(s, package, ExtendedVersion::RESTRICT_PARALLEL);
}

void PrintFormat::ver_properties(OutputString *s, Package *package, ExtendedVersion::Properties p) const {
	const ExtendedVersion *e(ver_restrict(package));
	if((e != NULLPTR) && (((e->propertiesFlags) & p) != ExtendedVersion::RESTRICT_NONE)) {
		s->set_one();
	}
}

void PrintFormat::VER_PROPERTIES(OutputString *s, Package *package) const {
	ver_properties(s, package, ExtendedVersion::PROPERTIES_ALL);
}

void PrintFormat::VER_PROPERTIESINTERACTIVE(OutputString *s, Package *package) const {
	ver_properties(s, package, ExtendedVersion::PROPERTIES_INTERACTIVE);
}

void PrintFormat::VER_PROPERTIESLIVE(OutputString *s, Package *package) const {
	ver_properties(s, package, ExtendedVersion::PROPERTIES_LIVE);
}

void PrintFormat::VER_PROPERTIESVIRTUAL(OutputString *s, Package *package) const {
	ver_properties(s, package, ExtendedVersion::PROPERTIES_VIRTUAL);
}

void PrintFormat::VER_PROPERTIESSET(OutputString *s, Package *package) const {
	ver_properties(s, package, ExtendedVersion::PROPERTIES_SET);
}

void PrintFormat::VER_HAVEDEPEND(OutputString *s, Package *package) const {
	if(version_variables->isinst) {
		eix_assert_paranoic(header != NULLPTR);
		vardb->readDepend(*package, version_variables->instver(), *header);
	}
	if(!ver_version()->depend.depend_empty()) {
		s->set_one();
	}
}

void PrintFormat::VER_HAVERDEPEND(OutputString *s, Package *package) const {
	if(version_variables->isinst) {
		eix_assert_paranoic(header != NULLPTR);
		vardb->readDepend(*package, version_variables->instver(), *header);
	}
	if(!ver_version()->depend.rdepend_empty()) {
		s->set_one();
	}
}

void PrintFormat::VER_HAVEPDEPEND(OutputString *s, Package *package) const {
	if(version_variables->isinst) {
		eix_assert_paranoic(header != NULLPTR);
		vardb->readDepend(*package, version_variables->instver(), *header);
	}
	if(!ver_version()->depend.pdepend_empty()) {
		s->set_one();
	}
}

void PrintFormat::VER_HAVEBDEPEND(OutputString *s, Package *package) const {
	if(version_variables->isinst) {
		eix_assert_paranoic(header != NULLPTR);
		vardb->readDepend(*package, version_variables->instver(), *header);
	}
	if(!ver_version()->depend.bdepend_empty()) {
		s->set_one();
	}
}

void PrintFormat::VER_HAVEIDEPEND(OutputString *s, Package *package) const {
	if(version_variables->isinst) {
		eix_assert_paranoic(header != NULLPTR);
		vardb->readDepend(*package, version_variables->instver(), *header);
	}
	if(!ver_version()->depend.idepend_empty()) {
		s->set_one();
	}
}

void PrintFormat::VER_HAVEDEPS(OutputString *s, Package *package) const {
	if(version_variables->isinst) {
		eix_assert_paranoic(header != NULLPTR);
		vardb->readDepend(*package, version_variables->instver(), *header);
	}
	if(!ver_version()->depend.empty()) {
		s->set_one();
	}
}

void PrintFormat::VER_DEPENDS(OutputString *s, Package *package) const {
	if(version_variables->isinst) {
		eix_assert_paranoic(header != NULLPTR);
		vardb->readDepend(*package, version_variables->instver(), *header);
	}
	s->assign_smart(ver_version()->depend.get_depend_brief());
}

void PrintFormat::VER_DEPEND(OutputString *s, Package *package) const {
	if(version_variables->isinst) {
		eix_assert_paranoic(header != NULLPTR);
		vardb->readDepend(*package, version_variables->instver(), *header);
	}
	s->assign_smart(ver_version()->depend.get_depend());
}

void PrintFormat::VER_RDEPENDS(OutputString *s, Package *package) const {
	if(version_variables->isinst) {
		eix_assert_paranoic(header != NULLPTR);
		vardb->readDepend(*package, version_variables->instver(), *header);
	}
	s->assign_smart(ver_version()->depend.get_rdepend_brief());
}

void PrintFormat::VER_RDEPEND(OutputString *s, Package *package) const {
	if(version_variables->isinst) {
		eix_assert_paranoic(header != NULLPTR);
		vardb->readDepend(*package, version_variables->instver(), *header);
	}
	s->assign_smart(ver_version()->depend.get_rdepend());
}

void PrintFormat::VER_PDEPENDS(OutputString *s, Package *package) const {
	if(version_variables->isinst) {
		eix_assert_paranoic(header != NULLPTR);
		vardb->readDepend(*package, version_variables->instver(), *header);
	}
	s->assign_smart(ver_version()->depend.get_pdepend_brief());
}

void PrintFormat::VER_PDEPEND(OutputString *s, Package *package) const {
	if(version_variables->isinst) {
		eix_assert_paranoic(header != NULLPTR);
		vardb->readDepend(*package, version_variables->instver(), *header);
	}
	s->assign_smart(ver_version()->depend.get_pdepend());
}

void PrintFormat::VER_BDEPENDS(OutputString *s, Package *package) const {
	if(version_variables->isinst) {
		eix_assert_paranoic(header != NULLPTR);
		vardb->readDepend(*package, version_variables->instver(), *header);
	}
	s->assign_smart(ver_version()->depend.get_bdepend_brief());
}

void PrintFormat::VER_BDEPEND(OutputString *s, Package *package) const {
	if(version_variables->isinst) {
		eix_assert_paranoic(header != NULLPTR);
		vardb->readDepend(*package, version_variables->instver(), *header);
	}
	s->assign_smart(ver_version()->depend.get_bdepend());
}

void PrintFormat::VER_IDEPENDS(OutputString *s, Package *package) const {
	if(version_variables->isinst) {
		eix_assert_paranoic(header != NULLPTR);
		vardb->readDepend(*package, version_variables->instver(), *header);
	}
	s->assign_smart(ver_version()->depend.get_idepend_brief());
}

void PrintFormat::VER_IDEPEND(OutputString *s, Package *package) const {
	if(version_variables->isinst) {
		eix_assert_paranoic(header != NULLPTR);
		vardb->readDepend(*package, version_variables->instver(), *header);
	}
	s->assign_smart(ver_version()->depend.get_idepend());
}

const MaskFlags *PrintFormat::ver_maskflags() const {
	return ((unlikely(version_variables->isinst)) ?
		NULLPTR : (&(version_variables->version()->maskflags)));
}

void PrintFormat::VER_ISHARDMASKED(OutputString *s, Package * /* package */) const {
	const MaskFlags *maskflags(ver_maskflags());
	if((maskflags != NULLPTR) && (maskflags->isHardMasked())) {
		s->set_one();
	}
}

void PrintFormat::VER_ISPROFILEMASKED(OutputString *s, Package * /* package */) const {
	const MaskFlags *maskflags(ver_maskflags());
	if((maskflags != NULLPTR) && (maskflags->isProfileMask())) {
		s->set_one();
	}
}

void PrintFormat::VER_ISMASKED(OutputString *s, Package * /* package */) const {
	const MaskFlags *maskflags(ver_maskflags());
	if((maskflags != NULLPTR) && (maskflags->isPackageMask())) {
		s->set_one();
	}
}

const KeywordsFlags *PrintFormat::ver_keywordsflags() const {
	return ((unlikely(version_variables->isinst)) ?
		NULLPTR : (&(version_variables->version()->keyflags)));
}

void PrintFormat::VER_ISSTABLE(OutputString *s, Package * /* package */) const {
	const KeywordsFlags *keywordsflags(ver_keywordsflags());
	if((keywordsflags != NULLPTR) && (keywordsflags->isStable())) {
		s->set_one();
	}
}

void PrintFormat::VER_ISUNSTABLE(OutputString *s, Package * /* package */) const {
	const KeywordsFlags *keywordsflags(ver_keywordsflags());
	if((keywordsflags != NULLPTR) && (keywordsflags->isUnstable())) {
		s->set_one();
	}
}

void PrintFormat::VER_ISALIENSTABLE(OutputString *s, Package * /* package */) const {
	const KeywordsFlags *keywordsflags(ver_keywordsflags());
	if((keywordsflags != NULLPTR) && (keywordsflags->isAlienStable())) {
		s->set_one();
	}
}

void PrintFormat::VER_ISALIENUNSTABLE(OutputString *s, Package * /* package */) const {
	const KeywordsFlags *keywordsflags(ver_keywordsflags());
	if((keywordsflags != NULLPTR) && (keywordsflags->isAlienUnstable())) {
		s->set_one();
	}
}

void PrintFormat::VER_ISMISSINGKEYWORD(OutputString *s, Package * /* package */) const {
	const KeywordsFlags *keywordsflags(ver_keywordsflags());
	if((keywordsflags != NULLPTR) && (keywordsflags->isMissingKeyword())) {
		s->set_one();
	}
}

void PrintFormat::VER_ISMINUSKEYWORD(OutputString *s, Package * /* package */) const {
	const KeywordsFlags *keywordsflags(ver_keywordsflags());
	if((keywordsflags != NULLPTR) && (keywordsflags->isMinusKeyword())) {
		s->set_one();
	}
}

void PrintFormat::VER_ISMINUSUNSTABLE(OutputString *s, Package * /* package */) const {
	const KeywordsFlags *keywordsflags(ver_keywordsflags());
	if((keywordsflags != NULLPTR) && (keywordsflags->isMinusUnstable())) {
		s->set_one();
	}
}

void PrintFormat::VER_ISMINUSASTERISK(OutputString *s, Package * /* package */) const {
	const KeywordsFlags *keywordsflags(ver_keywordsflags());
	if((keywordsflags != NULLPTR) && (keywordsflags->isMinusAsterisk())) {
		s->set_one();
	}
}

bool PrintFormat::ver_wasflags(Package *package, MaskFlags *maskflags, KeywordsFlags *keyflags) const {
	if(unlikely(version_variables->isinst)) {
		return false;
	}
	stability->calc_version_flags(false, maskflags, keyflags, version_variables->version(), package);
	return true;
}

void PrintFormat::VER_WASHARDMASKED(OutputString *s, Package *package) const {
	MaskFlags maskflags;
	if(ver_wasflags(package, &maskflags, NULLPTR) && maskflags.isHardMasked()) {
		s->set_one();
	}
}

void PrintFormat::VER_WASPROFILEMASKED(OutputString *s, Package *package) const {
	MaskFlags maskflags;
	if(ver_wasflags(package, &maskflags, NULLPTR) && maskflags.isProfileMask()) {
		s->set_one();
	}
}

void PrintFormat::VER_WASMASKED(OutputString *s, Package *package) const {
	MaskFlags maskflags;
	if(ver_wasflags(package, &maskflags, NULLPTR) && maskflags.isPackageMask()) {
		s->set_one();
	}
}

void PrintFormat::VER_WASSTABLE(OutputString *s, Package *package) const {
	KeywordsFlags keywordsflags;
	if(ver_wasflags(package, NULLPTR, &keywordsflags) && keywordsflags.isStable()) {
		s->set_one();
	}
}

void PrintFormat::VER_WASUNSTABLE(OutputString *s, Package *package) const {
	KeywordsFlags keywordsflags;
	if(ver_wasflags(package, NULLPTR, &keywordsflags) && keywordsflags.isUnstable()) {
		s->set_one();
	}
}

void PrintFormat::VER_WASALIENSTABLE(OutputString *s, Package *package) const {
	KeywordsFlags keywordsflags;
	if(ver_wasflags(package, NULLPTR, &keywordsflags) && keywordsflags.isAlienStable()) {
		s->set_one();
	}
}

void PrintFormat::VER_WASALIENUNSTABLE(OutputString *s, Package *package) const {
	KeywordsFlags keywordsflags;
	if(ver_wasflags(package, NULLPTR, &keywordsflags) && keywordsflags.isAlienUnstable()) {
		s->set_one();
	}
}

void PrintFormat::VER_WASMISSINGKEYWORD(OutputString *s, Package *package) const {
	KeywordsFlags keywordsflags;
	if(ver_wasflags(package, NULLPTR, &keywordsflags) && keywordsflags.isMissingKeyword()) {
		s->set_one();
	}
}

void PrintFormat::VER_WASMINUSKEYWORD(OutputString *s, Package *package) const {
	KeywordsFlags keywordsflags;
	if(ver_wasflags(package, NULLPTR, &keywordsflags) && keywordsflags.isMinusKeyword()) {
		s->set_one();
	}
}

void PrintFormat::VER_WASMINUSUNSTABLE(OutputString *s, Package *package) const {
	KeywordsFlags keywordsflags;
	if(ver_wasflags(package, NULLPTR, &keywordsflags) && keywordsflags.isMinusUnstable()) {
		s->set_one();
	}
}

void PrintFormat::VER_WASMINUSASTERISK(OutputString *s, Package *package) const {
	KeywordsFlags keywordsflags;
	if(ver_wasflags(package, NULLPTR, &keywordsflags) && keywordsflags.isMinusAsterisk()) {
		s->set_one();
	}
}

void PrintFormat::VER_HAVEMASKREASONS(OutputString *s, Package * /* package */) const {
	if(unlikely(version_variables->isinst)) {
		return;
	}
	if(version_variables->version()->have_reasons()) {
		s->set_one();
	}
}

void PrintFormat::ver_maskreasons(OutputString *s, const OutputString& skip, const OutputString& sep) const {
	if(unlikely(version_variables->isinst)) {
		return;
	}
	version_variables->version()->reasons_string(s, skip, sep);
}

void PrintFormat::VER_MASKREASONS(OutputString *s, Package * /* package */) const {
	ver_maskreasons(s, maskreasons_skip, maskreasons_sep);
}

void PrintFormat::VER_MASKREASONSS(OutputString *s, Package * /* package */) const {
	ver_maskreasons(s, maskreasonss_skip, maskreasonss_sep);
}

static Package *old_or_new(string *new_name, Package *older, Package *newer, const string& name) {
	const char *s(name.c_str());
	if(std::strncmp(s, "old", 3) == 0) {
		*new_name = s + 3;
		return older;
	}
	if(std::strncmp(s, "new", 3) == 0) {
		*new_name = s + 3;
		return newer;
	}
	*new_name = name;
	return newer;
}

void get_package_property(OutputString *s, const PrintFormat *fmt, void *entity, const string& name) {
	fmt->get_pkg_property(s, static_cast<Package *>(entity), name);
}

void get_diff_package_property(OutputString *s, const PrintFormat *fmt, void *entity, const string& name) {
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
		if(result) {
			s->set_one();
		}
		return;
	}
	string new_name;
	Package *package(old_or_new(&new_name, older, newer, name));
	fmt->get_pkg_property(s, package, new_name);
}
