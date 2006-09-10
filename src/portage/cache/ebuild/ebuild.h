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

#ifndef __EBUILD_H__
#define __EBUILD_H__

#include <portage/cache/base.h>
#include <string>

class Package;
class Version;

class EbuildCache : public BasicCache {
		friend void ebuild_sig_handler(int sig);
	private:
		static EbuildCache *handler_arg;
		bool have_set_signals;
		std::string *cachefile;
		typedef void signal_handler(int sig);
		signal_handler *handleTERM, *handleINT, *handleHUP;
		bool use_ebuild_sh;

		void add_handler();
		void remove_handler();
		bool make_tempfile();
		bool make_cachefile(const char *name, const std::string &dir, const Package &package, const Version &version);
		void delete_cachefile();
	public:
		EbuildCache(bool use_sh = false) : BasicCache(), cachefile(NULL), have_set_signals(false), use_ebuild_sh(use_sh)
		{ }

		~EbuildCache()
		{ delete_cachefile(); }

		void readPackage(Category &vec, char *pkg_name, std::string *directory_path, struct dirent **list, int numfiles) throw(ExBasic);
		bool readCategory(Category &vec) throw(ExBasic);

		const char *getType() const
		{
			if(use_ebuild_sh)
				return "ebuild*";
			return "ebuild";
		}
};

#endif /* __EBUILD_H__ */
