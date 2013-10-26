// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef SRC_PORTAGE_PACKAGESETS_H_
#define SRC_PORTAGE_PACKAGESETS_H_ 1

#include <vector>
#include <string>
#include <algorithm>

typedef std::vector<std::string>::size_type SetsIndex;

class SetsList : public std::vector<SetsIndex> {
	private:
		bool have_system;

	public:
		explicit SetsList(bool with_system = false) {
			have_system = with_system;
		}

		bool has_system() const {
			return have_system;
		}

		/// @return true if something has changed
		bool add_system() {
			if(have_system) {
				return false;
			}
			have_system = true;
			return true;
		}

		bool has(SetsIndex i) const {
			return (std::find(begin(), end(), i) != end());
		}

		/// @return true if something has changed
		bool add(SetsIndex i) {
			if(has(i)) {
				return false;
			}
			push_back(i);
			return true;
		}

		/// @return true if something has changed
		bool add(const SetsList &l) {
			bool r(false);
			if(l.has_system()) {
				if(add_system()) {
					r = true;
				}
			}
			for(SetsList::const_iterator it = l.begin();
				it != l.end(); ++it) {
				if(add(*it)) {
					r = true;
				}
			}
			return r;
		}

		void clear() {
			std::vector<SetsIndex>::clear();
			have_system = false;
		}
};

#endif  // SRC_PORTAGE_PACKAGESETS_H_
