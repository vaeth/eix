// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>

#ifndef SRC_PORTAGE_DEPEND_H_
#define SRC_PORTAGE_DEPEND_H_ 1

#include <config.h>  // IWYU pragma: keep

#include <string>

class Database;
class DBHeader;
class Version;
class PackageTree;

class Depend {
	friend class Database;

	private:
		std::string m_depend, m_rdepend, m_pdepend, m_bdepend;
		bool obsolete;

		static const char c_depend[];
		static const char c_rdepend[];

		static std::string subst(const std::string& in, const std::string& text, bool obs);

	public:
		static bool use_depend;

		Depend() : obsolete(false) {
		}

		void set(const std::string& depend, const std::string& rdepend, const std::string& pdepend, const std::string& bdepend, bool normspace);

		std::string get_depend() const {
			return subst(m_depend, m_rdepend, obsolete);
		}

		std::string get_depend_brief() const {
			return subst(m_depend, c_rdepend, obsolete);
		}

		std::string get_rdepend() const {
			return subst(m_rdepend, m_depend, obsolete);
		}

		std::string get_rdepend_brief() const {
			return subst(m_rdepend, c_depend, obsolete);
		}

		std::string get_pdepend() const {
			return m_pdepend;
		}

		std::string get_pdepend_brief() const {
			return m_pdepend;
		}

		std::string get_bdepend() const {
			return m_bdepend;
		}

		std::string get_bdepend_brief() const {
			return m_bdepend;
		}

		bool depend_empty() const {
			return m_depend.empty();
		}

		bool rdepend_empty() const {
			return m_rdepend.empty();
		}

		bool pdepend_empty() const {
			return m_pdepend.empty();
		}

		bool bdepend_empty() const {
			return m_bdepend.empty();
		}

		bool empty() const {
			return (m_depend.empty() && m_rdepend.empty() && m_pdepend.empty() && m_bdepend.empty());
		}

		void clear() {
			m_depend.clear();
			m_rdepend.clear();
			m_pdepend.clear();
			m_bdepend.clear();
			obsolete = false;
		}

		bool operator==(const Depend& d) const;

		bool operator!=(const Depend& d) const {
			return !(*this == d);
		}
};


#endif  // SRC_PORTAGE_DEPEND_H_
