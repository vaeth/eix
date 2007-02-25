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

#include "keywords.h"
#include <eixTk/test.h>

using namespace std;

void
test_keywords(string arch, string kw, Keywords::Type result)
{
	if(Keywords::get_type(arch, kw) != result)
	{
		cout << "arch: " << arch << endl;
		cout << "keywords: " << kw << endl;
		exit(1);
	}
}

int main()
{
	/// Test Keyword parsing

	test_keywords("x86", "-* ~alpha ~amd64 arm hppa -ia64 m68k sh ~sparc x86",
			Keywords::KEY_STABLE|Keywords::KEY_ALIENSTABLE|Keywords::KEY_ALIENUNSTABLE|Keywords::KEY_MINUSASTERISK);

	test_keywords("alpha", "-* ~alpha ~amd64 arm hppa -ia64 m68k sh ~sparc x86",
			Keywords::KEY_UNSTABLE|Keywords::KEY_ALIENSTABLE|Keywords::KEY_ALIENUNSTABLE|Keywords::KEY_MINUSASTERISK);

	test_keywords("alpha", "-* ~amd64 arm hppa -ia64 m68k sh ~sparc x86",
			Keywords::KEY_ALIENSTABLE|Keywords::KEY_ALIENUNSTABLE|Keywords::KEY_MINUSASTERISK);

	test_keywords("alpha", "-*",
			Keywords::KEY_MINUSASTERISK);

	test_keywords("alpha", "-alpha",
			Keywords::KEY_MINUSKEYWORD);

	test_keywords("alpha", "~alpha",
			Keywords::KEY_UNSTABLE);

	test_keywords("alpha", "alpha",
			Keywords::KEY_STABLE);

	test_keywords("sh", "~alpha ~amd64 arm hppa -ia64 ~mips s390 sh sparc x86",
			Keywords::KEY_STABLE|Keywords::KEY_ALIENSTABLE|Keywords::KEY_ALIENUNSTABLE);


	/// Test keyword-class methods

	Keywords kw;

	kw.set("alpha", "-*");
	TEST_ASSERT(kw.isMinusAsterisk());
	TEST_ASSERT(!kw.isStable());
	TEST_ASSERT(!kw.isUnstable());
	TEST_ASSERT(!kw.isAlienStable());
	TEST_ASSERT(!kw.isAlienUnstable());

	kw.set("alpha", "-alpha");
	TEST_ASSERT(kw.isMinusKeyword());
	TEST_ASSERT(!kw.isStable());
	TEST_ASSERT(!kw.isUnstable());
	TEST_ASSERT(!kw.isAlienStable());
	TEST_ASSERT(!kw.isAlienUnstable());

	kw.set("alpha", "~alpha");
	TEST_ASSERT(kw.isUnstable());
	TEST_ASSERT(!kw.isStable());
	TEST_ASSERT(kw.isUnstable());
	TEST_ASSERT(!kw.isAlienStable());
	TEST_ASSERT(!kw.isAlienUnstable());

	kw.set("alpha", "alpha");
	TEST_ASSERT(kw.isStable());
	TEST_ASSERT(!kw.isUnstable());
	TEST_ASSERT(!kw.isMinusAsterisk());
	TEST_ASSERT(!kw.isAlienStable());
	TEST_ASSERT(!kw.isAlienUnstable());

	kw.set("alpha", "beta");
	TEST_ASSERT(!kw.isStable());
	TEST_ASSERT(!kw.isUnstable());
	TEST_ASSERT(kw.isAlienStable());
	TEST_ASSERT(!kw.isAlienUnstable());


	kw.set("alpha", "~beta");
	TEST_ASSERT(!kw.isStable());
	TEST_ASSERT(!kw.isUnstable());
	TEST_ASSERT(!kw.isAlienStable());
	TEST_ASSERT(kw.isAlienUnstable());

	return 0;
}
