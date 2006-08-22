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
#include <portage/vardbpkg.h>

using namespace std;

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
		if(slot.size())
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

	if((name == "availableversions") ||
		(name == "availableversionslong") ||
		(name == "availablevresionsshort")) {
		print_versions(fmt, entity, (name != "availableversionsshort"));
	}
	else if(name == "overlaykey") {
		Version::Overlay ov_key = entity->largest_overlay;
		if(ov_key && entity->have_same_overlay_key) {
			cout << fmt->overlay_keytext(ov_key);
		}
	}
	else if((name == "best") ||
		(name == "bestlong") ||
		(name == "bestshort")) {
		Version *best = entity->best();
		if(best != NULL) {
			print_version(fmt, best, entity, (name != "bestshort"), false);
		}
	}
	else if((name == "bestslots") ||
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
	}
	else
		cout << get_package_property(fmt, void_entity, name);

}

string
get_package_property(const PrintFormat *fmt, void *void_entity, const string &name) throw(ExBasic)
{
	Package *entity = (Package*)void_entity;

	if(name == "category") {
		return entity->category;
	}
	else if(name == "name") {
		return entity->name;
	}
	else if(name == "description") {
		return entity->desc;
	}
	else if(name == "homepage") {
		return entity->homepage;
	}
	else if(name == "licenses") {
		return entity->licenses;
	}
	else if(name == "installedversions") {
		return entity->installed_versions;
	}
	else if(name == "provide") {
		return entity->provide;
	}
	else if(name == "overlaykey") {
		Version::Overlay ov_key = entity->largest_overlay;
		if(ov_key && entity->have_same_overlay_key) {
			return fmt->overlay_keytext(ov_key, false);
		}
		return "";
	}
	else if(name == "system") {
		if(entity->is_system_package) {
			return "system";
		}
		return "";
	}
	else if((name == "best") ||
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
	else if((name == "bestslots") ||
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
	else if(name == "marked")
	{
		if(fmt->marked_list)
		{
			if(fmt->marked_list->is_marked(*entity))
				return "1";
		}
		return "";
	}
	else if(name == "markedversions")
	{
		if(fmt->marked_list)
			return fmt->marked_list->getMarkedString(*entity);
		return "";
	}
	throw(ExBasic("Unknown property '%s'.", name.c_str()));
}

string
get_diff_package_property(const PrintFormat *fmt, void *void_entity, const string &name) throw(ExBasic)
{
	Package *newer = ((Package**)void_entity)[1];
	Package *older = ((Package**)void_entity)[0];
	if(name == "upgrade")  {
		Version *new_best = newer->best(),
				*old_best = older->best();

		if(new_best == NULL && old_best != NULL) {
			return "";
		}

		if((new_best != NULL && old_best == NULL)
				|| *new_best > *old_best) {
			return "yes";
		}
		return "";
	}
	return get_package_property(fmt, (void*)newer, name);
}

void
print_diff_package_property(const PrintFormat *fmt, void *void_entity, const string &name) throw(ExBasic)
{
	print_package_property(fmt, ((Package**)void_entity)[1], name);
}
