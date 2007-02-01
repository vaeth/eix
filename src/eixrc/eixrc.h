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

#ifndef __EIXRC_H__
#define __EIXRC_H__

#include <vector>
#include <string>
#include <map>
#include <set>
#include <portage/keywords.h>
#include <search/redundancy.h>

#define EIX_VARS_PREFIX "EIX_"
#define DIFF_EIX_VARS_PREFIX "DIFF_"

class EixRcOption {
	public:
		static const char STRING = 0, INTEGER = 1, BOOLEAN = 2, LOCAL = 3;
		char type;
		std::string key, value, local_value, description;

		EixRcOption(char t, std::string name, std::string val, std::string desc);
};

class EixRcReference;
class EixRc : public std::map<std::string,std::string> {
	public:
		std::string varprefix;
		std::string m_eprefix, m_eprefixconf, m_eprefixport;
		const char *eprefix;
		typedef std::vector<EixRcOption>::size_type default_index;
		typedef std::pair<RedAtom, RedAtom> RedPair;

		void read();

		void clear();

		void addDefault(EixRcOption option);

		bool getBool(const char *key)
		{ return istrue((*this)[key].c_str()); }

		void getRedundantFlags(const char *key,
			Keywords::Redundant type,
			RedPair &p);

		int getInteger(const char *key);

		void dumpDefaults(FILE *s, bool use_defaults);
	private:
		static bool istrue(const char *s);
		enum DelayedType { DelayedNotFound, DelayedVariable, DelayedIf, DelayedNotif, DelayedElse, DelayedFi };
		std::vector<EixRcOption> defaults;
		static bool getRedundantFlagAtom(const char *s, Keywords::Redundant type, RedAtom &r);
		std::string *resolve_delayed_recurse(std::string key, std::set<std::string> &visited, std::set<std::string> &has_reference, const char **errtext, std::string *errvar);

		  /** Create defaults and the main map with all variables
		     (including all values required by delayed references).
		   @arg has_reference is initialized to corresponding keys */
		void read_undelayed(std::set<std::string> &has_reference);
		void join_delayed(const std::string &val, std::set<std::string> &default_keys, const std::map<std::string,std::string> &tempmap);
		static DelayedType find_next_delayed(const std::string &str, std::string::size_type *pos, std::string::size_type *length = NULL);
		static std::string as_comment(const char *s);
};
#endif /* __EIXRC_H__ */
