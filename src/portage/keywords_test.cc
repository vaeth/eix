// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)                                                         
//   Wolfgang Frisch <xororand@users.sourceforge.net>                    
//   Emil Beinroth <emilbeinroth@gmx.net>                                
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>                     

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
			KeywordsFlags::KEY_STABLE|KeywordsFlags::KEY_ARCHSTABLE);

	test_keywords("sh", "~alpha ~amd64 arm hppa -ia64 ~mips s390 sh sparc x86",
			KeywordsFlags::KEY_STABLE|KeywordsFlags::KEY_ARCHSTABLE|KeywordsFlags::KEY_ALIENSTABLE|KeywordsFlags::KEY_ALIENUNSTABLE);


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
