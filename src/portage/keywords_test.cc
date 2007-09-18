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
#include <set>
#include <vector>

using namespace std;

void
test_keywords(string arch, string kw, KeywordsFlags::KeyType result)
{
	set<string> a;
	resolve_plus_minus(a, split_string(arch), true);
	if(KeywordsFlags::get_keyflags(a, kw, false) != result)
	{
		cout << "arch: " << arch << endl;
		cout << "keywords: " << kw << endl;
		cout << int(KeywordsFlags::get_keyflags(a, kw, false)) << " " << int(result) << endl;
		exit(1);
	}
}

int main()
{
	/// Test Keyword parsing

	test_keywords("x86", "-* ~alpha ~amd64 arm hppa -ia64 m68k sh ~sparc x86",
			KeywordsFlags::KEY_STABLE|KeywordsFlags::KEY_ARCHSTABLE|KeywordsFlags::KEY_ALIENSTABLE|KeywordsFlags::KEY_ALIENUNSTABLE|KeywordsFlags::KEY_MINUSASTERISK);

	test_keywords("alpha", "-* ~alpha ~amd64 arm hppa -ia64 m68k sh ~sparc x86",
			KeywordsFlags::KEY_ARCHUNSTABLE|KeywordsFlags::KEY_ALIENSTABLE|KeywordsFlags::KEY_ALIENUNSTABLE|KeywordsFlags::KEY_MINUSASTERISK);

	test_keywords("alpha", "-* ~amd64 arm hppa -ia64 m68k sh ~sparc x86",
			KeywordsFlags::KEY_ALIENSTABLE|KeywordsFlags::KEY_ALIENUNSTABLE|KeywordsFlags::KEY_MINUSASTERISK);

	test_keywords("alpha", "-*",
			KeywordsFlags::KEY_MINUSASTERISK);

	test_keywords("alpha", "-alpha",
			KeywordsFlags::KEY_MINUSKEYWORD);

	test_keywords("alpha", "~alpha",
			KeywordsFlags::KEY_ARCHUNSTABLE);

	test_keywords("alpha", "alpha",
			KeywordsFlags::KEY_STABLE);

	test_keywords("sh", "~alpha ~amd64 arm hppa -ia64 ~mips s390 sh sparc x86",
			KeywordsFlags::KEY_STABLE|KeywordsFlags::KEY_ALIENSTABLE|KeywordsFlags::KEY_ALIENUNSTABLE);


	/// Test keyword-class methods

	Keywords kw;

	set<string> a;
	resolve_plus_minus(a, split_string("alpha"), false);

	kw.set_full_keywords("-*");
	kw.set_keyflags(a, true);
	TEST_ASSERT(kw.keyflags.isMinusAsterisk());
	TEST_ASSERT(!kw.keyflags.isStable());
	TEST_ASSERT(!kw.keyflags.isUnstable());
	TEST_ASSERT(!kw.keyflags.isAlienStable());
	TEST_ASSERT(!kw.keyflags.isAlienUnstable());

	kw.set_full_keywords("-alpha");
	kw.set_keyflags(a, true);
	TEST_ASSERT(kw.keyflags.isMinusKeyword());
	TEST_ASSERT(!kw.keyflags.isStable());
	TEST_ASSERT(!kw.keyflags.isUnstable());
	TEST_ASSERT(!kw.keyflags.isAlienStable());
	TEST_ASSERT(!kw.keyflags.isAlienUnstable());

	kw.set_full_keywords("~alpha");
	kw.set_keyflags(a, true);
	TEST_ASSERT(kw.keyflags.isUnstable());
	TEST_ASSERT(!kw.keyflags.isStable());
	TEST_ASSERT(kw.keyflags.isUnstable());
	TEST_ASSERT(!kw.keyflags.isAlienStable());
	TEST_ASSERT(!kw.keyflags.isAlienUnstable());

	kw.set_full_keywords("alpha");
	kw.set_keyflags(a, true);
	TEST_ASSERT(kw.keyflags.isStable());
	TEST_ASSERT(!kw.keyflags.isUnstable());
	TEST_ASSERT(!kw.keyflags.isMinusAsterisk());
	TEST_ASSERT(!kw.keyflags.isAlienStable());
	TEST_ASSERT(!kw.keyflags.isAlienUnstable());

	kw.set_full_keywords("beta");
	kw.set_keyflags(a, true);
	TEST_ASSERT(!kw.keyflags.isStable());
	TEST_ASSERT(!kw.keyflags.isUnstable());
	TEST_ASSERT(kw.keyflags.isAlienStable());
	TEST_ASSERT(!kw.keyflags.isAlienUnstable());

	kw.set_full_keywords("~beta");
	kw.set_keyflags(a, true);
	TEST_ASSERT(!kw.keyflags.isStable());
	TEST_ASSERT(!kw.keyflags.isUnstable());
	TEST_ASSERT(!kw.keyflags.isAlienStable());
	TEST_ASSERT(kw.keyflags.isAlienUnstable());

	return 0;
}
