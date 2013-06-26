// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef SRC_PORTAGE_OVERLAY_H_
#define SRC_PORTAGE_OVERLAY_H_ 1

#include <map>
#include <string>
#include <vector>

#include "eixTk/inttypes.h"
#include "eixTk/null.h"

class OverlayIdent {
	private:
		void readLabel_internal(const char *Path);

	public:
		bool know_path, know_label;
		std::string path, label;
		typedef int64_t Priority;
		Priority priority;
		bool is_main;

		OverlayIdent(const char *Path, const char *Label = NULLPTR, Priority prio = 0, bool ismain = false) ATTRIBUTE_NONNULL((2));

		void readLabel(const char *Path = NULLPTR)
		{
			if(!know_label) {
				readLabel_internal(Path);
			}
		}

		void setLabel(const std::string &Label)
		{
			label = Label;
			know_label = true;
		}

		std::string human_readable() const;

		static void init_static();
};

class RepoList : public std::vector<OverlayIdent> {
	private:
		bool trust_cache;
		std::map<std::string, std::string> cache;

	public:
		typedef std::vector<OverlayIdent> super;

		RepoList() : trust_cache(true)
		{ }

		const char *get_path(const std::string &label);

		RepoList::iterator find_filename(const char *search, bool parent_ok = false, bool resolve_mask = true) ATTRIBUTE_NONNULL_;

		void set_priority(OverlayIdent *overlay);

		RepoList::const_iterator second() const
		{
			RepoList::const_iterator i(begin());
			if(i != end()) {
				++i;
			}
			return i;
		}

		void push_back(const OverlayIdent &s, bool no_path_dupes = false);

		void clear();
};

#endif  // SRC_PORTAGE_OVERLAY_H_
