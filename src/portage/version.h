// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef __VERSION_H__
#define __VERSION_H__

#include <database/io.h>
#include <portage/basicversion.h>
#include <portage/keywords.h>
#include <portage/packagesets.h>

#include <eixTk/stringutils.h>

#include <algorithm>

/* If NOT_FULL_USE is defined, then the iuse data will be handled per package
   and not per version to save memory and disk space.
   More precisely, if NOT_FULL_USE is defined then the version::m_iuse entry
   will be empty most of the time:
   The entry is cleared in Package::collect_iuse() which is called by
   Package::addVersionFinalize() / Package::addVersion()
   whenever a version is added to the package: Before clearing,
   collect_iuse() adds the corresponding data to the package-wide data.
   If on the other hand, NOT_FULL_USE is undefined, collect_iuse() will not
   delete this data, and the database-output function will write an empty
   string for the package-wide IUSE data, and the database-reading function
   will get forced to read all package versions (using Package::addVersion()
   and thus calculating the package-wide IUSE) before the package-wide
   IUSE data is assumed to be known. */

/*#define NOT_FULL_USE*/

/** Version expands the BasicVersion class by data relevant for versions in tree/overlays.
 */
class Version : public ExtendedVersion, public Keywords {
	public:
		friend void     io::write_version(FILE *fp, const Version *v, const DBHeader &hdr);
		friend Version *io::read_version(FILE *fp, const DBHeader &hdr);

		typedef io::UNumber Overlay;

		/** Key for Portagedb.overlays/overlaylist from header. */
		Overlay overlay_key;

		typedef enum {
			SAVEKEY_USER, SAVEKEY_ACCEPT, SAVEKEY_ARCH, SAVEKEY_SIZE
		} SavedKeyIndex;

		typedef enum {
			SAVEMASK_USER, SAVEMASK_USERFILE, SAVEMASK_USERPROFILE,
			SAVEMASK_PROFILE, SAVEMASK_FILE, SAVEMASK_SIZE
		} SavedMaskIndex;

		std::vector<KeywordsFlags> saved_keywords;
		std::vector<bool>          have_saved_keywords;
		std::vector<MaskFlags>     saved_masks;
		std::vector<bool>          have_saved_masks;

		std::vector<SetsIndex> sets_indizes;

		Version() : overlay_key(0),
			saved_keywords(SAVEKEY_SIZE, KeywordsFlags()),
			have_saved_keywords(SAVEKEY_SIZE, false),
			saved_masks(SAVEMASK_SIZE, MaskFlags()),
			have_saved_masks(SAVEMASK_SIZE, false)
		{ }

		/** Constructor, calls BasicVersion::parseVersion( str ) */
		Version(const char* str) : ExtendedVersion(str), overlay_key(0),
			saved_keywords(SAVEKEY_SIZE, KeywordsFlags()),
			have_saved_keywords(SAVEKEY_SIZE, false),
			saved_masks(SAVEMASK_SIZE, MaskFlags()),
			have_saved_masks(SAVEMASK_SIZE, false)
		{ }

		void save_keyflags(SavedKeyIndex i)
		{ have_saved_keywords[i] = true; saved_keywords[i] = keyflags; }

		void save_maskflags(SavedMaskIndex i)
		{ have_saved_masks[i] = true; saved_masks[i] = maskflags; }

		bool restore_keyflags(SavedKeyIndex i)
		{
			if(have_saved_keywords[i]) {
				keyflags = saved_keywords[i];
				return true;
			}
			return false;
		}

		bool restore_maskflags(SavedMaskIndex i)
		{
			if(have_saved_masks[i]) {
				maskflags = saved_masks[i];
				return true;
			}
			return false;
		}

		void set_iuse(const std::string &i);

		const std::string& iuse() const;

		const std::vector<std::string>& iuse_vector() const
		{ return m_iuse; }

		bool is_in_set(SetsIndex m_set) const
		{ return (std::find(sets_indizes.begin(), sets_indizes.end(), m_set) != sets_indizes.end()); }

		void add_to_set(SetsIndex m_set)
		{
			if(!is_in_set(m_set))
				sets_indizes.push_back(m_set);
		}

	protected:
		/** If NOT_FULL_USE is defined, this might "falsely" be empty
		    to save memory. See the comments above NOT_FULL_USE. */
		std::vector<std::string> m_iuse;

		/// joint strings from m_iuse; clear if you change m_iuse.
		mutable std::string m_cached_iuse;
};

/** The equality operator does *not* test the slots */
inline bool operator == (const Version& left, const Version &right)
{ return (!BasicVersion::compare(left, right)) && (left.overlay_key == right.overlay_key); }
inline bool operator != (const Version& left, const Version &right)
{ return (!BasicVersion::compare(left, right)) || (left.overlay_key != right.overlay_key); }

#endif /* __VERSION_H__ */
