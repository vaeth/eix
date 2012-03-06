// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include "depend.h"
#include <eixTk/stringutils.h>

#include <string>

using namespace std;

bool Depend::use_depend;

const std::string Depend::the_same("\"");

void
Depend::set(const string &depend, const string &rdepend, const string &pdepend, bool normspace)
{
	if(!use_depend) {
		return;
	}
	m_depend = depend;
	m_rdepend = rdepend;
	m_pdepend = pdepend;
	if(normspace) {
		trimall(m_depend);
		trimall(m_rdepend);
		trimall(m_pdepend);
	}
	if((!m_depend.empty()) && (m_depend == m_rdepend)) {
		m_rdepend = the_same;
	}
}
