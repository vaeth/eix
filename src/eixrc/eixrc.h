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

#include <eixTk/exceptions.h>
#include <varsreader.h>

#include <vector>
#include <string>
#include <portage/keywords.h>
#include <search/redundancy.h>

#define EIX_USERRC   "/.eixrc"

#ifndef SYSCONFDIR
#define SYSCONFDIR "/etc"
#endif /* SYSCONFDIR */

#define EIX_SYSTEMRC SYSCONFDIR"/eixrc"

class EixRcOption {
	public:
		static const char STRING = 0, INTEGER = 1, BOOLEAN = 2;
		char type;
		std::string key, value, description;
		EixRcOption(char t, std::string name, std::string val, std::string desc) {
			type = t;
			key = name;
			value = val;
			description = desc;
		}
};

inline std::string as_comment(const char *s) {
	std::string ret = s;
	std::string::size_type pos = 0;
	while(pos = ret.find("\n", pos), pos != std::string::npos) {
		ret.insert(pos + 1, "# ");
		pos += 2;
	}
	return ret;
}

class EixRc : public std::map<std::string,std::string> {

	private:
		std::vector<EixRcOption> defaults;

		bool getRedundantFlagAtom(const char *s,
			Keywords::Redundant type, RedAtom &r)
		{
			r.only &= ~type;
			if(s == NULL)
			{
				r.red &= ~type;
				return true;
			}
			if(*s == '+')
			{
				s++;
				r.only |= type;
				r.oins |= type;
			}
			else if(*s == '-')
			{
				s++;
				r.only |= type;
				r.oins &= ~type;
			}
			if((strcasecmp(s, "no") == 0) ||
			   (strcasecmp(s, "false") == 0))
			{
				r.red &= ~type;
			}
			else if(strcasecmp(s, "some") == 0)
			{
				r.red |= type;
				r.all &= ~type;
				r.spc &= ~type;
			}
			else if(strcasecmp(s, "some-installed") == 0)
			{
				r.red |= type;
				r.all &= ~type;
				r.spc |= type;
				r.ins |= type;
			}
			else if(strcasecmp(s, "some-uninstalled") == 0)
			{
				r.red |= type;
				r.all &= ~type;
				r.spc |= type;
				r.ins &= ~type;
			}
			else if(strcasecmp(s, "all") == 0)
			{
				r.red |= type;
				r.all |= type;
				r.spc &= ~type;
			}
			else if(strcasecmp(s, "all-installed") == 0)
			{
				r.red |= type;
				r.all |= type;
				r.spc |= type;
				r.ins |= type;
			}
			else if(strcasecmp(s, "all-uninstalled") == 0)
			{
				r.red |= type;
				r.all |= type;
				r.spc |= type;
				r.ins &= ~type;
			}
			else
				return false;
			return true;
		}


	public:
		typedef std::pair<RedAtom, RedAtom> RedPair;

		void read(void) {
			char *home = getenv("HOME");
			if(!(home))
				WARNING("No $HOME found in environment.");
			else
			{
				std::string eixrc(home);
				eixrc.append(EIX_USERRC);

				VarsReader rc(VarsReader::SUBST_VARS
						|VarsReader::INTO_MAP);
				rc.useMap(this);
				rc.read(EIX_SYSTEMRC);
				rc.read(eixrc.c_str());

				// look for stuff from ENV
				for(unsigned int i = 0;
					i<defaults.size();
					++i)
				{
					char *val = getenv(defaults[i].key.c_str());
					if(val != NULL)
					{
						(*this)[defaults[i].key] = std::string(val);
					}
				}
			}
		}

		void clear() {
			defaults.clear();
			((std::map<std::string,std::string>*) this)->clear();
		}

		void addDefault(EixRcOption option) {
			defaults.push_back(option);
			(*this)[option.key] = option.value;
		}

		bool getBool(const char *key) {
			const char *s = (*this)[key].c_str();
			if(strcasecmp(s, "true") == 0)
				return true;
			if(strcasecmp(s, "1") == 0)
				return true;
			if(strcasecmp(s, "yes") == 0)
				return true;
			if(strcasecmp(s, "y") == 0)
				return true;
			if(strcasecmp(s, "on") == 0)
				return true;
			return false;
		}

		void getRedundantFlags(const char *key,
			Keywords::Redundant type,
			RedPair &p)
		{
			std::string value=(*this)[key].c_str();
			std::vector<std::string> a=split_string(value);
			bool fail = false;

			for(;;)// a dummy loop for break on errors
			{
				std::vector<std::string>::iterator it = a.begin();
				if(it == a.end())
					break;
				if(!getRedundantFlagAtom(it->c_str(), type, p.first))
					break;
				++it;
				if(it == a.end())
				{
					getRedundantFlagAtom(NULL, type, p.second);
					return;
				}
				const char *s = it->c_str();
				if((strcasecmp(s, "or") == 0) ||
					(strcasecmp(s, "||") == 0) ||
					(strcasecmp(s, "|") == 0))
				{
					++it;
					if(it == a.end())
						break;
					s = it->c_str();
				}
				if(!getRedundantFlagAtom(s, type, p.first))
					break;
				++it;
				if(it == a.end())
					return;
				break;
			}
			WARNING("%s has unknown value \"%s\";\n"
				"\tassuming value \"all-installed\" instead.",
				key, value.c_str());
			getRedundantFlagAtom("all-installed", type, p.first);
			getRedundantFlagAtom(NULL, type, p.second);
		}

		int getInteger(const char *key) {
			return atoi((*this)[key].c_str());
		}

		void dumpDefaults(FILE *s, bool use_defaults) {
			const char *message = use_defaults ?
				"was locally changed to:" :
				"changed locally, default was:";
			for(unsigned int i = 0;
				i<defaults.size();
				++i)
			{
				const char *typestring = "UNKNOWN";
				switch(defaults[i].type) {
					case EixRcOption::BOOLEAN: typestring = "BOOLEAN";
								  break;
					case EixRcOption::STRING: typestring = "STRING";
								  break;
					case EixRcOption::INTEGER: typestring = "INTEGER";
								  break;
				}
				const char *key   = defaults[i].key.c_str();
				const char *deflt = defaults[i].value.c_str();
				const char *value = (*this)[defaults[i].key].c_str();
				const char *output = (use_defaults ? deflt : value);
				const char *comment = (use_defaults ? value : deflt);
				fprintf(s,
						"# %s\n"
						"# %s\n"
						"%s='%s'\n",
						as_comment(typestring).c_str(),
						as_comment(defaults[i].description.c_str()).c_str(),
						key,
						output);
				if(strcmp(deflt,value) == 0)
					fprintf(s, "\n");
				else {
					fprintf(s,
						"# %s\n"
						"# %s='%s'\n\n",
						message,
						key,
						as_comment(comment).c_str());
				}
			}
		}
};
#endif /* __EIXRC_H__ */
