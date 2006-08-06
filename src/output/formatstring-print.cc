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
#include <sstream>

using namespace std;

void
print_version(const PrintFormat *fmt, const Version *version, const Package *package)
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
		fputs("\n\t", stdout);

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
		fputs("\t\t", stdout);

	if (is_installed)
		cout << fmt->mark_installed;
	if (is_marked)
		cout << fmt->mark_version;
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
}

void
print_versions(const PrintFormat *fmt, const Package* p)
{
	Package::const_iterator version_it = p->begin();
	while(version_it != p->end()) {
		print_version(fmt, *version_it, p);

		if(!p->have_same_overlay_key && version_it->overlay_key) {
			if( ! fmt->no_color )
				cout << fmt->color_overlaykey;

			Version::Overlay ov_key = version_it->overlay_key;
			if(fmt->overlay_translations)
				ov_key = (*(fmt->overlay_translations))[ov_key - 1];
			cout << "[" << ov_key << "] ";
		}

		if(++version_it != p->end() && !fmt->style_version_lines)
			cout<<" ";
	}
	if( !fmt->no_color )
		cout << AnsiColor(AnsiColor::acDefault, 0);
}


void
print_package_property(const PrintFormat *fmt, void *void_entity, const string &name) throw(ExBasic)
{
	Package *entity = (Package*)void_entity;

	if(name == "availableversions") {
		print_versions(fmt, entity);
	}
	else if(name == "overlaykey") {
		Version::Overlay ov_key = entity->largest_overlay;
		if(ov_key && entity->have_same_overlay_key)
		{
			if(fmt->overlay_translations)
				ov_key = (*(fmt->overlay_translations))[ov_key - 1];
			cout << "[" << ov_key << "] ";
		}
	}
	else if(name == "best") {
		Version *best = entity->best();
		if(best != NULL) {
			print_version(fmt, best, entity);
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
		stringstream ss;
		string ret;
		Version::Overlay ov_key = entity->largest_overlay;
		if(ov_key && entity->have_same_overlay_key) {
			if(fmt->overlay_translations)
				ov_key = (*(fmt->overlay_translations))[ov_key - 1];
			ss << ov_key;
			ss >> ret;
		}
		return ret;
	}
	else if(name == "system") {
		if(entity->is_system_package) {
			return "system";
		}
		return "";
	}
	else if(name == "best") {
		Version *best = entity->best();
		if(best == NULL) {
			return "";
		}
		return best->getFull();
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
