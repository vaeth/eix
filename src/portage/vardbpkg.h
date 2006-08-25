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

#ifndef __PORTAGECONF_H__
#define __PORTAGECONF_H__

#include <vector>
#include <map>

#include <eixTk/exceptions.h>
#include <portage/package.h>

class PrintFormat;

/** Holds every installed version of a package. */
class VarDbPkg {
	private:
		/** Mapping of [category][package] to list versions. */
		std::map<std::string, std::map<std::string, std::vector<BasicVersion> >* > installed;
		std::string _directory; /**< This is the db-directory. */
		bool get_slots, take_care;

		/** Find installed versions of packet "name" in category "category".
		 * @return NULL if not found .. else pointer to vector of versions. */
		std::vector<BasicVersion> *getInstalledVector(const std::string &category, const std::string &name);

		/** Read category from db-directory.
		 * @param category read this category. */
		void readCategory(const char *category);

	public:
		/** Default constructor. */
		VarDbPkg(std::string directory, bool read_slots, bool care_slots) :
			_directory(directory), get_slots(read_slots || care_slots),
			take_care(care_slots)
		{ }

		~VarDbPkg() {
			std::map<std::string, std::map<std::string, std::vector<BasicVersion> >* >::iterator it = installed.begin();
			while(it != installed.end()) {
				delete it++->second;
			}
		}

		bool care_slots() const
		{ return take_care; }

		bool readSlot(const Package &p, BasicVersion &v) const;

		/** Find installed versions
		 * @return NULL if not found .. else pointer to vector of versions. */
		std::vector<BasicVersion> *getInstalledVector(const Package &p) {
			return getInstalledVector(p.category, p.name);
		};

		/** Returns true if a Package installed.
		 * @param p Check for this Package.
		 * @param v If not NULL, check for this BasicVersion. */
		bool isInstalled(const Package &p, const BasicVersion *v = NULL);

		/** Returns number of installed versions of this package
		 * @param p Check for this Package. */
		std::vector<BasicVersion>::size_type numInstalled(const Package &p);
};

#endif /* __PORTAGECONF_H__ */
