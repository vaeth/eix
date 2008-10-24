// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include "version.h"

using namespace std;

const string&
Version::iuse() const
{
	if(m_cached_iuse.empty() && ! m_iuse.empty())
		m_cached_iuse = join_vector(m_iuse);
	return m_cached_iuse;
}
