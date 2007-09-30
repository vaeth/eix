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

#ifndef __SQLITECACHE_H__
#define __SQLITECACHE_H__

#include <portage/cache/base.h>
#include <eixTk/stringutils.h>

class SqliteCache : public BasicCache {
	friend int sqlite_callback(void *NotUsed, int argc, char **argv, char **azColName);

	private:
		/** This variable is actually the this-parameter for our sqlite_callback function.
		    Note that this makes readCategories non-reentrant. */
		static SqliteCache *callback_arg;
		/** Our sqlite_callback() will set this to true in case of an error. */
		bool sqlite_callback_error;
		/** Parameter passing to sqlite_callback() */
		PackageTree *packagetree;
		Category *category;

	public:
		bool can_read_multiple_categories() const
		{ return true; }

		bool readCategories(PackageTree *packagetree, std::vector<std::string> *categories, Category *category = NULL) throw(ExBasic);

		const char *getType() const
		{ return "sqlite"; }
};

#endif /* __SQLITECACHE_H__ */
