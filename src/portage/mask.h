// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#ifndef SRC_PORTAGE_MASK_H_
#define SRC_PORTAGE_MASK_H_ 1

#include <string>

#include "eixTk/eixint.h"
#include "eixTk/ptr_list.h"
#include "eixTk/stringlist.h"
#include "portage/basicversion.h"
#include "portage/keywords.h"
#include "portage/packagesets.h"
#include "portage/version.h"

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
			maskPseudomask, maskMark, maskMarkOptional
		} Type;

		/** Describes the comparison operator before the mask. */
		typedef enum {
			maskOpAll, maskOpEqual,
			maskOpLess, maskOpLessEqual,
			maskOpGreaterEqual, maskOpGreater,
			maskOpRevisions, maskOpGlob, maskOpGlobExt,
			maskIsSet
		} Operator;

		StringList comments;

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
		bool test(const ExtendedVersion *ev) const ATTRIBUTE_NONNULL_;

	public:
		typedef eix::ptr_list<Version> Matches;

		/** Sets the stability & masked members of ve according to the mask
		 * @param ve         Version instance to be set
		 * @param do_test    set conditionally or unconditionally
		 * @param check      check these for changes */
		void apply(Version *ve, bool do_test, Keywords::Redundant check) const ATTRIBUTE_NONNULL_;

		Mask(Type type, const char *repo);

		explicit Mask(Type type) :
			m_type(type),
			m_test_slot(false),
			m_test_subslot(false),
			m_test_reponame(false) {
		}

		/** split a "mask string" into its components
		 * @return whether/which error occurred
		 * @param str_mask the string to be dissected
		 * @param errtext contains error message if not 0 and not parseOK
		 * @param accept_garbage passed to parseVersion if appropriate */
		BasicVersion::ParseResult parseMask(const char *str, std::string *errtext, eix::SignedBool accept_garbage) ATTRIBUTE_NONNULL_;
		BasicVersion::ParseResult parseMask(const char *str, std::string *errtext) ATTRIBUTE_NONNULL_ {
			return parseMask(str, errtext, 1);
		}

		void match(Matches *m, Package *pkg) const ATTRIBUTE_NONNULL_;

		bool have_match(const Package& pkg) const;

		void to_package(Package *p) const ATTRIBUTE_NONNULL_;

		const char *getName() const {
			return m_name.c_str();
		}

		const char *getCategory() const {
			return m_category.c_str();
		}

		Type get_type() const {
			return m_type;
		}

		/** Sets the stability members of all version in package according to the mask.
		 * @param pkg            package you want tested
		 * @param check          Redundancy checks which should apply */
		void checkMask(Package *pkg, Keywords::Redundant check) const ATTRIBUTE_NONNULL_;
		void checkMask(Package *pkg) const ATTRIBUTE_NONNULL_ {
			checkMask(pkg, Keywords::RED_NOTHING);
		}

		bool ismatch(const Package& pkg) const;

		bool is_set() const {
			return (m_operator == maskIsSet);
		}
};

class KeywordMask : public Mask {
	public:
		std::string keywords;
		bool locally_double;

		explicit KeywordMask(const char *repo) :
			Mask(maskTypeNone, repo),
			locally_double(false) {
		}

		explicit KeywordMask() :
			Mask(maskTypeNone),
			locally_double(false) {
		}

		void applyItem(Package *pkg) const ATTRIBUTE_NONNULL_;

		void applyItem(Version *ver) const ATTRIBUTE_NONNULL_ {
			ver->add_accepted_keywords(keywords);
		}
};

class PKeywordMask : public Mask {
	public:
		std::string keywords;

		explicit PKeywordMask(const char *repo) : Mask(maskTypeNone, repo) {
		}

		explicit PKeywordMask() : Mask(maskTypeNone) {
		}

		void applyItem(Package *pkg) const ATTRIBUTE_NONNULL_;

		void applyItem(Version *ver) const ATTRIBUTE_NONNULL_ {
			ver->modify_effective_keywords(keywords);
		}
};

class SetMask : public Mask {
	public:
		SetsIndex m_set;

		SetMask(SetsIndex set_index, const char *repo) : Mask(maskTypeNone, repo), m_set(set_index) {
		}

		explicit SetMask(SetsIndex set_index) : Mask(maskTypeNone), m_set(set_index) {
		}

		void applyItem(Package *pkg) const ATTRIBUTE_NONNULL_;
};

#endif  // SRC_PORTAGE_MASK_H_
