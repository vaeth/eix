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

#include <config.h>

#include <string>
#include <vector>

#include "eixTk/attribute.h"
#include "eixTk/eixint.h"
#include "eixTk/null.h"
#include "eixTk/ptr_container.h"
#include "eixTk/stringlist.h"
#include "portage/basicversion.h"
#include "portage/keywords.h"
#include "portage/packagesets.h"
#include "portage/version.h"

class ExtendedVersion;
class Package;

// A category which may not occur (i.e. which is reserved for maskIsSet)
#define SET_CATEGORY "@/"

/**
A class for parsing masking definitions like those in the profile and
/etc/portage/package.(un)mask
**/
class Mask : public BasicVersion {
	public:
		/**
		Describes the type of a mask.
		Nothing specific, entry in "packages"-file but not in
		system-profile, entry in packages-file and in system-profile,
		entry in package.mask, entry in package.unmask
		**/
		typedef enum {
			maskTypeNone, maskInProfile,
			maskInSystem, maskInWorld,
			maskMask, maskUnmask,
			maskPseudomask, maskMark, maskMarkOptional
		} Type;

		/**
		Describes the comparison operator before the mask.
		**/
		typedef enum {
			maskOpAll, maskOpEqual,
			maskOpLess, maskOpLessEqual,
			maskOpGreaterEqual, maskOpGreater,
			maskOpRevisions, maskOpGlob, maskOpGlobExt,
			maskIsSet
		} Operator;

		eix::TinyUnsigned priority;
		StringList comments;

	protected:
		Operator m_operator;     ///< Operator for mask.
		Type m_type;             ///< Mask type for this mask.

		std::string m_category;  ///< category
		std::string m_name;      ///< package name
		std::string m_slotname;
		std::string m_subslotname;
		std::string m_reponame;
		std::string m_glob;      ///< the glob string for MaskOpGlobExt
		bool m_test_slot;        ///< must we match a slot?
		bool m_test_subslot;     ///< must we match a subslot?
		bool m_test_reponame;    ///< must we match a reponame?


		/**
		Test if the mask applies to a Version.
		@param ev test this version
		@return true if applies.
		**/
		ATTRIBUTE_NONNULL_ bool test(const ExtendedVersion *ev) const;

	public:
		typedef eix::ptr_container<std::vector<Version *> > Matches;

		/**
		Set the stability & masked members of ve according to the mask
		@param ve         Version instance to be set
		@param do_test    set conditionally or unconditionally
		@param check      check these for changes
		**/
		ATTRIBUTE_NONNULL_ void apply(Version *ve, bool do_test, Keywords::Redundant check) const;

		explicit Mask(Type type) :
			m_type(type),
			m_test_slot(false),
			m_test_subslot(false),
			m_test_reponame(false) {
		}

		/**
		split a "mask string" into its components
		@return whether/which error occurred
		@param str_mask the string to be dissected
		@param errtext contains error message if not 0 and not parseOK
		@param accept_garbage passed to parseVersion if appropriate
		**/
		ATTRIBUTE_NONNULL((2, 3)) BasicVersion::ParseResult parseMask(const char *str, std::string *errtext, eix::SignedBool accept_garbage, const char *default_repo);
		ATTRIBUTE_NONNULL_ BasicVersion::ParseResult parseMask(const char *str, std::string *errtext, eix::SignedBool accept_garbage) {
			return parseMask(str, errtext, accept_garbage, NULLPTR);
		}
		ATTRIBUTE_NONNULL_ BasicVersion::ParseResult parseMask(const char *str, std::string *errtext) {
			return parseMask(str, errtext, 1, NULLPTR);
		}

		ATTRIBUTE_NONNULL_ void match(Matches *m, Package *pkg) const;

		bool have_match(const Package& pkg) const;

		ATTRIBUTE_NONNULL_ void to_package(Package *p) const;

		const char *getName() const {
			return m_name.c_str();
		}

		const char *getCategory() const {
			return m_category.c_str();
		}

		Type get_type() const {
			return m_type;
		}

		/**
		Set the stability members of all version in package according to the mask.
		@param pkg            package you want tested
		@param check          Redundancy checks which should apply
		**/
		ATTRIBUTE_NONNULL_ void checkMask(Package *pkg, Keywords::Redundant check) const;
		ATTRIBUTE_NONNULL_ void checkMask(Package *pkg) const {
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

		KeywordMask() :
			Mask(maskTypeNone),
			locally_double(false) {
		}

		ATTRIBUTE_NONNULL_ void applyItem(Package *pkg) const;

		ATTRIBUTE_NONNULL_ void applyItem(Version *ver) const {
			ver->add_accepted_keywords(keywords);
		}
};

class PKeywordMask : public Mask {
	public:
		std::string keywords;

		PKeywordMask() : Mask(maskTypeNone) {
		}

		ATTRIBUTE_NONNULL_ void applyItem(Package *pkg) const;

		ATTRIBUTE_NONNULL_ void applyItem(Version *ver) const {
			ver->modify_effective_keywords(keywords);
		}
};

class SetMask : public Mask {
	public:
		SetsIndex m_set;

		explicit SetMask(SetsIndex set_index) : Mask(maskTypeNone), m_set(set_index) {
		}

		ATTRIBUTE_NONNULL_ void applyItem(Package *pkg) const;
};

#endif  // SRC_PORTAGE_MASK_H_
