// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Bob Shaffer II <bob.shaffer.2 at gmail.com>
//   Martin Väth <martin@mvath.de>

#include "output/print-xml.h"
#include <config.h>  // IWYU pragma: keep

#include <set>
#include <string>

#include "database/header.h"
#include "eixTk/dialect.h"
#include "eixTk/formated.h"
#include "eixTk/likely.h"
#include "eixTk/null.h"
#include "eixTk/stringtypes.h"
#include "eixTk/sysutils.h"
#include "eixTk/unordered_set.h"
#include "eixrc/eixrc.h"
#include "output/formatstring.h"
#include "portage/basicversion.h"
#include "portage/depend.h"
#include "portage/extendedversion.h"
#include "portage/instversion.h"
#include "portage/keywords.h"
#include "portage/overlay.h"
#include "portage/package.h"
#include "portage/set_stability.h"
#include "portage/vardbpkg.h"
#include "portage/version.h"

using std::set;
using std::string;

const PrintXml::XmlVersion PrintXml::current;

static void print_iuse(const IUseSet::IUseStd& s, IUse::Flags wanted, const char *dflt);

void PrintXml::runclear() {
	started = false;
	curcat.clear();
	count = 0;
}

void PrintXml::clear(EixRc *eixrc) {
	if(unlikely(eixrc == NULLPTR)) {
		print_overlay = false;
		keywords_mode = KW_NONE;
		dateformat = "%s";
	} else {
		dateformat = (*eixrc)["XML_DATE"];
		print_overlay = eixrc->getBool("XML_OVERLAY");
		static CONSTEXPR const char *values[] = {
			"none",
			"both",
			"effective*",
			"effective",
			"full*",
			"full",
			NULLPTR };
		switch(eixrc->getTinyTextlist("XML_KEYWORDS", values)) {
			case 0:
			case -1: keywords_mode = KW_NONE;  break;
			case -2: keywords_mode = KW_BOTH;  break;
			case -3: keywords_mode = KW_EFFS;  break;
			case -4: keywords_mode = KW_EFF;   break;
			case -5: keywords_mode = KW_FULLS; break;
			default: keywords_mode = KW_FULL;  break;
		}
	}
	runclear();
}

void PrintXml::start() {
	if(unlikely(started)) {
		return;
	}
	started = true;

	eix::say("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		"<eixdump version=\"%s\">") % current;
}

void PrintXml::finish() {
	if(!started) {
		return;
	}

	if(count) {
		eix::say("\t</category>");
	}
	eix::say("</eixdump>");

	runclear();
}

static void print_iuse(const IUseSet::IUseStd& s, IUse::Flags wanted, const char *dflt) {
	bool have_found(false);
	for(IUseSet::IUseStd::const_iterator it(s.begin()); likely(it != s.end()); ++it) {
		if(!((it->flags) & wanted)) {
			continue;
		}
		if(likely(have_found)) {
			eix::print(" %s") % PrintXml::escape_xmlstring(false, it->name());
			continue;
		}
		have_found = true;
		if(dflt != NULLPTR) {
			eix::print("\t\t\t\t<iuse default=\"%s\">") % dflt;
		} else {
			eix::print("\t\t\t\t<iuse>");
		}
		eix::print() % PrintXml::escape_xmlstring(false, it->name());
	}
	if(have_found) {
		eix::say("</iuse>");
	}
}

void PrintXml::package(Package *pkg) {
	if(unlikely(!started)) {
		start();
	}
	if(unlikely(curcat != pkg->category)) {
		if(!curcat.empty()) {
			eix::say("\t</category>");
		}
		curcat = pkg->category;
		eix::say("\t<category name=\"%s\">") % escape_xmlstring(true, curcat);
	}
	// category, name, desc, homepage, licenses;
	eix::say("\t\t<package name=\"%s\">")
		% escape_xmlstring(true, pkg->name);
	say_xml_element("\t\t\t", "description", pkg->desc);
	say_xml_element("\t\t\t", "homepage", pkg->homepage);
	say_xml_element("\t\t\t", "licenses", pkg->licenses);

	UNORDERED_SET<const Version*> have_inst;
	if((likely(var_db_pkg != NULLPTR)) && var_db_pkg->isInstalled(*pkg)) {
		set<BasicVersion> know_inst;
		// First we check which versions are installed with correct overlays.
		if(likely(hdr != NULLPTR)) {
			// Package is a 'list' of Versions with added members ^^
			for(Package::const_iterator ver(pkg->begin());
				likely(ver != pkg->end()); ++ver) {
				if(var_db_pkg->isInstalledVersion(*pkg, *ver, *hdr) > 0) {
					know_inst.INSERT(**ver);
					have_inst.INSERT(*ver);
				}
			}
		}
		// From the remaining ones we choose the last.
		// The following should actually be const_reverse_iterator,
		// but some compilers would then need a cast of rend(),
		// see https://bugs.gentoo.org/show_bug.cgi?id=354071
		for(Package::reverse_iterator ver(pkg->rbegin());
			likely(ver != pkg->rend()); ++ver) {
			if(know_inst.count(**ver) == 0) {
				know_inst.INSERT(**ver);
				have_inst.INSERT(*ver);
			}
		}
	}

	for(Package::const_iterator ver(pkg->begin()); likely(ver != pkg->end()); ++ver) {
		bool versionInstalled(false);
		InstVersion *installedVersion(NULLPTR);
		if(have_inst.count(*ver) != 0) {
			if(var_db_pkg->isInstalled(*pkg, *ver, &installedVersion)) {
				versionInstalled = true;
				var_db_pkg->readInstDate(*pkg, installedVersion);
				var_db_pkg->readEapi(*pkg, installedVersion);
			}
		}

		eix::print("\t\t\t<version id=\"%s\" EAPI=\"%s\"")
			% escape_xmlstring(true, ver->getFull())
			% escape_xmlstring(true, ver->eapi.get());
		ExtendedVersion::Overlay overlay_key(ver->overlay_key);
		if(unlikely(overlay_key != 0)) {
			if(print_format->is_virtual(overlay_key)) {
				eix::print(" virtual=\"1\"");
			}
			const OverlayIdent& overlay(hdr->getOverlay(overlay_key));
			if((print_overlay || overlay.label.empty()) && !(overlay.path.empty())) {
				eix::print(" overlay=\"%s\"")
					% escape_xmlstring(true, overlay.path);
			}
			if(!overlay.label.empty()) {
				eix::print(" repository=\"%s\"")
					% escape_xmlstring(true, overlay.label);
			}
		}
		if(!ver->get_shortfullslot().empty()) {
			eix::print(" slot=\"%s\"")
				% escape_xmlstring(true, ver->get_longfullslot());
		}
		if(versionInstalled) {
			eix::print(" installed=\"1\" installDate=\"%s\" installEAPI=\"%s\"")
				% escape_xmlstring(true, date_conv(dateformat.c_str(), installedVersion->instDate))
				% escape_xmlstring(false, installedVersion->eapi.get());
		}
		eix::say('>');

		MaskFlags currmask(ver->maskflags);
		KeywordsFlags currkey(ver->keyflags);
		MaskFlags wasmask;
		KeywordsFlags waskey;
		stability->calc_version_flags(false, &wasmask, &waskey, *ver, pkg);

		WordVec mask_text, unmask_text;
		// The following might give a memory leak with -flto for unknown reasons:
		// mask_text.PUSH_BACK("FOO") or mask_text.PUSH_BACK("BAR")
		if(wasmask.isHardMasked()) {
			if(currmask.isProfileMask()) {
				mask_text.push_back("profile");
			} else if(currmask.isPackageMask()) {
				mask_text.push_back("hard");
			} else if(wasmask.isProfileMask()) {
				mask_text.push_back("profile");
				unmask_text.push_back("package_unmask");
			} else {
				mask_text.push_back("hard");
				unmask_text.push_back("package_unmask");
			}
		} else if(currmask.isHardMasked()) {
			mask_text.push_back("package_mask");
		}

		if(currkey.isStable()) {
			if(waskey.isStable()) {
				//
			} else if(waskey.isUnstable()) {
				mask_text.push_back("keyword");
				unmask_text.push_back("package_keywords");
			} else if(waskey.isMinusKeyword()) {
				mask_text.push_back("minus_keyword");
				unmask_text.push_back("package_keywords");
			} else if(waskey.isAlienStable()) {
				mask_text.push_back("alien_stable");
				unmask_text.push_back("package_keywords");
			} else if(waskey.isAlienUnstable()) {
				mask_text.push_back("alien_unstable");
				unmask_text.push_back("package_keywords");
			} else if(waskey.isMinusUnstable()) {
				mask_text.push_back("minus_unstable");
				unmask_text.push_back("package_keywords");
			} else if(waskey.isMinusAsterisk()) {
				mask_text.push_back("minus_asterisk");
				unmask_text.push_back("package_keywords");
			} else {
				mask_text.push_back("missing_keyword");
				unmask_text.push_back("package_keywords");
			}
		} else if(currkey.isUnstable()) {
			mask_text.push_back("keyword");
		} else if(currkey.isMinusKeyword()) {
			mask_text.push_back("minus_keyword");
		} else if(currkey.isAlienStable()) {
			mask_text.push_back("alien_stable");
		} else if(currkey.isAlienUnstable()) {
			mask_text.push_back("alien_unstable");
		} else if(currkey.isMinusUnstable()) {
			mask_text.push_back("minus_unstable");
		} else if(currkey.isMinusAsterisk()) {
			mask_text.push_back("minus_asterisk");
		} else {
			mask_text.push_back("missing_keyword");
		}

		for(WordVec::const_iterator it(mask_text.begin());
			unlikely(it != mask_text.end()); ++it) {
			eix::say("\t\t\t\t<mask type=\"%s\"/>") % (*it);
		}

		if(unlikely(ver->have_reasons())) {
			const Version::Reasons *reasons_ptr(ver->reasons_ptr());
			for(Version::Reasons::const_iterator it(reasons_ptr->begin());
				unlikely(it != reasons_ptr->end()); ++it) {
				const WordVec *vec(it->asWordVecPtr());
				if((vec == NULLPTR) || (vec->empty())) {
					continue;
				}
				eix::print("\t\t\t\t<maskreason>");
				bool pret(false);
				for(WordVec::const_iterator wit(vec->begin());
					likely(wit != vec->end()); ++wit) {
					if(likely(pret)) {
						eix::say_empty();
					} else {
						pret = true;
					}
					eix::say() % escape_xmlstring(false, *wit);
				}
				eix::say("</maskreason>");
			}
		}

		for(WordVec::const_iterator it(unmask_text.begin());
			unlikely(it != unmask_text.end()); ++it) {
			eix::say("\t\t\t\t<unmask type=\"%s\"/>") % (*it);
		}

		if(!(ver->iuse.empty())) {
			const IUseSet::IUseStd& s(ver->iuse.asStd());
			print_iuse(s, IUse::USEFLAGS_NORMAL, NULLPTR);
			print_iuse(s, IUse::USEFLAGS_PLUS, "1");
			print_iuse(s, IUse::USEFLAGS_MINUS, "-1");
		}
		if(Version::use_required_use) {
			string &required_use(ver->required_use);
			if(!(required_use.empty())) {
				eix::say("\t\t\t\t<required_use>%s</required_use>")
					% escape_xmlstring(false, required_use);
			}
		}
		if(versionInstalled) {
			string iuse_disabled, iuse_enabled;
			var_db_pkg->readUse(*pkg, installedVersion);
			WordVec inst_iuse(installedVersion->inst_iuse);
			WordSet usedUse(installedVersion->usedUse);
			for(WordVec::const_iterator iu(inst_iuse.begin()); likely(iu != inst_iuse.end()); iu++) {
				if(usedUse.count(*iu) == 0) {
					if(!iuse_disabled.empty()) {
						iuse_disabled.append(1, ' ');
					}
					iuse_disabled.append(*iu);
				} else {
					if(!iuse_enabled.empty()) {
						iuse_enabled.append(1, ' ');
					}
					iuse_enabled.append(*iu);
				}
			}
			if(!iuse_disabled.empty()) {
				eix::say("\t\t\t\t<use enabled=\"0\">%s</use>")
					% escape_xmlstring(false, iuse_disabled);
			}
			if(!iuse_enabled.empty()) {
				eix::say("\t\t\t\t<use enabled=\"1\">%s</use>")
					% escape_xmlstring(false, iuse_enabled);
			}
		}

		ExtendedVersion::Restrict restrict(ver->restrictFlags);
		if(unlikely(restrict != ExtendedVersion::RESTRICT_NONE)) {
			if(unlikely(restrict & ExtendedVersion::RESTRICT_BINCHECKS)) {
				eix::say("\t\t\t\t<restrict flag=\"binchecks\"/>");
			}
			if(unlikely(restrict & ExtendedVersion::RESTRICT_STRIP)) {
				eix::say("\t\t\t\t<restrict flag=\"strip\"/>");
			}
			if(unlikely(restrict & ExtendedVersion::RESTRICT_TEST)) {
				eix::say("\t\t\t\t<restrict flag=\"test\"/>");
			}
			if(unlikely(restrict & ExtendedVersion::RESTRICT_USERPRIV)) {
				eix::say("\t\t\t\t<restrict flag=\"userpriv\"/>");
			}
			if(unlikely(restrict & ExtendedVersion::RESTRICT_INSTALLSOURCES)) {
				eix::say("\t\t\t\t<restrict flag=\"installsources\"/>");
			}
			if(unlikely(restrict & ExtendedVersion::RESTRICT_FETCH)) {
				eix::say("\t\t\t\t<restrict flag=\"fetch\"/>");
			}
			if(unlikely(restrict & ExtendedVersion::RESTRICT_MIRROR)) {
				eix::say("\t\t\t\t<restrict flag=\"mirror\"/>");
			}
			if(unlikely(restrict & ExtendedVersion::RESTRICT_PRIMARYURI)) {
				eix::say("\t\t\t\t<restrict flag=\"primaryuri\"/>");
			}
			if(unlikely(restrict & ExtendedVersion::RESTRICT_BINDIST)) {
				eix::say("\t\t\t\t<restrict flag=\"bindist\"/>");
			}
			if(unlikely(restrict & ExtendedVersion::RESTRICT_PARALLEL)) {
				eix::say("\t\t\t\t<restrict flag=\"parallel\"/>");
			}
		}
		ExtendedVersion::Restrict properties(ver->propertiesFlags);
		if(unlikely(properties != ExtendedVersion::PROPERTIES_NONE)) {
			if(unlikely(properties & ExtendedVersion::PROPERTIES_INTERACTIVE)) {
				eix::say("\t\t\t\t<properties flag=\"interactive\"/>");
			}
			if(unlikely(properties & ExtendedVersion::PROPERTIES_LIVE)) {
				eix::say("\t\t\t\t<properties flag=\"live\"/>");
			}
			if(unlikely(properties & ExtendedVersion::PROPERTIES_VIRTUAL)) {
				eix::say("\t\t\t\t<properties flag=\"virtual\"/>");
			}
			if(unlikely(properties & ExtendedVersion::PROPERTIES_SET)) {
				eix::say("\t\t\t\t<properties flag=\"set\"/>");
			}
		}

		if(keywords_mode != KW_NONE) {
			bool print_full(keywords_mode != KW_EFF);
			bool print_effective(keywords_mode != KW_FULL);
			string full_kw, eff_kw;
			if(print_full) {
				full_kw = ver->get_full_keywords();
			}
			if(print_effective) {
				eff_kw = ver->get_effective_keywords();
			}
			if((keywords_mode == KW_FULLS) || (keywords_mode == KW_EFFS)) {
				if(likely(full_kw == eff_kw)) {
					if(keywords_mode == KW_FULLS) {
						print_effective = false;
					} else {
						print_full = false;
					}
				}
			}
			if(print_full) {
				say_xml_element("\t\t\t\t", "keywords", full_kw);
			}
			if(print_effective) {
				say_xml_element("\t\t\t\t", "effective_keywords", eff_kw);
			}
		}

		if(Depend::use_depend) {
			const string& depend = ver->depend.get_depend();
			if(!depend.empty()) {
				eix::say("\t\t\t\t<depend>%s</depend>")
					% escape_xmlstring(false, depend);
			}
			const string& rdepend = ver->depend.get_rdepend();
			if(!rdepend.empty()) {
				eix::say("\t\t\t\t<rdepend>%s</rdepend>")
					% escape_xmlstring(false, rdepend);
			}
			const string& pdepend = ver->depend.get_pdepend();
			if(!pdepend.empty()) {
				eix::say("\t\t\t\t<pdepend>%s</pdepend>")
					% escape_xmlstring(false, pdepend);
			}
			const string& bdepend = ver->depend.get_bdepend();
			if(!bdepend.empty()) {
				eix::say("\t\t\t\t<bdepend>%s</bdepend>")
					% escape_xmlstring(false, bdepend);
			}
		}
		eix::say("\t\t\t</version>");
	}
	eix::say("\t\t</package>");
	++count;
}  // NOLINT(readability/fn_size)

string PrintXml::escape_xmlstring(bool quoted, const string& s) {
	string ret;
	string::size_type prev(0);
	string::size_type len(s.length());
	for(string::size_type i(0); likely(i < len); ++i) {
		const char *replace;
		switch(s[i]) {
			case '&':
				replace = "&amp;";
				break;
			case '<':
				replace = "&lt;";
				break;
			case '>':
				replace = "&gt;";
				break;
			case '\'':
				replace = (quoted ? "&apos;" : NULLPTR);
				break;
			case '\"':
				replace = (quoted ? "&quot;" : NULLPTR);
				break;
			default:
				replace = NULLPTR;
				break;
		}
		if(unlikely(replace != NULLPTR)) {
			ret.append(s, prev, i - prev);
			ret.append(replace);
			prev = i + 1;
		}
	}
	if(likely(prev == 0)) {
		return s;
	}
	if(prev < len) {
		ret.append(s, prev, len - prev);
	}
	return ret;
}

void PrintXml::say_xml_element(const string& prefix, const string& name, const string& content) {
	if(unlikely(content.empty())) {
		eix::say("%s<%s/>") % prefix % name;
		return;
	}
	eix::say("%s<%s>%s</%2$s>") % prefix % name
		% escape_xmlstring(false, content);
}
