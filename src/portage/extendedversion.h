// vim:set noet cinoptions=g0,t0,^-2 sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef EIX__EXTENDEDVERSION_H__
#define EIX__EXTENDEDVERSION_H__ 1

#include <portage/basicversion.h>
#include <portage/depend.h>
#include <database/types.h>
#include <eixTk/inttypes.h>

#include <string>

#include <cstddef>

class Package;
class PortageSettings;

class ExtendedVersion : public BasicVersion
{
	private:
		typedef io::UChar HaveBinPkg;
		static const HaveBinPkg
			HAVEBINPKG_UNKNOWN = 0x00,
			HAVEBINPKG_NO      = 0x01,
			HAVEBINPKG_YES     = 0x02;
		mutable HaveBinPkg have_bin_pkg_m; // mutable: it is just a cache

	public:
		typedef uint16_t Restrict;
		static const Restrict // order according to frequency...
			RESTRICT_NONE           = 0x0000,
			RESTRICT_BINCHECKS      = 0x0001,
			RESTRICT_STRIP          = 0x0002,
			RESTRICT_TEST           = 0x0004,
			RESTRICT_USERPRIV       = 0x0008,
			RESTRICT_INSTALLSOURCES = 0x0010,
			RESTRICT_FETCH          = 0x0020,
			RESTRICT_MIRROR         = 0x0040,
			RESTRICT_PRIMARYURI     = 0x0080,
			RESTRICT_BINDIST        = 0x0100,
			RESTRICT_PARALLEL       = 0x0200;

		typedef io::UChar Properties;
		static const Properties // order according to frequency...
			PROPERTIES_NONE        = 0x00,
			PROPERTIES_INTERACTIVE = 0x01,
			PROPERTIES_LIVE        = 0x02,
			PROPERTIES_VIRTUAL     = 0x04,
			PROPERTIES_SET         = 0x08;

		Restrict restrictFlags;
		Properties propertiesFlags;

		/** The slot, the version represents.
		    For saving space, the default "0" is always stored as "" */
		std::string slotname;

		/** The repository name */
		std::string reponame;

		/** The dependencies */
		Depend depend;

		typedef io::UNumber Overlay;
		/** Key for Portagedb.overlays/overlaylist from header. */
		Overlay overlay_key;

		ExtendedVersion(const char *str = NULL) :
			BasicVersion(str),
			have_bin_pkg_m(HAVEBINPKG_UNKNOWN),
			restrictFlags(RESTRICT_NONE),
			propertiesFlags(PROPERTIES_NONE),
			overlay_key(0)
		{ }

		static Restrict calcRestrict(const std::string& str);
		static Properties calcProperties(const std::string& str);

		void set_restrict(const std::string& str)
		{ restrictFlags = calcRestrict(str); }

		void set_properties(const std::string& str)
		{ propertiesFlags = calcProperties(str); }

		bool have_bin_pkg(const PortageSettings *ps, const Package *pkg) const;
};

#endif /* EIX__EXTENDEDVERSION_H__ */
