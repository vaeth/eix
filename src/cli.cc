// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include "../config.h"
#include "cli.h"
#include <eixTk/stringutils.h>

using namespace std;

#define FINISH_CURRENT do { \
	current->setTest(test); \
	current->finalize(); \
} while(0)

#define USE_NEXT do { \
	FINISH_CURRENT; \
	current = next; \
	test = new PackageTest(varpkg_db, portagesettings, stability, header); \
} while(0)

inline bool optional_increase(ArgumentReader::iterator &arg, ArgumentReader::iterator end)
{
	ArgumentReader::iterator next = arg;
	if(++next == end)
		return false;
	arg = next;
	return true;
}

Matchatom *
parse_cli(EixRc &eixrc, VarDbPkg &varpkg_db, PortageSettings &portagesettings, const SetStability &stability, const DBHeader &header, MarkedList **marked_list, ArgumentReader::iterator arg, ArgumentReader::iterator end)
{
	/* Our root Matchatom. */
	Matchatom   *root    = new Matchatom();
	Matchatom   *current = root;
	PackageTest *test    = new PackageTest(varpkg_db, portagesettings, stability, header);

	bool need_logical_operator = false;
	bool have_default_operator = false;
	bool default_operator = false;
	while(arg != end)
	{
		// Check for logical operator {{{
		{
			Matchatom *next = NULL;

			if(**arg == 'a')
			{
				next = current->AND();
				++arg;
			}
			else if(**arg == 'o')
			{
				next = current->OR();
				++arg;
			}
			else if(need_logical_operator)
			{
				if(!have_default_operator)
				{
					have_default_operator = true;
					default_operator = eixrc.getBool("DEFAULT_IS_OR");
				}
				if(default_operator)
					next = current->OR();
				else
					next = current->AND();
			}

			if(next != NULL)
			{
				USE_NEXT;
				need_logical_operator = false;
				continue;
			}
		}
		// }}}

		EixRc::RedPair red;
		PackageTest::TestInstalled test_installed;
		bool firsttime;
		switch(**arg)
		{
			// Check local options {{{
			case 'I': test->Installed();
				  break;
			case 'i': test->Installed(true);
				  break;
			case '1': test->Slotted();
				  break;
			case '2': test->Slotted(true);
				  break;
			case 'u': test->Upgrade(eixrc.getLocalMode("UPGRADE_LOCAL_MODE"));
				  break;
			case O_UPGRADE_LOCAL:
				  test->Upgrade(LOCALMODE_LOCAL);
				  break;
			case O_UPGRADE_NONLOCAL:
				  test->Upgrade(LOCALMODE_NONLOCAL);
				  break;
			case O_STABLE_DEFAULT: test->SetStabilityDefault(PackageTest::STABLE_FULL);
				  break;
			case O_TESTING_DEFAULT: test->SetStabilityDefault(PackageTest::STABLE_TESTING);
				  break;
			case O_NONMASKED_DEFAULT: test->SetStabilityDefault(PackageTest::STABLE_NONMASKED);
				  break;
			case O_SYSTEM_DEFAULT: test->SetStabilityDefault(PackageTest::STABLE_SYSTEM);
				  break;
			case O_STABLE_LOCAL: test->SetStabilityLocal(PackageTest::STABLE_FULL);
				  break;
			case O_TESTING_LOCAL: test->SetStabilityLocal(PackageTest::STABLE_TESTING);
				  break;
			case O_NONMASKED_LOCAL: test->SetStabilityLocal(PackageTest::STABLE_NONMASKED);
				  break;
			case O_SYSTEM_LOCAL: test->SetStabilityLocal(PackageTest::STABLE_SYSTEM);
				  break;
			case O_STABLE_NONLOCAL: test->SetStabilityNonlocal(PackageTest::STABLE_FULL);
				  break;
			case O_TESTING_NONLOCAL: test->SetStabilityNonlocal(PackageTest::STABLE_TESTING);
				  break;
			case O_NONMASKED_NONLOCAL: test->SetStabilityNonlocal(PackageTest::STABLE_NONMASKED);
				  break;
			case O_SYSTEM_NONLOCAL: test->SetStabilityNonlocal(PackageTest::STABLE_SYSTEM);
				  break;
			case O_OVERLAY:
				  if(optional_increase(arg, end)) {
					header.get_overlay_vector(
						test->OverlayList(),
						arg->m_argument,
						portagesettings["PORTDIR"].c_str());
					break;
				  }
			case 'O': test->Overlay();
				  break;
			case O_ONLY_OVERLAY:
				  if(optional_increase(arg, end)) {
					header.get_overlay_vector(
						test->OverlayOnlyList(),
						arg->m_argument,
						portagesettings["PORTDIR"].c_str());
					break;
				  }
				  header.get_overlay_vector(
					test->OverlayOnlyList(),
					"",
					portagesettings["PORTDIR"].c_str());
				  break;
			case O_INSTALLED_OVERLAY:
				  if(optional_increase(arg, end)) {
					header.get_overlay_vector(
						test->InOverlayInstList(),
						arg->m_argument,
						portagesettings["PORTDIR"].c_str());
					break;
				  }
				  // No break here...
			case O_INSTALLED_SOME: header.get_overlay_vector(
					test->InOverlayInstList(),
					"",
					portagesettings["PORTDIR"].c_str());
				  break;
			case O_FROM_OVERLAY:
				  if(optional_increase(arg, end)) {
					header.get_overlay_vector(
						test->FromOverlayInstList(),
						arg->m_argument,
						portagesettings["PORTDIR"].c_str());
					test->FromForeignOverlayInstList()->push_back(arg->m_argument);
					break;
				  }
				  // No break here...
			case 'J':
				  header.get_overlay_vector(
					test->FromOverlayInstList(),
					"",
					portagesettings["PORTDIR"].c_str());
				  test->FromForeignOverlayInstList()->push_back("");
				  break;
			case 'd': test->DuplPackages(eixrc.getBool("DUP_PACKAGES_ONLY_OVERLAYS"));
				  break;
			case 'D': test->DuplVersions(eixrc.getBool("DUP_VERSIONS_ONLY_OVERLAYS"));
				  break;
			case O_FETCH: test->Restrictions(ExtendedVersion::RESTRICT_FETCH);
				  break;
			case O_MIRROR: test->Restrictions(ExtendedVersion::RESTRICT_MIRROR);
				  break;
			case 'T': red.first = red.second = RedAtom();
				  if(eixrc.getBool("TEST_FOR_REDUNDANCY"))
				  {
					eixrc.getRedundantFlags("REDUNDANT_IF_DOUBLE",
						Keywords::RED_DOUBLE, red);
					eixrc.getRedundantFlags("REDUNDANT_IF_DOUBLE_LINE",
						Keywords::RED_DOUBLE_LINE, red);
					eixrc.getRedundantFlags("REDUNDANT_IF_MIXED",
						Keywords::RED_MIXED, red);
					eixrc.getRedundantFlags("REDUNDANT_IF_WEAKER",
						Keywords::RED_WEAKER, red);
					eixrc.getRedundantFlags("REDUNDANT_IF_STRANGE",
						Keywords::RED_STRANGE, red);
					eixrc.getRedundantFlags("REDUNDANT_IF_NO_CHANGE",
						Keywords::RED_NO_CHANGE, red);
					eixrc.getRedundantFlags("REDUNDANT_IF_MASK_NO_CHANGE",
						Keywords::RED_MASK, red);
					eixrc.getRedundantFlags("REDUNDANT_IF_UNMASK_NO_CHANGE",
						Keywords::RED_UNMASK, red);
					eixrc.getRedundantFlags("REDUNDANT_IF_DOUBLE_MASKED",
						Keywords::RED_DOUBLE_MASK, red);
					eixrc.getRedundantFlags("REDUNDANT_IF_DOUBLE_UNMASKED",
						Keywords::RED_DOUBLE_UNMASK, red);
					eixrc.getRedundantFlags("REDUNDANT_IF_DOUBLE_USE",
						Keywords::RED_DOUBLE_USE, red);
					eixrc.getRedundantFlags("REDUNDANT_IF_DOUBLE_CFLAGS",
						Keywords::RED_DOUBLE_CFLAGS, red);
					eixrc.getRedundantFlags("REDUNDANT_IF_MINUSASTERISK",
						Keywords::RED_MINUSASTERISK, red);
					eixrc.getRedundantFlags("REDUNDANT_IF_IN_KEYWORDS",
						Keywords::RED_IN_KEYWORDS, red);
					eixrc.getRedundantFlags("REDUNDANT_IF_IN_MASK",
						Keywords::RED_IN_MASK, red);
					eixrc.getRedundantFlags("REDUNDANT_IF_IN_UNMASK",
						Keywords::RED_IN_UNMASK, red);
					eixrc.getRedundantFlags("REDUNDANT_IF_IN_USE",
						Keywords::RED_IN_USE, red);
					eixrc.getRedundantFlags("REDUNDANT_IF_IN_CFLAGS",
						Keywords::RED_IN_CFLAGS, red);
				  }
				  test_installed = PackageTest::INS_NONE;
				  if(eixrc.getBool("TEST_FOR_NONEXISTENT"))
				  {
					test_installed |= PackageTest::INS_NONEXISTENT;
					if(eixrc.getBool("NONEXISTENT_IF_MASKED"))
						test_installed |= PackageTest::INS_MASKED;
					if(eixrc.getBool("NONEXISTENT_IF_OTHER_OVERLAY")) {
						test_installed |= PackageTest::INS_OVERLAY;
					}
				  }
				  test->ObsoleteCfg(red.first, red.second, test_installed);
				  break;
			case '!': test->Invert();
				  break;
			// }}}

			// Check for field-designators {{{
			case 's': *test |= PackageTest::NAME;          break;
			case 'C': *test |= PackageTest::CATEGORY;      break;
			case 'A': *test |= PackageTest::CATEGORY_NAME; break;
			case 'S': *test |= PackageTest::DESCRIPTION;   break;
			case 'L': *test |= PackageTest::LICENSE;       break;
			case 'H': *test |= PackageTest::HOMEPAGE;      break;
			case 'P': *test |= PackageTest::PROVIDE;       break;
			case 'U': *test |= PackageTest::IUSE;          break;
			case O_INSTALLED_WITH_USE:
			          *test |= PackageTest::USE_ENABLED;   break;
			case O_INSTALLED_WITHOUT_USE:
			          *test |= PackageTest::USE_DISABLED;  break;
			// }}}

			// Check for algorithms {{{
			case 'f': if(++arg != end
					 && arg->type == Parameter::ARGUMENT
					 && is_numeric(arg->m_argument))
				  {
					  test->setAlgorithm(new FuzzyAlgorithm(atoi(arg->m_argument)));
				  }
				  else
				  {
					  test->setAlgorithm(new FuzzyAlgorithm(LEVENSHTEIN_DISTANCE));
					  arg--;
				  }
				  break;
			case 'r': test->setAlgorithm(new RegexAlgorithm());
				  break;
			case 'e': test->setAlgorithm(new ESMAlgorithm());
				  break;
			case 'p': test->setAlgorithm(new WildcardAlgorithm());
				  break;
			// }}}

			// Read from pipe {{{
			case '|':
				test->setAlgorithm(new ESMAlgorithm());
				*test = PackageTest::CATEGORY_NAME;
				firsttime = true;
				while(!cin.eof())
				{
					string line;
					getline(cin, line);
					trim(&line);
					vector<string> wordlist = split_string(line.c_str());
					vector<string>::iterator word = wordlist.begin();
					string::size_type i;
					for(; word != wordlist.end(); ++word)
					{
						i = word->find("/");
						if(i == string::npos)
							continue;
						if(word->find("/", i + 1) == string::npos)
							break;
					}
					if(word == wordlist.end())
						continue;
					if(! firsttime)
					{
						Matchatom *next = current->OR();
						USE_NEXT;
						test->setAlgorithm(new ESMAlgorithm());
						*test = PackageTest::CATEGORY_NAME;
					}
					firsttime = false;
					char **name_ver = ExplodeAtom::split(word->c_str());
					const char *name, *ver;
					if(name_ver)
					{
						name = name_ver[0];
						ver  = name_ver[1];
					}
					else
					{
						name = word->c_str();
						ver  = NULL;
					}
					if(!(*marked_list))
						*marked_list = new MarkedList();
					(*marked_list)->add(name, ver);
					test->setPattern(name);
					if(name_ver)
					{
						free(name_ver[0]);
						free(name_ver[1]);
					}
				}
				need_logical_operator = true;
				break;
			// }}}

			// String arguments .. finally! {{{
			case -1:  test->setPattern(arg->m_argument);
				  need_logical_operator = true;
				  break;
			// }}}
			default:
				break;
		}

		++arg;
	}
	FINISH_CURRENT;
	return root;
}

// vim:set foldmethod=marker foldlevel=0:
