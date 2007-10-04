// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)                                                         
//   Wolfgang Frisch <xororand@users.sourceforge.net>                    
//   Emil Beinroth <emilbeinroth@gmx.net>                                
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>                     

#ifndef __NOCACHE_H__
#define __NOCACHE_H__

#include <portage/cache/base.h>

class NoneCache : public BasicCache {

	private:
		bool nosubst;

		void readPackage(Category &vec, const char *pkg_name, std::string *directory_path, struct dirent **list, int numfiles) throw(ExBasic);
	public:
		NoneCache(bool no_substitute = false) : BasicCache(), nosubst(no_substitute)
		{ }

		bool readCategory(Category &vec) throw(ExBasic);

		bool use_prefixport() const
		{ return true; }

		const char *getType() const
		{
			if(nosubst)
				return "none*";
			return "none";
		}
};

#endif /* __NOCACHE_H__ */
