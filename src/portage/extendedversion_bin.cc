// vim:set noet cinoptions=g0,t0,(0 sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include "extendedversion.h"
#include <eixTk/sysutils.h>
#include <portage/conf/portagesettings.h>
#include <portage/package.h>

#include <string>

using namespace std;

bool
ExtendedVersion::have_bin_pkg(const PortageSettings *ps, const Package *pkg) const
{
	switch(have_bin_pkg_m) {
		case HAVEBINPKG_UNKNOWN:
			{
				const string &s((*ps)["PKGDIR"]);
				if((s.empty()) || !is_file((s + "/" + pkg ->category + "/" + pkg->name + "-" + getFull() + ".tbz2").c_str())) {
					have_bin_pkg_m = HAVEBINPKG_NO;
					return false;
				}
				have_bin_pkg_m = HAVEBINPKG_YES;
			}
			break;
		case HAVEBINPKG_NO:
			return false;
		default:
		// case HAVEBINPKG_YES;
			break;
	}
	return true;
}
