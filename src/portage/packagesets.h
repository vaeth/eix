// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#ifndef SRC_PORTAGE_PACKAGESETS_H_
#define SRC_PORTAGE_PACKAGESETS_H_ 1

#include <vector>
#include <string>

typedef std::vector<std::string>::size_type SetsIndex;

class SetsList : public std::vector<SetsIndex> {
	private:
		bool have_system;

	public:
		typedef std::vector<SetsIndex> super;

		SetsList() : have_system(false) {
		}

		explicit SetsList(bool with_system) : have_system(with_system) {
		}

		bool has_system() const {
			return have_system;
		}

		/// @return true if something has changed
		bool add_system();

		bool has(SetsIndex i) const ATTRIBUTE_PURE;

		/// @return true if something has changed
		bool add(SetsIndex i);

		/// @return true if something has changed
		bool add(const SetsList& l);

		void clear();
};

#endif  // SRC_PORTAGE_PACKAGESETS_H_
