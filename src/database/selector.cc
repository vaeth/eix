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

#include "selector.h"

#include <database/database.h>

Package *
DatabaseMatchIterator::next()
{
	/* Read until we have 0 packages left in this category, then read the
	 * category-header and reset the pkg-counter. */
	if(_pkgs-- == 0) {
		if(_cats-- == 0) {
			return NULL;
		}
		_pkgs = CategoryHeader::read(_input, _catname);
		return next();
	}

	/* Read offset so we can jump the package if we need to. */
	Package::offset_type next_pkg;
	off_t begin_pkg = ftello(_input);
	next_pkg = io::read<Package::offset_type>(_input);

	Package *pkg = new Package(_input);
	pkg->category = _catname;
	if(_criterium->match(pkg)) {
		pkg->readMissing();
		return pkg;
	}
	delete pkg;
	fseeko(_input, begin_pkg + next_pkg , SEEK_SET);
	return next();
}
