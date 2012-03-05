// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef EIX__DEPEND_H__
#define EIX__DEPEND_H__ 1

#include <database/io.h>

#include <string>

#include <cstdio>

class DBHeader;
class Version;
class PackageTree;

class Depend
{
	friend void io::read_depend(FILE *fp, Depend &dep, const DBHeader &hdr);
	friend void io::write_depend(FILE *fp, const Depend &dep, const DBHeader &hdr);
	friend void io::prep_header_hashs(DBHeader &hdr, const PackageTree &tree);

	private:
		static const std::string the_same;
		std::string m_depend, m_rdepend, m_pdepend;

	public:
		static bool use_depend;

		void set(const std::string &depend, const std::string &rdepend, const std::string &pdepend, bool trimspace);

		std::string get_depend() const
		{ return m_depend; }

		std::string get_rdepend() const
		{ return ((m_rdepend == the_same) ? m_depend : m_rdepend); }

		std::string get_pdepend() const
		{ return m_pdepend; }

		void clear()
		{ m_depend.clear(); m_rdepend.clear(); m_pdepend.clear(); }
};

#endif /* EIX__DEPEND_H__ */
