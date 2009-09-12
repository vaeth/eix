// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>


#include "formatstring-print.h"
#include <eixrc/eixrc.h>
#include <eixTk/sysutils.h>
#include <portage/vardbpkg.h>
#include <portage/conf/portagesettings.h>
#include <output/formatstring.h>

using namespace std;

class VersionVariables {
	public:
		const Version *version;
		InstVersion *instver;
		bool first, last, slotfirst, slotlast, oneslot, isinst;
		string result;

		VersionVariables()
		{
			version = NULL;
			instver = NULL;
			first = last = slotfirst = slotlast = oneslot = true;
			isinst = false;
		}
};

string
PrintFormat::get_inst_use(const Package &package, InstVersion &i) const
{
	if((!vardb) || !(vardb->readUse(package, i)))
		return "";
	if(i.inst_iuse.empty())
		return "";
	string ret, add;
	for(vector<string>::iterator it = i.inst_iuse.begin();
		it != i.inst_iuse.end(); ++it) {
		bool is_unset = false;
		string *curr = &ret;
		if(i.usedUse.find(*it) == i.usedUse.end()) {
			is_unset = true;
			if(!alpha_use)
				curr = &add;
		}
		if(!curr->empty())
			curr->append(" ");
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
			ret.append(" ");
		ret.append(add);
	}
	return ret;
}

string
PrintFormat::get_version_stability(const Version *version, const Package *package) const
{
	bool need_color = !(no_color);

	MaskFlags currmask(version->maskflags);
	KeywordsFlags currkey(version->keyflags);
	MaskFlags wasmask;
	KeywordsFlags waskey;
	const string *colorp = NULL;
	string mask_text, keyword_text;
	stability->calc_version_flags(false, wasmask, waskey, version, package);
	if(wasmask.isHardMasked()) {
		if( need_color && color_original ) {
			need_color = false;
			colorp = &color_masked;
		}
		if(currmask.isProfileMask()) {
			if( need_color ) {
				need_color = false;
				colorp = &color_masked;
			}
			mask_text = tag_for_profile;
		}
		else if(currmask.isPackageMask()) {
			if( need_color ) {
				need_color = false;
				colorp = &color_masked;
			}
			mask_text = tag_for_masked;
		}
		else if(wasmask.isProfileMask()) {
			mask_text = tag_for_ex_profile;
		}
		else {
			mask_text = tag_for_ex_masked;
		}
	}
	else if(currmask.isHardMasked()) {
		if( need_color && color_local_mask ) {
			need_color = false;
			colorp = &color_masked;
		}
		mask_text = tag_for_locally_masked;
	}

	if(currkey.isStable()) {
		if( need_color && !(color_original) ) {
			need_color = false;
			colorp = &color_stable;
		}
		if (waskey.isStable()) {
			if( need_color )
				colorp = &color_stable;
			keyword_text = tag_for_stable;
		}
		else if (waskey.isUnstable()) {
			if( need_color )
				colorp = &color_unstable;
			keyword_text = tag_for_ex_unstable;
		}
		else if (waskey.isMinusKeyword()) {
			if( need_color )
				colorp = &color_masked;
			keyword_text = tag_for_ex_minus_keyword;
		}
		else if (waskey.isAlienStable()) {
			if( need_color )
				colorp = &color_masked;
			keyword_text = tag_for_ex_alien_stable;
		}
		else if (waskey.isAlienUnstable()) {
			if( need_color )
				colorp = &color_masked;
			keyword_text = tag_for_ex_alien_unstable;
		}
		else if (waskey.isMinusAsterisk()) {
			if( need_color )
				colorp = &color_masked;
			keyword_text = tag_for_ex_minus_asterisk;
		}
		else {
			if( need_color )
				colorp = &color_masked;
			keyword_text = tag_for_ex_missing_keyword;
		}
	}
	else if (currkey.isUnstable()) {
		if( need_color )
			colorp = &color_unstable;
		keyword_text = tag_for_unstable;
	}
	else if (currkey.isMinusKeyword()) {
		if( need_color )
			colorp = &color_masked;
		keyword_text = tag_for_minus_keyword;
	}
	else if (currkey.isAlienStable()) {
		if( need_color )
			colorp = &color_masked;
		keyword_text = tag_for_alien_stable;
	}
	else if (currkey.isAlienUnstable()) {
		if( need_color )
			colorp = &color_masked;
		keyword_text = tag_for_alien_unstable;
	}
	else if (currkey.isMinusAsterisk()) {
		if( need_color )
			colorp = &color_masked;
		keyword_text = tag_for_minus_asterisk;
	}
	else {
		if( need_color )
			colorp = &color_masked;
		keyword_text = tag_for_missing_keyword;
	}
	mask_text.append(keyword_text);
	if(colorp)
		return (*colorp) + mask_text;
	return mask_text;
}

string
PrintFormat::get_marked_version(const Version *version, const Package *package, bool midslot) const
{
	string ret;;
	bool is_installed = false;
	bool is_marked = false;
	bool is_upgrade = false;
	if(!no_color) {
		if(vardb) {
			is_installed = vardb->isInstalledVersion(*package, version, *header, (*portagesettings)["PORTDIR"].c_str());
			if(!is_installed)
				is_upgrade = package->is_best_upgrade(true,
					version, vardb, portagesettings, false);
		}
		if(marked_list)
			is_marked = marked_list->is_marked(*package, version);
	}
	if (is_installed)
		ret = mark_installed;
	else if (is_upgrade)
		ret = mark_upgrade;
	if (is_marked)
		ret.append(mark_version);
	if(midslot)
		ret.append(version->getFullSlotted(colon_slots));
	else
		ret.append(version->getFull());
	if (is_marked) {
		ret.append(mark_version_end);
		if(is_installed &&
			(mark_version_end != mark_installed_end))
			ret.append(mark_installed_end);
		else if(is_upgrade &&
			(mark_version_end != mark_upgrade_end))
			ret.append(mark_upgrade_end);
	}
	else if (is_installed)
		ret.append(mark_installed_end);
	else if(is_upgrade)
		ret.append(mark_upgrade_end);
	return ret;
}

string
PrintFormat::get_properties(const ExtendedVersion *version) const
{
	ExtendedVersion::Restrict properties = version->propertiesFlags;
	if(properties == ExtendedVersion::PROPERTIES_NONE)
		return "";
	string result;
	if(properties & ExtendedVersion::PROPERTIES_INTERACTIVE) {
		if(! no_color)
			result = color_properties_interactive;
		result.append(tag_properties_interactive);
	}
	if(properties & ExtendedVersion::PROPERTIES_LIVE) {
		if(! no_color)
			result.append(color_properties_live);
		result.append(tag_properties_live);
	}
	if(properties & ExtendedVersion::PROPERTIES_VIRTUAL) {
		if(! no_color)
			result.append(color_properties_virtual);
		result.append(tag_properties_virtual);
	}
	if(properties & ExtendedVersion::PROPERTIES_SET) {
		if(! no_color)
			result.append(color_properties_set);
		result.append(tag_properties_set);
	}
	return result;
}

string
PrintFormat::get_restrictions(const ExtendedVersion *version) const
{
	ExtendedVersion::Restrict restrict = version->restrictFlags;
	if(restrict == ExtendedVersion::RESTRICT_NONE)
		return "";
	string result;
	if(restrict & ExtendedVersion::RESTRICT_FETCH) {
		if(! no_color)
			result = color_restrict_fetch;
		result.append(tag_restrict_fetch);
	}
	if(restrict & ExtendedVersion::RESTRICT_MIRROR) {
		if(! no_color)
			result.append(color_restrict_mirror);
		result.append(tag_restrict_mirror);
	}
	if(restrict & ExtendedVersion::RESTRICT_PRIMARYURI) {
		if(! no_color)
			result.append(color_restrict_primaryuri);
		result.append(tag_restrict_primaryuri);
	}
	if(restrict & ExtendedVersion::RESTRICT_BINCHECKS) {
		if(! no_color)
			result.append(color_restrict_binchecks);
		result.append(tag_restrict_binchecks);
	}
	if(restrict & ExtendedVersion::RESTRICT_STRIP) {
		if(! no_color)
			result.append(color_restrict_strip);
		result.append(tag_restrict_strip);
	}
	if(restrict & ExtendedVersion::RESTRICT_TEST) {
		if(! no_color)
			result.append(color_restrict_test);
		result.append(tag_restrict_test);
	}
	if(restrict & ExtendedVersion::RESTRICT_USERPRIV) {
		if(! no_color)
			result.append(color_restrict_userpriv);
		result.append(tag_restrict_userpriv);
	}
	if(restrict & ExtendedVersion::RESTRICT_INSTALLSOURCES) {
		if(! no_color)
			result.append(color_restrict_installsources);
		result.append(tag_restrict_installsources);
	}
	if(restrict & ExtendedVersion::RESTRICT_BINDIST) {
		if(! no_color)
			result.append(color_restrict_bindist);
		result.append(tag_restrict_bindist);
	}
	return result;
}

string
PrintFormat::get_version_keywords(const Package *package, const Version *version) const
{
	if(print_effective)
		portagesettings->get_effective_keywords_userprofile(const_cast<Package *>(package));
	string keywords = version->get_full_keywords();
	string effective = version->get_effective_keywords();
	if(keywords.empty()) {
		if(effective.empty() || !print_effective)
			return "";
	}
	string result = before_keywords;
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
PrintFormat::get_installed(const Package *package, Node *root, bool only_marked) const
{
	if(!vardb)
		return;
	if(only_marked && (!marked_list))
		return;
	vector<InstVersion> *vec = vardb->getInstalledVector(*package);
	if(!vec)
		return;
	bool have_prevversion = false;
	for(vector<InstVersion>::iterator it = vec->begin();
		it != vec->end(); ++it) {
		if(only_marked) {
			if(!(marked_list->is_marked(*package, &(*it))))
				continue;
		}
		if(have_prevversion) {
			version_variables->last = version_variables->slotlast = false;
			recPrint(&(version_variables->result), package, &get_package_property, root);
			version_variables->first = version_variables->slotfirst = false;
		}
		have_prevversion = true;
		version_variables->instver = &(*it);
	}
	if(have_prevversion) {
		version_variables->last = version_variables->slotlast = true;
		recPrint(&(version_variables->result), package, &get_package_property, root);
	}
}

void
PrintFormat::get_versions_versorted(const Package *package, Node *root, vector<Version*> *versions) const
{
	bool have_prevversion = false;
	for(Package::const_iterator vit = package->begin(); vit != package->end(); ++vit) {
		if(versions) {
			if(find(versions->begin(), versions->end(), *vit) == versions->end())
				continue;
		}
		if(have_prevversion) {
			version_variables->last = version_variables->slotlast = false;
			recPrint(&(version_variables->result), package, &get_package_property, root);
			version_variables->first = version_variables->slotfirst = false;
		}
		have_prevversion = true;
		version_variables->version = *vit;
	}
	if(have_prevversion) {
		version_variables->last = version_variables->slotlast = true;
		recPrint(&(version_variables->result), package, &get_package_property, root);
	}
}

void
PrintFormat::get_versions_slotsorted(const Package *package, Node *root, vector<Version*> *versions) const
{
	const SlotList *sl = &(package->slotlist());
	SlotList::size_type slotnum = 0;
	if(versions) {
		for(SlotList::const_iterator it = sl->begin();
			it != sl->end(); ++it) {
			const VersionList *vl = &(it->const_version_list());
			for(VersionList::const_iterator vit = vl->begin();
				vit != vl->end(); ++vit) {
				if(find(versions->begin(), versions->end(), *vit) != versions->end()) {
					++slotnum;
					break;
				}
			}
		}
	}
	else
		slotnum = sl->size();
	if(!slotnum)
		return;
	version_variables->oneslot = (slotnum == 1);

	bool have_prevversion = false;
	SlotList::size_type prevslot = slotnum + 1;
	version_variables->slotfirst = true;
	for(SlotList::const_iterator it = sl->begin(); slotnum; ++it, --slotnum) {
		const VersionList *vl = &(it->const_version_list());
		for(VersionList::const_iterator vit = vl->begin();
			vit != vl->end(); ++vit) {
			if(versions) {
				if(find(versions->begin(), versions->end(), *vit) == versions->end())
					continue;
			}
			if(have_prevversion) {
				version_variables->last = false;
				version_variables->slotlast = (prevslot != slotnum);
				recPrint(&(version_variables->result), package, &get_package_property, root);
				version_variables->first = false;
			}
			have_prevversion = true;
			version_variables->version = *vit;
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

string
PrintFormat::get_pkg_property(const Package *package, const string &name) const throw(ExBasic)
{
	if(version_variables) {
		if(name == "first") {
			if(version_variables->first)
				return "1";
			return "";
		}
		if(name == "last") {
			if(version_variables->last)
				return "1";
			return "";
		}
		if(name == "slotfirst") {
			if(version_variables->slotfirst)
				return "1";
			return "";
		}
		if(name == "slotlast") {
			if(version_variables->slotlast)
				return "1";
			return "";
		}
		if(name == "oneslot") {
			if(version_variables->oneslot)
				return "1";
			return "";
		}
		if((name == "slot") || (name == "isslot")) {
			const string *slot;
			if(version_variables->isinst) {
				InstVersion *i = version_variables->instver;
				if((!vardb) || !(package->guess_slotname(*i, vardb)))
					i->slotname = "?";
				slot = &(i->slotname);
			}
			else
				slot = &(version_variables->version->slotname);
			if(name.size() != 4) {
				if(slot->empty() || (*slot == "0"))
					return "";
				return "1";
			}
			if(slot->empty())
				return "0";
			return *slot;
		}
		if(name == "stability") {
			if(version_variables->isinst)
				return "";
			return get_version_stability(version_variables->version, package);
		}
		if(name == "version") {
			if(version_variables->isinst)
				return version_variables->instver->getFull();
			return get_marked_version(version_variables->version, package, false);
		}
		if(name == "version*") {
			if(version_variables->isinst)
				return version_variables->instver->getFullSlotted(colon_slots);
			return get_marked_version(version_variables->version, package, true);
		}
		if(name == "plainversion") {
			if(version_variables->isinst)
				return version_variables->instver->getFull();
			return version_variables->version->getFull();
		}
		if(name == "plainversion*") {
			if(version_variables->isinst)
				return version_variables->instver->getFullSlotted(colon_slots);
			return version_variables->instver->getFullSlotted(colon_slots);
		}
		if(name == "properties") {
			if(version_variables->isinst) {
				if((!vardb) || (!header))
					return "";
				vardb->readRestricted(*package, *(version_variables->instver), *header, (*portagesettings)["PORTDIR"].c_str());
				return get_properties(version_variables->instver);
			}
			return get_properties(version_variables->version);
		}
		if(name == "restrictions") {
			if(version_variables->isinst) {
				if((!vardb) || (!header))
					return "";
				vardb->readRestricted(*package, *(version_variables->instver), *header, (*portagesettings)["PORTDIR"].c_str());
				return get_restrictions(version_variables->instver);
			}
			return get_restrictions(version_variables->version);
		}
		if(name == "overlayver") {
			if(version_variables->isinst) {
				InstVersion *i = version_variables->instver;
				if((!vardb) || (!header) || !(vardb->readOverlay(*package, *i, *header, (*portagesettings)["PORTDIR"].c_str()))) {
					if(no_color)
						return "[?]";
					return color_overlaykey + "[?]" +
						AnsiColor(AnsiColor::acDefault).asString();
				}
				if(i->overlay_key > 0) {
					if((!package->have_same_overlay_key()) || (package->largest_overlay != i->overlay_key))
						return overlay_keytext(i->overlay_key);
				}
				return "";
			}
			if(!package->have_same_overlay_key()) {
				if(version_variables->version->overlay_key)
					return overlay_keytext(version_variables->version->overlay_key);
			}
			return "";
		}
		if(name == "versionkeywords") {
			if(version_variables->isinst)
				return "";
			return get_version_keywords(package, version_variables->version);
		}
		if(name == "haveuse") {
			if(version_variables->isinst) {
				InstVersion &i = *(version_variables->instver);
				if(vardb && (vardb->readUse(*package, i)) && !(i.inst_iuse.empty()))
					return "1";
				return "";
			}
			if(version_variables->version->iuse_vector().empty())
				return "";
			return "1";
		}
		if((name == "isbestupgrade") || (name == "isbestupgrade*") ||
			(name == "isbestupgradeslot") || (name == "isbestupgradeslot*")) {
			if(version_variables->isinst)
				return "";
			if(vardb && portagesettings &&
				package->is_best_upgrade(
					(name.find("slot") != string::npos),
					version_variables->version,
					vardb, portagesettings,
					(name.find('*') != string::npos)))
				return "1";
			return "";
		}
		if(name == "use") {
			if(version_variables->isinst)
				return get_inst_use(*package, *(version_variables->instver));
			return version_variables->version->iuse();
		}
		if(strncmp(name.c_str(), "date:", 5) == 0) {
			if(version_variables->isinst) {
				return date_conv((*eix_rc)[name.substr(5)].c_str(),
					version_variables->instver->instDate);
			}
			return "";
		}
		if((name.find("stable") != string::npos) ||
			(name.find("mask") != string::npos) ||
			(name.find("keyword") != string::npos) ||
			(name.find("asterisk") != string::npos)) {
			if(version_variables->isinst)
				return "";
			const Version *version = version_variables->version;
			MaskFlags mymask(version->maskflags);
			KeywordsFlags mykey(version->keyflags);
			if(name.find("was") != string::npos) {
				stability->calc_version_flags(false, mymask, mykey,
					version, package);
			}
			if(name.find("mask") != string::npos) {
				if(name.find("hard") != string::npos) {
					if(mymask.isHardMasked())
						return "1";
					return "";
				}
				if(name.find("profile") != string::npos) {
					if(mymask.isProfileMask())
						return "1";
					return "";
				}
				if(mymask.isPackageMask())
					return "1";
				return "";
			}
			if(name.find("alien") != string::npos) {
				if(name.find("unstable") != string::npos) {
					if(mykey.isAlienUnstable())
						return "1";
					return "";
				}
				if(mykey.isAlienStable())
					return "1";
				return "";
			}
			if(name.find("unstable") != string::npos) {
				if(mykey.isUnstable())
					return "1";
				return "";
			}
			if(name.find("stable") != string::npos) {
				if(mykey.isStable())
					return "1";
				return "";
			}
			if(name.find("keyword") != string::npos) {
				if(mykey.isMinusKeyword())
					return "1";
				return "";
			}
			if(mykey.isMinusAsterisk())
				return "1";
			return "";
		}
		if(name == "markedversion") {
			if(version_variables->isinst)
				return "";
			if(marked_list && marked_list->is_marked(*package,
				version_variables->version))
				return "1";
			return "";
		}
		if(name == "installedversion") {
			if(version_variables->isinst)
				return "1";
			if(vardb && header && vardb->isInstalledVersion(*package,
				version_variables->version,
				*header, (*portagesettings)["PORTDIR"].c_str()))
				return "1";
			return "";
		}
		if(name.find("restrict") != string::npos) {
			ExtendedVersion::Restrict restrict;
			if(version_variables->isinst)
				restrict = version_variables->instver->restrictFlags;
			else
				restrict = version_variables->version->restrictFlags;
			if(name.find("fetch") != string::npos) {
				if(restrict & ExtendedVersion::RESTRICT_FETCH)
					return "1";
			}
			else if(name.find("mirror") != string::npos) {
				if(restrict & ExtendedVersion::RESTRICT_MIRROR)
					return "1";
			}
			else if(name.find("primary") != string::npos) {
				if(restrict & ExtendedVersion::RESTRICT_PRIMARYURI)
					return "1";
			}
			else if(name.find("bincheck") != string::npos) {
				if(restrict & ExtendedVersion::RESTRICT_BINCHECKS)
					return "1";
			}
			else if(name.find("strip") != string::npos) {
				if(restrict & ExtendedVersion::RESTRICT_STRIP)
					return "1";
			}
			else if(name.find("test") != string::npos) {
				if(restrict & ExtendedVersion::RESTRICT_TEST)
					return "1";
			}
			else if(name.find("user") != string::npos) {
				if(restrict & ExtendedVersion::RESTRICT_USERPRIV)
					return "1";
			}
			else if(name.find("install") != string::npos) {
				if(restrict & ExtendedVersion::RESTRICT_INSTALLSOURCES)
					return "1";
			}
			else if(name.find("bindist") != string::npos) {
				if(restrict & ExtendedVersion::RESTRICT_BINDIST)
					return "1";
			}
			else if(restrict != ExtendedVersion::RESTRICT_NONE)
				return "1";
			return "";
		}
		if(name.find("proper") != string::npos) {
			ExtendedVersion::Properties properties;
			if(version_variables->isinst)
				properties = version_variables->instver->propertiesFlags;
			else
				properties = version_variables->version->propertiesFlags;
			if(name.find("inter") != string::npos) {
				if(properties & ExtendedVersion::PROPERTIES_INTERACTIVE)
					return "1";
			}
			else if(name.find("live") != string::npos) {
				if(properties & ExtendedVersion::PROPERTIES_LIVE)
					return "1";
			}
			else if(name.find("virtual") != string::npos) {
				if(properties & ExtendedVersion::PROPERTIES_VIRTUAL)
					return "1";
			}
			else if(name.find("set") != string::npos) {
				if(properties & ExtendedVersion::PROPERTIES_SET)
					return "1";
			}
			else if(properties != ExtendedVersion::PROPERTIES_NONE)
				return "1";
			return "";
		}
	}
	string::size_type col = name.find(':');
	if((col != string::npos) && (col > 2) && (col < name.length() - 1)) {
		// <availableversions:VAR[:VAR]>, <installedversions:VAR>, ...
		string plainname = name.substr(0, col);
		string varname = name.substr(col + 1);
		string varsortname;
		string *parsed = NULL;
		col = varname.find(':');
		if(col != string::npos) {
			varsortname = varname.substr(col + 1);
			varname.erase(col);
		}
		VersionVariables variables;
		VersionVariables *previous_variables = version_variables;
		version_variables = &variables;
		if(plainname.find("best") != string::npos) {
			// <bestversionslot:VAR>, ...
			bool accept_unstable = (plainname.find_first_of('*') != string::npos);
			if(plainname.find("slot") != string::npos) {
				vector<Version*> versions;
				if(plainname.find("upgrade") != string::npos)
					package->best_slots_upgrade(versions, vardb, portagesettings, accept_unstable);
				else
					package->best_slots(versions, accept_unstable);
				if(!versions.empty()) {
					parsed = &varname;
					get_versions_versorted(package, parse_variable(varname), &versions);
				}
			}
			else {
				// <bestversion:VAR>
				variables.version = package->best(accept_unstable);
				if(variables.version) {
					parsed = &varname;
					recPrint(&(variables.result), package, get_package_property, parse_variable(varname));
				}
			}
		}
		else {
			bool marked = (plainname.find("mark") != string::npos);
			if(plainname.find("install") != string::npos) {
				// <installedversions:VAR>, ...
				variables.isinst = true;
				parsed = &varname;
				get_installed(package, parse_variable(varname), marked);
			}
			else {
				// <{available,marked}versions:VAR[:VAR]>, ...
				vector<Version*> *versions = NULL;
				if(marked) {
					versions = new vector<Version*>;
					for(Package::const_iterator it = package->begin();
						it != package->end(); ++it) {
						if(marked_list->is_marked(*package, &(**it))) {
							versions->push_back(*it);
						}
					}
				}
				if((!versions) || !(versions->empty())) {
					if(varsortname.empty() || !(package->have_nontrivial_slots())) {
						parsed = &varname;
						get_versions_versorted(package, parse_variable(varname), versions);
					}
					else {
						parsed = &varsortname;
						get_versions_slotsorted(package, parse_variable(varsortname), versions);
					}
				}
				if(versions)
					delete versions;
			}
		}
		if(parsed)
			varcache[*parsed].in_use = false;
		version_variables = previous_variables;
		return variables.result;
	}
	if(name == "installed") {
		if(vardb) {
			vector<InstVersion> *vec = vardb->getInstalledVector(*package);
			if(vec && !(vec->empty()))
				return "1";
		}
		return "";
	}
	if(name == "versionlines") {
		if(style_version_lines)
			return "1";
		return "";
	}
	if(name == "slotsorted") {
		if(slot_sorted)
			return "1";
		return "";
	}
	if((name == "havebest") || (name == "havebest*")) {
		if(package->best(name.find_first_of('*') != string::npos))
			return "1";
		return "";
	}
	if(name == "category")
		return package->category;
	if(name == "name")
		return package->name;
	if(name == "description")
		return package->desc;
	if(name == "homepage")
		return package->homepage;
	if(name.find("license") != string::npos)
		return package->licenses;
	if(name == "provide")
		return package->provide;
	if(name.find("overlay") != string::npos) {
		Version::Overlay ov_key = package->largest_overlay;
		if(ov_key && package->have_same_overlay_key()) {
			return overlay_keytext(ov_key, false);
		}
		return "";
	}
	if(name == "system") {
		if(package->is_system_package())
			return "1";
		return "";
	}
	if(name.find("world") != string::npos) {
		if(name.find("set") != string::npos) {
			if(package->is_world_sets_package())
				return "1";
		}
		else if(package->is_world_package())
			return "1";
		return "";
	}
	if(name.find("set") != string::npos) {
		if(name.find("all") != string::npos)
			return portagesettings->get_setnames(package, true);
		return portagesettings->get_setnames(package);
	}
	if((name.find("upgrade") != string::npos) ||
		(name.find("update") != string::npos)) {
		LocalCopy copy(this, const_cast<Package*>(package));
		bool result = package->can_upgrade(vardb, portagesettings,
			(name.find("install") == string::npos),
			(name.find("best") == string::npos));
		copy.restore(const_cast<Package*>(package));
		if(result)
			return "1";
		return "";
	}
	if(name.find("downgrade") != string::npos) {
		LocalCopy copy(this, const_cast<Package*>(package));
		bool result = package->must_downgrade(vardb, (name.find("best") == string::npos));
		copy.restore(const_cast<Package*>(package));
		if(result)
			return "1";
		return "";
	}
	if(name.find("recommend") != string::npos) {
		LocalCopy copy(this, const_cast<Package*>(package));
		bool result = package->recommend(vardb, portagesettings,
			(name.find("install") == string::npos),
			(name.find("best") == string::npos));
		copy.restore(const_cast<Package*>(package));
		if(result)
			return "1";
		return "";
	}
	if(name == "marked") {
		if(marked_list) {
			if(marked_list->is_marked(*package))
				return "1";
		}
		return "";
	}
	if(name == "colliuse")
		return package->coll_iuse();
	if(name == "haveversionuse") {
#ifndef NOT_FULL_USE
		if(package->versions_have_full_use)
			return "1";
#endif
		return "";
	}
	throw ExBasic(_("Unknown property %r")) % name;
}

const Package *old_or_new(string *new_name, const Package *older, const Package *newer, const string &name)
{
	const char *s = name.c_str();
	if(strncmp(s, "old", 3) == 0)
	{
		*new_name = s + 3;
		return older;
	}
	if(strncmp(s, "new", 3) == 0)
	{
		*new_name = s + 3;
		return newer;
	}
	*new_name = name;
	return newer;
}

string
get_package_property(const PrintFormat *fmt, const void *entity, const string &name)
{
	return fmt->get_pkg_property(static_cast<const Package *>(entity), name);
}

string
get_diff_package_property(const PrintFormat *fmt, const void *entity, const string &name)
{
	const Package *older = (static_cast<const Package* const*>(entity))[0];
	const Package *newer = (static_cast<const Package* const*>(entity))[1];
	if(name == "better")
	{
		LocalCopy copynewer(fmt, const_cast<Package*>(newer));
		LocalCopy copyolder(fmt, const_cast<Package*>(older));
		bool result = newer->have_worse(*older, true);
		copyolder.restore(const_cast<Package*>(older));
		copynewer.restore(const_cast<Package*>(newer));
		if(result)
			return "better";
		return "";
	}
	if(name == "bestbetter")
	{
		LocalCopy copynewer(fmt, const_cast<Package*>(newer));
		LocalCopy copyolder(fmt, const_cast<Package*>(older));
		bool result = newer->have_worse(*older, false);
		copyolder.restore(const_cast<Package*>(older));
		copynewer.restore(const_cast<Package*>(newer));
		if(result)
			return "better";
		return "";
	}
	if(name == "worse")
	{
		LocalCopy copynewer(fmt, const_cast<Package*>(newer));
		LocalCopy copyolder(fmt, const_cast<Package*>(older));
		bool result = older->have_worse(*newer, true);
		copyolder.restore(const_cast<Package*>(older));
		copynewer.restore(const_cast<Package*>(newer));
		if(result)
			return "worse";
		return "";
	}
	if(name == "bestworse")
	{
		LocalCopy copynewer(fmt, const_cast<Package*>(newer));
		LocalCopy copyolder(fmt, const_cast<Package*>(older));
		bool result = older->have_worse(*newer, false);
		copyolder.restore(const_cast<Package*>(older));
		copynewer.restore(const_cast<Package*>(newer));
		if(result)
			return "worse";
		return "";
	}
	if(name == "differ")
	{
		LocalCopy copynewer(fmt, const_cast<Package*>(newer));
		LocalCopy copyolder(fmt, const_cast<Package*>(older));
		bool result = newer->differ(*older, true);
		copyolder.restore(const_cast<Package*>(older));
		copynewer.restore(const_cast<Package*>(newer));
		if(result)
			return "differ";
		return "";
	}
	if(name == "bestdiffer")
	{
		LocalCopy copynewer(fmt, const_cast<Package*>(newer));
		LocalCopy copyolder(fmt, const_cast<Package*>(older));
		bool result = newer->differ(*older, false);
		copyolder.restore(const_cast<Package*>(older));
		copynewer.restore(const_cast<Package*>(newer));
		if(result)
			return "differ";
		return "";
	}
	string new_name;
	const Package *package = old_or_new(&new_name, older, newer, name);
	return fmt->get_pkg_property(package, new_name);
}
