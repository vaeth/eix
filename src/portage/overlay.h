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

#include <string>

#include <cstddef>

class OverlayIdent {
	public:
		bool know_path, know_label;
		std::string path, label;

		OverlayIdent(const char *Path, const char *Label = NULL);

		void readLabel(const char *Path = NULL);

		void setLabel(const std::string &Label)
		{ label = Label; know_label = true; }

		std::string human_readable() const;
};


#endif /* EIX__OVERLAY_H__ */
