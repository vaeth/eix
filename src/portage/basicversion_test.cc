// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)                                                         
//   Wolfgang Frisch <xororand@users.sourceforge.net>                    
//   Emil Beinroth <emilbeinroth@gmx.net>                                

#include "basicversion.h"
#include <eixTk/test.h>
#include <algorithm>
#include <list>

// - Pushback unsorted elements to vector
// - Sort vector
// - Pushback sorted elements to a second vector
// - Compare vectors

using namespace std;

int main()
{

	vector<BasicVersion> s1;
	s1.push_back(BasicVersion("3.3.5-r1"));
	s1.push_back(BasicVersion("3.4.5_rc10"));
	s1.push_back(BasicVersion("3.3.6"));
	s1.push_back(BasicVersion("3.4.4-r1"));
	s1.push_back(BasicVersion("3.4.6"));
	s1.push_back(BasicVersion("3.2.3-r4"));
	s1.push_back(BasicVersion("3.3.2-r7"));
	s1.push_back(BasicVersion("3.4.5-r1"));
	s1.push_back(BasicVersion("3.4.5_pre100-r19"));
	s1.push_back(BasicVersion("4.1.0"));
	s1.push_back(BasicVersion("3.4.5_pre100"));
	s1.push_back(BasicVersion("3.4.5"));
	s1.push_back(BasicVersion("4.0.2-r3"));
	s1.push_back(BasicVersion("3.4.5_pre0"));
	s1.push_back(BasicVersion("3.3.5.20050130-r1"));
	s1.push_back(BasicVersion("4.1.0a"));
	s1.push_back(BasicVersion("3.4.5_pre100-r0"));
	s1.push_back(BasicVersion("3.1.1-r2"));
	s1.push_back(BasicVersion("3.4.1-r3"));
	s1.push_back(BasicVersion("3.4.5_rc0"));
	s1.push_back(BasicVersion("3.2.2"));
	s1.push_back(BasicVersion("4.0.3"));
	s1.push_back(BasicVersion("4.1.0z"));
	s1.push_back(BasicVersion("2.95.3-r9"));
	s1.push_back(BasicVersion("1.0_rc1_rc2"));
	s1.push_back(BasicVersion("1.0_rc1"));
	s1.push_back(BasicVersion("1.0_rc1_p1"));
	s1.push_back(BasicVersion("1.0_rc2_rc2"));
	s1.push_back(BasicVersion("1.0_rc2"));
	s1.push_back(BasicVersion("1.0_rc2_p1"));
	s1.push_back(BasicVersion("1.0"));
	s1.push_back(BasicVersion("1.0_p1_rc1"));
	s1.push_back(BasicVersion("1.0_p1"));
	s1.push_back(BasicVersion("1.0_p1_p1"));

	sort(s1.begin(), s1.end());

	vector<BasicVersion> s2;
	s2.push_back(BasicVersion("1.0_rc1_rc2"));
	s2.push_back(BasicVersion("1.0_rc1"));
	s2.push_back(BasicVersion("1.0_rc1_p1"));
	s2.push_back(BasicVersion("1.0_rc2_rc2"));
	s2.push_back(BasicVersion("1.0_rc2"));
	s2.push_back(BasicVersion("1.0_rc2_p1"));
	s2.push_back(BasicVersion("1.0"));
	s2.push_back(BasicVersion("1.0_p1_rc1"));
	s2.push_back(BasicVersion("1.0_p1"));
	s2.push_back(BasicVersion("1.0_p1_p1"));
	s2.push_back(BasicVersion("2.95.3-r9"));
	s2.push_back(BasicVersion("3.1.1-r2"));
	s2.push_back(BasicVersion("3.2.2"));
	s2.push_back(BasicVersion("3.2.3-r4"));
	s2.push_back(BasicVersion("3.3.2-r7"));
	s2.push_back(BasicVersion("3.3.5-r1"));
	s2.push_back(BasicVersion("3.3.5.20050130-r1"));
	s2.push_back(BasicVersion("3.3.6"));
	s2.push_back(BasicVersion("3.4.1-r3"));
	s2.push_back(BasicVersion("3.4.4-r1"));
	s2.push_back(BasicVersion("3.4.5_pre0"));
	s2.push_back(BasicVersion("3.4.5_pre100"));
	s2.push_back(BasicVersion("3.4.5_pre100-r0"));
	s2.push_back(BasicVersion("3.4.5_pre100-r19"));
	s2.push_back(BasicVersion("3.4.5_rc0"));
	s2.push_back(BasicVersion("3.4.5_rc10"));
	s2.push_back(BasicVersion("3.4.5"));
	s2.push_back(BasicVersion("3.4.5-r1"));
	s2.push_back(BasicVersion("3.4.6"));
	s2.push_back(BasicVersion("4.0.2-r3"));
	s2.push_back(BasicVersion("4.0.3"));
	s2.push_back(BasicVersion("4.1.0"));
	s2.push_back(BasicVersion("4.1.0a"));
	s2.push_back(BasicVersion("4.1.0z"));

	for(vector<BasicVersion>::size_type i = 0;
		i < s1.size();
		++i)
	{
		if(s1[i] != s2[i])
		{
#ifdef DEBUG_BASICVERSION
			for(vector<BasicVersion>::iterator it = s1.begin();
				it != s1.end(); ++it)
				cout << it->getFull() << endl;
#endif
			return 1;
		}
	}
	return 0;
}

#if defined(INSTANTIATE_TEMPLATES)
template class vector<BasicVersion>;
#endif
