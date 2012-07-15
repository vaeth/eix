// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef EIX__OVERLAY_H__
#define EIX__OVERLAY_H__ 1

#include <eixTk/null.h>

#include <map>
#include <string>
#include <vector>

class OverlayIdent {
	private:
		void readLabel_internal(const char *Path);
	public:
		bool know_path, know_label;
		std::string path, label;

		OverlayIdent(const char *Path, const char *Label = NULLPTR);

		void readLabel(const char *Path = NULLPTR)
		{ if(!know_label) readLabel_internal(Path); }

		void setLabel(const std::string &Label)
		{ label = Label; know_label = true; }

		std::string human_readable() const;
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

		RepoList::iterator find_filename(const char *search, bool parent_ok = false, bool resolve_mask = true);

		RepoList::const_iterator second() const
		{
			RepoList::const_iterator i(begin());
			if(i != end()) {
				++i;
			}
			return i;
		}

		void clear()
		{ trust_cache = true; super::clear(); cache.clear(); }

		void push_back(const char *s)
		{ trust_cache = false; super::push_back(OverlayIdent(s)); }
};

#endif /* EIX__OVERLAY_H__ */
