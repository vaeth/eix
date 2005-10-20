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

#ifndef __MASK_H__
#define __MASK_H__

#include <map>
#include <string>

#include <portage/package.h>

using namespace std;

/** A class for parsing masking definitions
 * like those in the profile and /etc/portage/package.(un)mask */
class Mask : public BasicVersion {
	public:
		/** Describes the type of a mask.
		 * Nothing specific, entry in "packages"-file but not in
		 * system-profile, entry in packages-file and in system-profile,
		 * entry in package.mask, entry in package.unmask */
		typedef enum {
			maskTypeNone, maskAllowedByProfile,
			maskInSystem,
			maskMask, maskUnmask
		} Type;

		/** Describes the comparison operator before the mask. */
		typedef enum {
			maskOpAll, maskOpEqual,
			maskOpLess, maskOpLessEqual,
			maskOpGreaterEqual, maskOpGreater,
			maskOpRevisions
		} Operator;

	private:
		Operator _op; /**< Operator for mask. */
		Type _type;   /**< Mask type for this mask. */
		int _wcpos;   /**< Position to asterisk in version-string. */
		char *_category, /**< category */
			 *_name;     /**< package name */

		/** split a "mask string" into its components
		 * @param str_mask the string to be dissected
		 * @throw ExBasic on errors */
		void splitMaskString(string str_mask) throw(ExBasic);

		/** Sets the stability & masked members of ve according to the mask
		 * @param ve Version instance to be set */
		void apply(Version& ve, BasicVersion *bv = NULL);

		/** Tests if the mask applies to a Version.
		 * @param ve test this version
		 * @return true if applies. */
		bool test(Version& ve, BasicVersion *bv = NULL);

		void expand(Package *pkg);

		const char *getMaskOperator(Operator type);

	public:
		/** Parse mask-string. */
		Mask(string strMask, Type type);

		/** Free category, name, ver */
		~Mask();

		/** Tests if the mask matches a certain cat/pkg pair.
		 * @param name name of package (NULL if shall not be tested)
		 * @param category category of package (NULL if shall not be tested) */
		bool catpkgTest(const char *name, const char *category) {
			return !((name && strcmp(name, _name))
					|| (category && strcmp(category, _category)));
		}


		vector<Version*> getMatches(Package &pkg);

		const char *getVersion() {
			return ((BasicVersion*)this)->toString().c_str();
		}

		const char *getName() {
			return _name;
		}

		const char *getCategory() {
			return _category;
		}

		/** Sets the stability members of all version in package according to the mask.
		 * @param pkg            package you want tested
		 * @param check_name     true if name should be tested
		 * @param check_category true if category should be tested */
		void checkMask(Package& pkg, const bool check_category, const bool check_name);
		
		bool test(Package &pkg, Version &v);
			
		/** Print mask. */
		friend ostream& operator<< (ostream& os, Mask& m);
};

class KeywordMask : public Mask {
	public:
		KeywordMask(string strMask, Type type)
		: Mask(strMask, type) {
		}

		string keywords;
};

class MaskList : public map<string,map<string,vector<Mask*> > > {

	public:
		typedef vector<Mask*>::iterator viterator;

		~MaskList() {
			for(MaskList::iterator it = begin(); it != end(); ++it) {
				for(map<string,vector<Mask*> >::iterator t = it->second.begin(); t != it->second.end(); ++t) {
					for(unsigned int i = 0; i<t->second.size(); ++i) {
						delete (t->second[i]);
					}
				}
			}
		}

		void add(Mask *m) {
			OOM_ASSERT(m);
			(*this)[m->getCategory()][m->getName()].push_back(m);
		}
		
		friend ostream& operator<< (ostream& os, MaskList& mlist) {
			for(MaskList::iterator it = mlist.begin(); it != mlist.end(); ++it) {
				for(map<string,vector<Mask*> >::iterator t = it->second.begin(); t != it->second.end(); ++t) {
					for(unsigned int i = 0; i<t->second.size(); ++i) {
						os << *(t->second[i]) << endl;
					}
				}
			}
			return os;
		}

		vector<Mask*> *get(Package *p);
};

inline ostream& operator<< (ostream& os, Mask& m) {
	switch(m._type) {
		case Mask::maskTypeNone:          os << string("None       "); break;
		case Mask::maskAllowedByProfile:  os << string("Allowed    "); break;
		case Mask::maskInSystem:          os << string("inSystem   "); break;
		case Mask::maskMask:              os << string("Mask       "); break;
		case Mask::maskUnmask:            os << string("Unmask     "); break;
	}
	switch(m._op) {
		case Mask::maskOpAll:          os << string("  "); break;
		case Mask::maskOpEqual:        os << string(" ="); break;
		case Mask::maskOpLess:         os << string(" <"); break;
		case Mask::maskOpLessEqual:    os << string("<="); break;
		case Mask::maskOpGreaterEqual: os << string(">="); break;
		case Mask::maskOpGreater:      os << string(" >"); break;
		case Mask::maskOpRevisions:    os << string(" ~"); break;
	}
	os << string(m.getCategory()) + "/" + m.getName();
	if(m.getVersion() && strcmp(m.getVersion(), ""))
		os << string("-") + m.getVersion();
	return os;
}

#endif /* __MASK_H__ */
