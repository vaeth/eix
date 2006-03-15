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

#ifndef __EIXRC_H__
#define __EIXRC_H__

#include <eixTk/exceptions.h>
#include <varsreader.h>

#include <vector>
#include <string>

#define EIX_USERRC   "/.eixrc"

#ifndef SYSCONFDIR
#define SYSCONFDIR "/etc"
#endif /* SYSCONFDIR */

#define EIX_SYSTEMRC SYSCONFDIR"/eixrc"

class EixRcOption {
	public:
		static const char STRING = 0, INTEGER = 1, BOOLEAN = 2;
		char type;
		string key, value, description;
		EixRcOption(char t, string name, string val, string desc) {
			type = t;
			key = name;
			value = val;
			description = desc;
		}
};

class EixRc : public map<string,string> {

	private:
		vector<EixRcOption> defaults;

	public:
		void read(void) {
			char *home = getenv("HOME");
			if(!(home))
				WARNING("No $HOME found in environment.");
			else
			{
				string eixrc(home);
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
						(*this)[defaults[i].key] = string(val);
					}
				}
			}
		}
		
		void clear() {
			defaults.clear();
			((map<string,string>*) this)->clear();
		}

		void addDefault(EixRcOption option) {
			defaults.push_back(option);
			(*this)[option.key] = option.value;
		}

		bool getBool(const char *key) {
			return ! strcasecmp((*this)[key].c_str(),"true");
		}

		int getInteger(const char *key) {
			return atoi((*this)[key].c_str());
		}

		void dumpDefaults(FILE *s) {
			for(unsigned int i = 0;
				i<defaults.size();
				++i)
			{
				char *typestring = "";
				switch(defaults[i].type) {
					case EixRcOption::BOOLEAN: typestring = "BOOLEAN";
								  break;
					case EixRcOption::STRING: typestring = "STRING";
								  break;
					case EixRcOption::INTEGER: typestring = "INTEGER";
								  break;
				}

				fprintf(s,
						"# %s\n"
						"# %s\n"
						"%s='%s'\n\n",
						typestring,
						defaults[i].description.c_str(),
						defaults[i].key.c_str(),
						defaults[i].value.c_str());
			}
		}
};
#endif /* __EIXRC_H__ */
