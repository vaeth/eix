// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#ifndef SRC_EIXTK_AUTO_ARRAY_H_
#define SRC_EIXTK_AUTO_ARRAY_H_ 1

#include <config.h>

#include "eixTk/null.h"

// check_includes: include "eixTk/auto_array.h"

namespace eix {
template<typename m_Type> class auto_array {
	public:
		explicit auto_array(m_Type *p) : m_p(p) {
		}

		~auto_array() {
			if(m_p != NULLPTR) {
				delete[] m_p;
			}
		}

		m_Type* get() const {
			return m_p;
		}

	protected:
		m_Type* m_p;
	};
}  // namespace eix

#endif  // SRC_EIXTK_AUTO_ARRAY_H_
