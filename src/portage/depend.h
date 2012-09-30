// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef SRC_PORTAGE_DEPEND_H_
#define SRC_PORTAGE_DEPEND_H_ 1

#include <cstdio>

#include <string>

#include "database/io.h"

class DBHeader;
class Version;
class PackageTree;

class Depend
{
	friend bool io::read_depend(Depend *dep, const DBHeader &hdr, FILE *fp, std::string *errtext) ATTRIBUTE_NONNULL((1, 3));
	friend bool io::write_depend(const Depend &dep, const DBHeader &hdr, FILE *fp, std::string *errtext);
	friend void io::prep_header_hashs(DBHeader *hdr, const PackageTree& tree) ATTRIBUTE_NONNULL_;

	private:
		std::string m_depend, m_rdepend, m_pdepend, m_hdepend;

		static const char c_depend[];
		static const char c_rdepend[];

		static std::string subst(const std::string &in, const std::string &text);

	public:
		static bool use_depend;

		void set(const std::string &depend, const std::string &rdepend, const std::string &pdepend, const std::string &hdepend, bool normspace);

		std::string get_depend() const
		{ return subst(m_depend, m_rdepend); }

		std::string get_depend_brief() const
		{ return subst(m_depend, c_rdepend); }

		std::string get_rdepend() const
		{ return subst(m_rdepend, m_depend); }

		std::string get_rdepend_brief() const
		{ return subst(m_rdepend, c_depend); }

		std::string get_pdepend() const
		{ return m_pdepend; }

		std::string get_pdepend_brief() const
		{ return m_pdepend; }

		std::string get_hdepend() const
		{ return m_hdepend; }

		std::string get_hdepend_brief() const
		{ return m_hdepend; }

		bool depend_empty() const
		{ return m_depend.empty(); }

		bool rdepend_empty() const
		{ return m_rdepend.empty(); }

		bool pdepend_empty() const
		{ return m_pdepend.empty(); }

		bool hdepend_empty() const
		{ return m_hdepend.empty(); }

		bool empty() const
		{ return (m_depend.empty() && m_rdepend.empty() && m_pdepend.empty() && m_hdepend.empty()); }

		void clear()
		{
			m_depend.clear();
			m_rdepend.clear();
			m_pdepend.clear();
			m_hdepend.clear();
		}
};

#endif  // SRC_PORTAGE_DEPEND_H_
