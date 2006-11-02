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


#include "formatstring-print.h"
#include <eixTk/sysutils.h>
#include <portage/vardbpkg.h>
#include <portage/conf/portagesettings.h>

using namespace std;

string
get_basic_version(const PrintFormat *fmt, const BasicVersion *version, bool pure_text, const string &intermediate)
{
	if((!fmt->show_slots))
		return version->getFull();
	if(pure_text || fmt->no_color || (!fmt->colored_slots))
		return version->getFullSlotted(fmt->colon_slots, intermediate);
	string slot = version->getSlotAppendix(fmt->colon_slots);
	if(slot.empty())
		return version->getFull();
	return version->getFull() + intermediate + fmt->color_slots + slot +
		AnsiColor(AnsiColor::acDefault, 0).asString();
}

string
get_inst_use(const Package &p, InstVersion &i, const PrintFormat &fmt, const char **a)
{
	if(fmt.instUseFormat.empty())
		return "";
	if(!fmt.vardb->readUse(p, i))
		return "";
	if(i.iuse.empty())
		return "";
	string ret = "";
	for(vector<string>::iterator it = i.iuse.begin();
		it != i.iuse.end(); ++it)
	{
		if(!ret.empty())
			ret.append(" ");
		int addindex = 0;
		if(i.usedUse.find(*it) == i.usedUse.end())
			addindex = 2;
		if(a[addindex])
			ret.append(a[addindex]);
		else if(addindex == 2)
			ret.append("-");
		ret.append(*it);
		if(a[addindex+1])
			ret.append(a[addindex+1]);
	}
	char *tmp;
	if( asprintf(&tmp, fmt.instUseFormat.c_str(), ret.c_str()) < 0)
		return ret;
	ret = tmp;
	free(tmp);
	return ret;
}

const char
	INST_WITH_DATE=1,      //< Installed string should contain date
	INST_WITH_USEFLAGS=2,  //< Installed string should contain useflags
	INST_WITH_NEWLINE=4,   //< Installed string should contain newlines
	INST_SHORTDATE=8;      //< date should be in short format
string
getInstalledString(const Package &p, const PrintFormat &fmt, bool pure_text, char formattype, const vector<string> &prepend)
{
	if(!fmt.vardb)
		return "";
	vector<InstVersion> *vec = fmt.vardb->getInstalledVector(p);
	if(!vec) {
		return "";
	}

	vector<InstVersion>::iterator it = vec->begin();
	if(it == vec->end())
		return "";
	string ret;
	bool useflags;
	for(;;) {
		if(!p.guess_slotname(*it, fmt.vardb))
			it->slot = "?";
		if(prepend.size() > 0)
			ret.append(prepend[0]);
		ret.append(get_basic_version(&fmt, &(*it), pure_text,
			((prepend.size() > 1) ? prepend[1] : "")));
		if(prepend.size() > 2)
			ret.append(prepend[2]);
		if(formattype & INST_WITH_DATE)
		{
			string date =
				date_conv(((formattype & INST_SHORTDATE) ?
					fmt.dateFormatShort.c_str() :
					fmt.dateFormat.c_str()),
					it->instDate);
			if(!date.empty())
			{
				if(prepend.size() > 4)
					ret.append(prepend[4]);
				ret.append(date);
				if(prepend.size() > 5)
					ret.append(prepend[5]);
			}
		}
		useflags = false;
		if(formattype & INST_WITH_USEFLAGS) {
			const char *a[4];
			for(int i=0; i<4; ++i) {
				a[i] = NULL;
				if(prepend.size() > 8+i)
					if((i == 2) || (!prepend[8+i].empty()))
						a[i] = prepend[8+i].c_str();
			}
			string inst_use = get_inst_use(p, *it, fmt, a);
			if(!inst_use.empty()) {
				if(prepend.size() > 6)
					ret.append(prepend[6]);
				ret.append(inst_use);
				useflags = true;
				if(prepend.size() > 7)
					ret.append(prepend[7]);
			}
		}
		if(prepend.size() > 3)
			ret.append(prepend[3]);
		if(++it == vec->end())
			return ret;
		if((formattype & INST_WITH_NEWLINE) &&
			(useflags || fmt.style_version_lines))
			ret.append("\n\t\t\t  ");
		else
			ret.append(" ");
	}
}

void
print_version(const PrintFormat *fmt, const Version *version, const Package *package, bool with_slots, bool exclude_overlay)
{
	bool is_installed = false;
	bool is_marked = false;
	if(!fmt->no_color)
	{
		if(fmt->vardb)
			is_installed = fmt->vardb->isInstalled(*package, version);
		if(fmt->marked_list)
			is_marked = fmt->marked_list->is_marked(*package, version);
	}

	if(fmt->style_version_lines)
		fputs("\n\t\t", stdout);

	if(version->isProfileMask()) {
		if( !fmt->no_color )
			cout << fmt->color_masked;
		fputs("[P]", stdout);
	}
	else if(version->isPackageMask()) {
		if( !fmt->no_color )
			cout << fmt->color_masked;
		fputs("[M]", stdout);
	}
	else if(version->isStable()) {
		if( !fmt->no_color )
			cout << fmt->color_stable;
	}
	else if (version->isUnstable()) {
		if( !fmt->no_color )
			cout << fmt->color_unstable;
		fputs("~", stdout);
	}
	else if (version->isMinusAsterisk()) {
		if( !fmt->no_color )
			cout << fmt->color_masked;
		fputs("*", stdout);
	}
	else if (version->isMinusKeyword()) {
		if( !fmt->no_color )
			cout << fmt->color_masked;
		fputs("-", stdout);
	}
	else if (version->isMissingKeyword()) {
		if( !fmt->no_color )
			cout << fmt->color_masked;
		fputs("!", stdout);
	}
	else {
		if( !fmt->no_color )
			cout << fmt->color_masked;
		fputs("?", stdout);
	}

	if (fmt->style_version_lines)
		fputs("\t", stdout);

	if (is_installed)
		cout << fmt->mark_installed;
	if (is_marked)
		cout << fmt->mark_version;
	if (with_slots && fmt->show_slots && (!fmt->colored_slots))
		cout << version->getFullSlotted(fmt->colon_slots);
	else
		cout << version->getFull();
	if (is_marked)
	{
		cout << fmt->mark_version_end;
		if(is_installed &&
			(fmt->mark_version_end != fmt->mark_installed_end))
		{
			cout << fmt->mark_installed_end;
		}
	}
	else if (is_installed)
		cout << fmt->mark_installed_end;
	if (with_slots && fmt->show_slots && fmt->colored_slots)
	{
		string slot = version->getSlotAppendix(fmt->colon_slots);
		if(!slot.empty())
		{
			if(! fmt->no_color)
				cout << fmt->color_slots;
			cout << slot;
		}
	}
	if(!exclude_overlay)
	{
		if(!package->have_same_overlay_key && version->overlay_key)
			cout << fmt->overlay_keytext(version->overlay_key);
	}
}

void
print_versions_versions(const PrintFormat *fmt, const Package* p, bool with_slots)
{
	Package::const_iterator vit = p->begin();
	while(vit != p->end()) {
		print_version(fmt, *vit, p, with_slots, false);
		if(++vit != p->end() && !fmt->style_version_lines)
			cout << " ";
	}
	if( !fmt->no_color )
		cout << AnsiColor(AnsiColor::acDefault, 0);
}

void
print_versions_slots(const PrintFormat *fmt, const Package* p)
{
	if(!p->have_nontrivial_slots)
	{
		print_versions_versions(fmt, p, false);
		return;
	}
	const SlotList *sl = &(p->slotlist);
	bool only_one = (sl->size() == 1);
	for(SlotList::const_iterator it = sl->begin();
		it != sl->end(); ++it)
	{
		const char *s = it->slot();
		if((!only_one) || fmt->style_version_lines)
			fputs("\n\t", stdout);
		if( !fmt->no_color)
			cout << fmt->color_slots;
		if(s[0])
			cout << "(" << s << ")";
		else
			cout << "(0)";
		if( !fmt->no_color)
			cout << AnsiColor(AnsiColor::acDefault, 0);
		if( !fmt->style_version_lines)
			cout << (only_one ? "  " : "\t");
		const VersionList *vl = &(it->const_version_list());
		VersionList::const_iterator vit = vl->begin();
		while(vit != vl->end())
		{
			print_version(fmt, *vit, p, false, false);
			if(++vit != vl->end() && !fmt->style_version_lines)
				cout << " ";
		}
		if( !fmt->no_color )
			cout << AnsiColor(AnsiColor::acDefault, 0);
	}
}

void
print_versions(const PrintFormat *fmt, const Package* p, bool with_slots)
{
	if(fmt->slot_sorted)
		print_versions_slots(fmt, p);
	else
		print_versions_versions(fmt, p, with_slots);
}

void
print_package_property(const PrintFormat *fmt, void *void_entity, const string &name) throw(ExBasic)
{
	Package *entity = (Package*)void_entity;

	vector<string> prepend = split_string(name, ":", false);
	string plainname = prepend[0];
	prepend.erase(prepend.begin());

	if((name == "availableversions") ||
		(name == "availableversionslong") ||
		(name == "availablevresionsshort")) {
		print_versions(fmt, entity, (name != "availableversionsshort"));
		return;
	}
	if((plainname == "installedversions") ||
		(plainname == "installedversionsdate") ||
		(plainname == "installedversionsshortdate") ||
		(plainname == "installedversionsshort")) {
		if(!fmt->vardb)
			return;
		char formattype = 0;
		if(plainname != "installedversionsshort") {
			formattype = INST_WITH_DATE;
			if(plainname == "installedversions")
				formattype |= INST_WITH_USEFLAGS|INST_WITH_NEWLINE;
			else if(plainname == "installedversionsshortdate")
				formattype |= INST_SHORTDATE;
		}
		cout << getInstalledString(*entity, *fmt, false, formattype, prepend);
		return;
	}
	if(name == "overlaykey") {
		Version::Overlay ov_key = entity->largest_overlay;
		if(ov_key && entity->have_same_overlay_key) {
			cout << fmt->overlay_keytext(ov_key);
		}
		return;
	}
	if((name == "best") ||
		(name == "bestlong") ||
		(name == "bestshort")) {
		Version *best = entity->best();
		if(best != NULL) {
			print_version(fmt, best, entity, (name != "bestshort"), false);
		}
		return;
	}
	if((name == "bestslots") ||
		(name == "bestslotslong") ||
		(name == "bestslotsshort")) {
		vector<Version*> versions;
		entity->best_slots(versions);
		for(vector<Version*>::const_iterator it = versions.begin();
			it != versions.end(); ++it)
		{
			if(it != versions.begin())
				cout << " ";
			print_version(fmt, *it, entity, (name != "bestslotshort"), false);
		}
		return;
	}
	cout << get_package_property(fmt, void_entity, name);
}

string
get_package_property(const PrintFormat *fmt, void *void_entity, const string &name) throw(ExBasic)
{
	Package *entity = (Package*)void_entity;

	if(name == "category") {
		return entity->category;
	}
	if(name == "name") {
		return entity->name;
	}
	if(name == "description") {
		return entity->desc;
	}
	if(name == "homepage") {
		return entity->homepage;
	}
	if(name == "licenses") {
		return entity->licenses;
	}
	if((name == "installedversions") ||
		(name == "installedversionsdate") ||
		(name == "installedversionsshortdate") ||
		(name == "installedversionsshort")) {
		if(!fmt->vardb)
			return "";
		char formattype = 0;
		if(name != "installedversionsshort") {
			formattype = INST_WITH_DATE;
			if(name == "installedversions")
				formattype |= INST_WITH_USEFLAGS|INST_WITH_NEWLINE;
			else if(name == "installedversionsshortdate")
				formattype |= INST_SHORTDATE;
		}
		vector<string> prepend;
		return getInstalledString(*entity, *fmt, true, formattype, prepend);
	}
	if(name == "provide") {
		return entity->provide;
	}
	if(name == "overlaykey") {
		Version::Overlay ov_key = entity->largest_overlay;
		if(ov_key && entity->have_same_overlay_key) {
			return fmt->overlay_keytext(ov_key, false);
		}
		return "";
	}
	if(name == "system") {
		if(entity->is_system_package) {
			return "system";
		}
		return "";
	}
	if((name == "best") ||
		(name == "bestlong") ||
		(name == "bestshort")) {
		Version *best = entity->best();
		if(best == NULL) {
			return "";
		}
		if(fmt->show_slots && (name != "bestshort"))
			return best->getFullSlotted(fmt->colon_slots);
		return best->getFull();
	}
	if((name == "bestslots") ||
		(name == "bestslotslong") ||
		(name == "bestslotsshort")) {
		bool with_slots = (name != "bestslotshort");
		vector<Version*> versions;
		entity->best_slots(versions);
		string ret;
		for(vector<Version*>::const_iterator it = versions.begin();
			it != versions.end(); ++it)
		{
			if(ret.length())
				ret += " ";
			if(with_slots)
				ret += (*it)->getFullSlotted(fmt->colon_slots);
			else
				ret += (*it)->getFull();
		}
		return ret;
	}
	if(name == "marked")
	{
		if(fmt->marked_list)
		{
			if(fmt->marked_list->is_marked(*entity))
				return "1";
		}
		return "";
	}
	if(name == "markedversions")
	{
		if(fmt->marked_list)
			return fmt->marked_list->getMarkedString(*entity);
		return "";
	}
	if(name == "upgrade")
	{
		if(LocalCopy(fmt, entity).package->can_upgrade(fmt->vardb, true, true))
			return "1";
		return "";
	}
	if(name == "upgradeorinstall")
	{
		if(LocalCopy(fmt, entity).package->can_upgrade(fmt->vardb, false, true))
			return "1";
		return "";
	}
	if(name == "downgrade")
	{
		if(LocalCopy(fmt, entity).package->must_downgrade(fmt->vardb, true))
			return "1";
		return "";
	}
	if(name == "recommend")
	{
		if(LocalCopy(fmt, entity).package->recommend(fmt->vardb, true, true))
			return "1";
		return "";
	}
	if(name == "recommendorinstall")
	{
		if(LocalCopy(fmt, entity).package->recommend(fmt->vardb, false, true))
			return "1";
		return "";
	}
	if(name == "bestupgrade")
	{
		if(LocalCopy(fmt, entity).package->can_upgrade(fmt->vardb, true, false))
			return "1";
		return "";
	}
	if(name == "bestupgradeorinstall")
	{
		if(LocalCopy(fmt, entity).package->can_upgrade(fmt->vardb, false, false))
			return "1";
		return "";
	}
	if(name == "bestdowngrade")
	{
		if(LocalCopy(fmt, entity).package->must_downgrade(fmt->vardb, false))
			return "1";
		return "";
	}
	if(name == "bestrecommend")
	{
		if(LocalCopy(fmt, entity).package->recommend(fmt->vardb, true, false))
			return "1";
		return "";
	}
	if(name == "bestrecommendorinstall")
	{
		if(LocalCopy(fmt, entity).package->recommend(fmt->vardb, false, false))
			return "1";
		return "";
	}
	throw(ExBasic("Unknown property '%s'.", name.c_str()));
}

void *old_or_new(string *new_name, Package *older, Package *newer, const string &name)
{
	const char *s = name.c_str();
	if(strncmp(s, "old", 3) == 0)
	{
		*new_name = s + 3;
		return (void *)older;
	}
	if(strncmp(s, "new", 3) == 0)
	{
		*new_name = s + 3;
		return (void *)newer;
	}
	*new_name = name;
	return (void *)newer;
}

string
get_diff_package_property(const PrintFormat *fmt, void *void_entity, const string &name) throw(ExBasic)
{
	Package *older = ((Package**)void_entity)[0];
	Package *newer = ((Package**)void_entity)[1];
	if(name == "better")
	{
		if(LocalCopy(fmt, newer).package->have_worse(*(LocalCopy(fmt, older).package), true))
			return "1";
		return "";
	}
	if(name == "bestbetter")
	{
		if(LocalCopy(fmt, newer).package->have_worse(*(LocalCopy(fmt, older).package), false))
			return "1";
		return "";
	}
	if(name == "worse")
	{
		if(LocalCopy(fmt, older).package->have_worse(*(LocalCopy(fmt, newer).package), true))
			return "1";
		return "";
	}
	if(name == "bestworse")
	{
		if(LocalCopy(fmt, older).package->have_worse(*(LocalCopy(fmt, newer).package), false))
			return "1";
		return "";
	}
	if(name == "differ")
	{
		if(LocalCopy(fmt, newer).package->differ(*(LocalCopy(fmt, older).package), true))
			return "1";
		return "";
	}
	if(name == "bestdiffer")
	{
		if(LocalCopy(fmt, newer).package->differ(*(LocalCopy(fmt, older).package), false))
			return "1";
		return "";
	}
	string new_name;
	void *entity = old_or_new(&new_name, older, newer, name);
	return get_package_property(fmt, entity, new_name);
}

void
print_diff_package_property(const PrintFormat *fmt, void *void_entity, const string &name) throw(ExBasic)
{
	Package *older = ((Package**)void_entity)[0];
	Package *newer = ((Package**)void_entity)[1];
	string new_name;
	void *entity = old_or_new(&new_name, older, newer, name);
	print_package_property(fmt, entity, new_name);
}
