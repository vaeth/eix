// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef EIX__MASK_H__
#define EIX__MASK_H__ 1

#include <eixTk/null.h>
#include <eixTk/ptr_list.h>
#include <portage/basicversion.h>
#include <portage/keywords.h>
#include <portage/packagesets.h>
#include <portage/version.h>
#include <search/packagetest.h>

#include <string>

class ExtendedVersion;
class Package;

// A category which may not occur (i.e. which is reserved for maskIsSet)
#define SET_CATEGORY "@/"

//  >app-shells/bash-3*      <- NOT ALLOWED
//  ~app-shells/bash-3*      <- OK, BUT DOESN'T SELECT bash-3.0-r12; SELECT
//                              ONLY ~app-shells/bash-3
//  =app-shells/bash-3*      <- OK

/** A class for parsing masking definitions
 * like those in the profile and /etc/portage/package.(un)mask */
class Mask : public BasicVersion {
	public:

		/** Describes the type of a mask.
		 * Nothing specific, entry in "packages"-file but not in
		 * system-profile, entry in packages-file and in system-profile,
		 * entry in package.mask, entry in package.unmask */
		typedef enum {
			maskTypeNone, maskAllowedByProfile,
			maskInSystem, maskInWorld,
			maskMask, maskUnmask,
			maskPseudomask
		} Type;

		/** Describes the comparison operator before the mask. */
		typedef enum {
			maskOpAll, maskOpEqual,
			maskOpLess, maskOpLessEqual,
			maskOpGreaterEqual, maskOpGreater,
			maskOpRevisions, maskOpGlob,
			maskIsSet
		} Operator;

	protected:
		Operator m_operator; /**< Operator for mask. */
		Type m_type;   /**< Mask type for this mask. */

		std::string m_category; /**< category */
		std::string m_name;     /**< package name */
		std::string m_slotname;
		std::string m_subslotname;
		std::string m_reponame;
		std::string m_glob;     /**< the glob string for MaskOpGlob */
		bool m_test_slot;       /**< must we match a slot? */
		bool m_test_subslot;    /**< must we match a subslot? */
		bool m_test_reponame;   /**< must we match a reponame? */


		/** Tests if the mask applies to a Version.
		 * @param ev test this version
		 * @return true if applies. */
		bool test(const ExtendedVersion *ev) const;

	public:
		typedef eix::ptr_list<Version> Matches;

		/** Sets the stability & masked members of ve according to the mask
		 * @param ve         Version instance to be set
		 * @param do_test    set conditionally or unconditionally
		 * @param check      check these for changes */
		void apply(Version *ve, bool do_test, Keywords::Redundant check) const;

		/** Parse mask-string. */
		Mask(Type type, const char *repo = NULLPTR);

		/** split a "mask string" into its components
		 * @return whether/which error occurred
		 * @param str_mask the string to be dissected
		 * @param errtext contains error message if not 0 and not parseOK
		 * @param accept_garbage passed to parseVersion if appropriate */
		BasicVersion::ParseResult parseMask(const char *str, std::string *errtext, bool accept_garbage = true);

		void match(Matches &m, Package &pkg) const;

		bool have_match(Package &pkg) const;

		void to_package(Package *p) const;

		const char *getName() const
		{ return m_name.c_str(); }

		const char *getCategory() const
		{ return m_category.c_str(); }

		Type get_type () const
		{ return m_type; }

		/** Sets the stability members of all version in package according to the mask.
		 * @param pkg            package you want tested
		 * @param check          Redundancy checks which should apply */
		void checkMask(Package& pkg, Keywords::Redundant check = Keywords::RED_NOTHING) const;

		bool ismatch(Package& pkg) const;

		bool is_set() const
		{ return (m_operator == maskIsSet); }
};

class KeywordMask : public Mask {
	public:
		std::string keywords;
		bool locally_double;

		KeywordMask(const char *repo = NULLPTR) : Mask(maskTypeNone, repo)
		{ }

		void applyItem(Package& pkg) const;
		void applyItem(Version *ver) const
		{ ver->add_accepted_keywords(keywords); }
};

class PKeywordMask : public Mask {
	public:
		std::string keywords;

		PKeywordMask(const char *repo = NULLPTR) : Mask(maskTypeNone, repo)
		{ }

		void applyItem(Package& pkg) const;
		void applyItem(Version *ver) const
		{ ver->modify_effective_keywords(keywords); }
};

class SetMask : public Mask {
	public:
		SetsIndex m_set;

		SetMask(SetsIndex set_index, const char *repo = NULLPTR) :
			Mask(maskTypeNone, repo), m_set(set_index)
		{ }

		void applyItem(Package& pkg) const;
};


#endif /* EIX__MASK_H__ */
