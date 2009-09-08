// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Bob Shaffer II <bob.shaffer.2 at gmail.com>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include "print-xml.h"
#include <portage/vardbpkg.h>
#include <portage/set_stability.h>
#include <database/header.h>

using namespace std;

void
PrintXml::start()
{
	if(started)
		return;
	started = true;
	curcat.clear();
	count = 0;

	cout << "<?xml version='1.0' encoding='UTF-8'?>\n"
		"<eixdump>\n";
}

void
PrintXml::finish()
{
	if(!started)
		return;

	if (count) {
		cout << "\t</category>\n";
	}
	cout << "</eixdump>\n";

	clear();
}

void
PrintXml::package(const Package *pkg)
{
	if(!started)
		start();
	if (curcat != pkg->category) {
		if (!curcat.empty()) {
			cout << "\t</category>\n";
		}
		curcat = pkg->category;
		cout << "\t<category name=\"" << curcat << "\">\n";
	}
	// category, name, desc, homepage, licenses, provide;
	cout << "\t\t<package name=\"" << pkg->name << "\">\n";
	cout << "\t\t\t<description>" << escape_string(pkg->desc) << "</description>\n";
	cout << "\t\t\t<homepage>" << escape_string(pkg->homepage) << "</homepage>\n";

	set<const Version*> have_inst;
	if(var_db_pkg && var_db_pkg->isInstalled(*pkg)) {
		set<BasicVersion> know_inst;
		// First we check which versions are installed with correct overlays.
		if(hdr) {
			// Package is a 'list' of Versions with added members ^^
			for(Package::const_iterator ver = pkg->begin();
				ver != pkg->end(); ++ver) {
				if(var_db_pkg->isInstalledVersion(*pkg, *ver, *hdr, portdir.c_str()) > 0) {
					know_inst.insert(**ver);
					have_inst.insert(*ver);
				}
			}
		}
		// From the remaining ones we choose the last.
		for(Package::const_reverse_iterator ver = pkg->rbegin();
			ver != pkg->rend(); ++ver) {
			if(know_inst.find(**ver) == know_inst.end()) {
				know_inst.insert(**ver);
				have_inst.insert(*ver);
			}
		}
	}

	for (Package::const_iterator ver = pkg->begin(); ver != pkg->end(); ++ver) {
		bool versionInstalled = false;
		InstVersion *installedVersion = NULL;
		if(have_inst.find(*ver) != have_inst.end()) {
			if(var_db_pkg->isInstalled(*pkg, *ver, &installedVersion))
				versionInstalled = true;
		}

		cout << "\t\t\t<version id=\"" << ver->getFull() << "\"";
		Version::Overlay overlay_key = ver->overlay_key;
		const OverlayIdent &overlay = hdr->getOverlay(overlay_key);
		if(overlay_key && !overlay.path.empty()) {
			cout << " overlay=\"" << escape_string(overlay.path) << "\"";
		}
		if(!overlay.label.empty()) {
			cout << " repository=\"" << escape_string(overlay.label) << "\"";
		}
		if (versionInstalled) {
			cout << " installed=\"1\"";
		}
		if (!ver->slotname.empty()) {
			cout << " slot=\"" << escape_string(ver->slotname) << "\"";
		}
		if (versionInstalled) {
			cout << " installDate=\"" << installedVersion->instDate << "\"";
		}
		cout << ">\n";

		MaskFlags currmask(ver->maskflags);
		KeywordsFlags currkey(ver->keyflags);
		MaskFlags wasmask;
		KeywordsFlags waskey;
		stability->calc_version_flags(false, wasmask, waskey, *ver, pkg);

		//string full_kw = ver->get_full_keywords();
		//string eff_kw = ver->get_effective_keywords();
		const char *mask_text = NULL;
		const char *unmask_text = NULL;
		if(wasmask.isHardMasked()) {
			if(currmask.isProfileMask()) {
				mask_text = "profile";
			}
			else if(currmask.isPackageMask()) {
				mask_text = "hard";
			}
			else if(wasmask.isProfileMask()) {
				mask_text = "profile";
				unmask_text = "package_unmask";
			}
			else {
				mask_text = "hard";
				unmask_text = "package_unmask";
			}
		}
		else if(currmask.isHardMasked()) {
			mask_text = "package_mask";
		}

		if(currkey.isStable()) {
			if (waskey.isStable()) {
				//mask_text = NULL;
			}
			else if (waskey.isUnstable()) {
				mask_text = "keyword";
				unmask_text = "package_keywords";
			}
			else if (waskey.isMinusKeyword()) {
				mask_text = "minus_keyword";
				unmask_text = "package_keywords";
			}
			else if (waskey.isAlienStable()) {
				mask_text = "alien_stable";
				unmask_text = "package_keywords";
			}
			else if (waskey.isAlienUnstable()) {
				mask_text = "alien_unstable";
				unmask_text = "package_keywords";
			}
			else if (waskey.isMinusAsterisk()) {
				mask_text = "minus_asterisk";
				unmask_text = "package_keywords";
			}
			else {
				mask_text = "missing_keyword";
				unmask_text = "package_keywords";
			}
		}
		else if (currkey.isUnstable()) {
			mask_text = "keyword";
		}
		else if (currkey.isMinusKeyword()) {
			mask_text = "minus_keyword";
		}
		else if (currkey.isAlienStable()) {
			mask_text = "alien_stable";
		}
		else if (currkey.isAlienUnstable()) {
			mask_text = "alien_unstable";
		}
		else if (currkey.isMinusAsterisk()) {
			mask_text = "minus_asterisk";
		}
		//else {
		//	mask_text = "missing_keyword";
		//}

		if (mask_text) {
			cout << "\t\t\t\t<mask type=\"" << mask_text << "\" />\n";
		}
		if (unmask_text) {
			cout << "\t\t\t\t<unmask type=\"" << unmask_text << "\" />\n";
		}

		if (versionInstalled) {
			string iuse_disabled, iuse_enabled;
			var_db_pkg->readUse(*pkg, *installedVersion);
			vector<string> inst_iuse = installedVersion->inst_iuse;
			set<string> usedUse = installedVersion->usedUse;
			for (vector<string>::iterator iu = inst_iuse.begin(); iu != inst_iuse.end(); iu++) {
				if (usedUse.find(*iu) == usedUse.end()) {
					if (!iuse_disabled.empty()) iuse_disabled.append(" ");
					iuse_disabled.append(*iu);
				} else {
					if (!iuse_enabled.empty()) iuse_enabled.append(" ");
					iuse_enabled.append(*iu);
				}
			}
			if (!iuse_disabled.empty()) {
				cout << "\t\t\t\t<iuse enabled=\"0\">" << iuse_disabled << "</iuse>\n";
			}
			if (!iuse_enabled.empty()) {
				cout << "\t\t\t\t<iuse enabled=\"1\">" << iuse_enabled << "</iuse>\n";
			}
		} else {
			if (ver->iuse_vector().size()) {
				cout << "\t\t\t\t<iuse>" << ver->iuse() << "</iuse>\n";
			}
		}

		ExtendedVersion::Restrict restrict = ver->restrictFlags;
		if ( restrict != ExtendedVersion::RESTRICT_NONE) {
			if (restrict & ExtendedVersion::RESTRICT_BINCHECKS) {
				cout << "\t\t\t\t<restrict flag=\"binchecks\" />\n";
			}
			if (restrict & ExtendedVersion::RESTRICT_STRIP) {
				cout << "\t\t\t\t<restrict flag=\"strip\" />\n";
			}
			if (restrict & ExtendedVersion::RESTRICT_TEST) {
				cout << "\t\t\t\t<restrict flag=\"test\" />\n";
			}
			if (restrict & ExtendedVersion::RESTRICT_USERPRIV) {
				cout << "\t\t\t\t<restrict flag=\"userpriv\" />\n";
			}
			if (restrict & ExtendedVersion::RESTRICT_INSTALLSOURCES) {
				cout << "\t\t\t\t<restrict flag=\"installsources\" />\n";
			}
			if (restrict & ExtendedVersion::RESTRICT_FETCH) {
				cout << "\t\t\t\t<restrict flag=\"fetch\" />\n";
			}
			if (restrict & ExtendedVersion::RESTRICT_MIRROR) {
				cout << "\t\t\t\t<restrict flag=\"mirror\" />\n";
			}
			if (restrict & ExtendedVersion::RESTRICT_BINDIST) {
				cout << "\t\t\t\t<restrict flag=\"bindist\" />\n";
			}
		}
		ExtendedVersion::Restrict properties = ver->propertiesFlags;
		if(properties != ExtendedVersion::PROPERTIES_NONE) {
			if (properties & ExtendedVersion::PROPERTIES_INTERACTIVE) {
				cout << "\t\t\t\t<properties flag=\"interactive\" />\n";
			}
			if (properties & ExtendedVersion::PROPERTIES_LIVE) {
				cout << "\t\t\t\t<properties flag=\"live\" />\n";
			}
			if (properties & ExtendedVersion::PROPERTIES_VIRTUAL) {
				cout << "\t\t\t\t<properties flag=\"virtual\" />\n";
			}
			if (properties & ExtendedVersion::PROPERTIES_SET) {
				cout << "\t\t\t\t<properties flag=\"set\" />\n";
			}
		}

		//cout << "\t\t\t\t<full_keywords>" << full_kw << "</full_keywords>\n";
		//cout << "\t\t\t\t<effective_keywords>" << eff_kw << "</effective_keywords>\n";
		cout << "\t\t\t</version>\n";
	}
	cout << "\t\t</package>\n";
	++count;
}

string
PrintXml::escape_string(const string &s)
{
	string ret;
	string::size_type prev = 0;
	string::size_type len = s.length();
	for(string::size_type i = 0; i < len; ++i) {
		const char *replace;
		switch(s[i]) {
			case '&': replace = "&amp;"; break;
			case '<': replace = "&lt;"; break;
			case '>': replace = "&gt;"; break;
			case '\'': replace = "&apos;"; break;
			case '\"': replace = "&quot;"; break;
			default: replace = NULL; break;
		}
		if(replace) {
			ret.append(s, prev, i - prev);
			ret.append(replace);
			prev = i + 1;
		}
	}
	if(!prev)
		return s;
	if(prev < len)
		ret.append(s, prev, len - prev);
	return ret;
}
