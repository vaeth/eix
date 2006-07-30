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

#include "../config.h"
#include "cli.h"

Matchatom *
parse_cli(EixRc &eixrc, VarDbPkg &varpkg_db, PortageSettings &portagesettings, ArgumentReader::iterator arg, ArgumentReader::iterator end)
{
	/* Our root Matchatom. */
	Matchatom   *root    = new Matchatom();
	Matchatom   *current = root;
	PackageTest *test    = new PackageTest(&varpkg_db);

	bool need_logical_operator = false;
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
				next = current->AND();
			}

			if(next != NULL)
			{
				current->setTest(test);
				current->finalize();
				need_logical_operator = false;
				current = next;
				test = new PackageTest(&varpkg_db);
				continue;
			}
		}
		// }}}

		EixRc::RedPair red;
		switch(**arg)
		{
			// Check local options {{{
			case 'I': test->Installed();    break;
			case 'D': test->DuplVersions(); break;
			case 'T': eixrc.getRedundantFlags("REDUNDANT_IF_DOUBLE",
					Keywords::RED_DOUBLE, red);
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
				  test->ObsoleteCfg(portagesettings, red.first, red.second);
				  break;
			case '!': test->Invert();       break;
			// }}}

			// Check for field-designators {{{
			case 's': *test |= PackageTest::NAME;          break;
			case 'C': *test |= PackageTest::CATEGORY;      break;
			case 'A': *test |= PackageTest::CATEGORY_NAME; break;
			case 'S': *test |= PackageTest::DESCRIPTION;   break;
			case 'L': *test |= PackageTest::LICENSE;       break;
			case 'H': *test |= PackageTest::HOMEPAGE;      break;
			case 'P': *test |= PackageTest::PROVIDE;       break;
			// }}}

			// Check for algorithms {{{
			case 'f':
					  if(++arg != end
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

			// String arguments .. finally! {{{
			case -1:  test->setPattern(arg->m_argument);
					  need_logical_operator = true;
					  break;
			// }}}
		}

		++arg;
	}
	current->setTest(test);
	current->finalize();
	return root;
}

// vim:set foldmethod=marker foldlevel=0:
