// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef SRC_PORTAGE_KEYWORDS_H_
#define SRC_PORTAGE_KEYWORDS_H_ 1

#include <string>

#include "eixTk/constexpr.h"
#include "eixTk/eixint.h"
#include "eixTk/inttypes.h"
#include "eixTk/null.h"
#include "eixTk/stringtypes.h"

class MaskFlags {
	public:
		typedef eix::UChar MaskType;
		static CONSTEXPR MaskType
			MASK_NONE               = 0x00U,
			MASK_PACKAGE            = 0x01U,
			MASK_PROFILE            = 0x02U,
			MASK_HARD               = MASK_PACKAGE|MASK_PROFILE,
			MASK_SYSTEM             = 0x04U,
			MASK_WORLD              = 0x08U,
			MASK_WORLD_SETS         = 0x10U,
			MASK_MARKED             = 0x20U;

		MaskFlags() : m_mask(MASK_NONE) {
		}

		explicit MaskFlags(MaskType t) : m_mask(t) {
		}

		void set(MaskType t) {
			m_mask = t;
		}

		MaskType get() const {
			return m_mask;
		}

		MaskType getall(MaskType t) const {
			return (m_mask & t);
		}

		bool havesome(MaskType t) const {
			return (m_mask & t);
		}

		bool haveall(MaskType t) const {
			return ((m_mask & t) == t);
		}

		void setbits(MaskType t) {
			m_mask |= t;
		}

		void clearbits(MaskType t) {
			m_mask &= ~t;
		}

		bool isHardMasked() const {
			return havesome(MaskFlags::MASK_HARD);
		}

		/** @return true if version is masked by profile. */
		bool isProfileMask() const {
			return havesome(MaskFlags::MASK_PROFILE);
		}

		/** @return true if version is masked by a package.mask. */
		bool isPackageMask() const {
			return havesome(MaskFlags::MASK_PACKAGE);
		}

		/** @return true if version is part of a package that is a system-package. */
		bool isSystem() const {
			return havesome(MaskFlags::MASK_SYSTEM);
		}

		/** @return true if version is part of world. */
		bool isWorld() const {
			return havesome(MaskFlags::MASK_WORLD);
		}

		/** @return true if version is part of world sets. */
		bool isWorldSets() const {
			return havesome(MaskFlags::MASK_WORLD_SETS);
		}

		/** @return true if version is marked. */
		bool isMarked() const {
			return havesome(MaskFlags::MASK_MARKED);
		}

	protected:
		MaskType m_mask;
};

inline static bool operator==(MaskFlags const& left, MaskFlags const& right) {
	return (left.get() == right.get());
}

inline static bool operator!=(MaskFlags const& left, MaskFlags const& right) {
	return (left.get() != right.get());
}

class KeywordsFlags {
	public:
		typedef eix::UChar KeyType;
		static CONSTEXPR KeyType
			KEY_EMPTY          = 0x00U,
			KEY_STABLE         = 0x01U,  /**< stabilized */
			KEY_ARCHSTABLE     = 0x02U,  /**<  ARCH  */
			KEY_ARCHUNSTABLE   = 0x04U,  /**< ~ARCH  */
			KEY_ALIENSTABLE    = 0x08U,  /**<  ALIEN */
			KEY_ALIENUNSTABLE  = 0x10U,  /**< ~ALIEN */
			KEY_MINUSKEYWORD   = 0x20U,  /**< -ARCH  */
			KEY_MINUSUNSTABLE  = 0x40U,  /**< -~*    */
			KEY_MINUSASTERISK  = 0x80U,  /**<  -*    */
			KEY_SOMESTABLE     = KEY_ARCHSTABLE|KEY_ALIENSTABLE,
			KEY_SOMEUNSTABLE   = KEY_ARCHUNSTABLE|KEY_ALIENUNSTABLE,
			KEY_TILDESTARMATCH = KEY_SOMESTABLE|KEY_SOMEUNSTABLE;

		static KeyType get_keyflags(const WordSet& accepted_keywords, const std::string& keywords);

		explicit KeywordsFlags() : m_keyword(KEY_EMPTY) {
		}

		explicit KeywordsFlags(KeyType t) : m_keyword(t) {
		}

		void set_keyflags(KeyType t) {
			m_keyword = t;
		}

		KeyType get() const {
			return m_keyword;
		}

		KeyType getall(KeyType t) const {
			return (m_keyword & t);
		}

		bool havesome(KeyType t) const {
			return (m_keyword & t);
		}

		bool haveall(KeyType t) const {
			return ((m_keyword & t) == t);
		}

		void setbits(KeyType t) {
			m_keyword |= t;
		}

		void clearbits(KeyType t) {
			m_keyword &= ~t;
		}

		/** @return true if version is marked stable. */
		bool isStable() const {
			return havesome(KEY_STABLE);
		}

		/** @return true if version is unstable. */
		bool isUnstable() const {
			return havesome(KEY_ARCHUNSTABLE);
		}

		/** @return true if version is masked by -* keyword. */
		bool isMinusAsterisk() const {
			return havesome(KEY_MINUSASTERISK);
		}

		/** @return true if version is masked by -~* keyword. */
		bool isMinusUnstable() const {
			return havesome(KEY_MINUSUNSTABLE);
		}

		/** @return true if version is masked by -keyword. */
		bool isMinusKeyword() const {
			return havesome(KEY_MINUSKEYWORD);
		}

		/** @return true if version is masked by ALIENARCH */
		bool isAlienStable() const {
			return havesome(KEY_ALIENSTABLE);
		}

		/** @return true if version is masked by ~ALIENARCH */
		bool isAlienUnstable() const {
			return havesome(KEY_ALIENUNSTABLE);
		}

		/** @return true if version is masked (only) by missing keyword */
		bool isMissingKeyword() const {
			return ((m_keyword == KEY_EMPTY) || (m_keyword == KEY_ARCHSTABLE));
		}

	protected:
		KeyType m_keyword;
};

inline static bool operator==(const KeywordsFlags& left, const KeywordsFlags& right) {
	return (left.get() == right.get());
}

inline static bool operator!=(const KeywordsFlags& left, const KeywordsFlags& right) {
	return (left.get() != right.get());
}

class Keywords {
	public:
		typedef uint32_t Redundant;
		static CONSTEXPR Redundant
			RED_NOTHING         = 0x000000U,  /**< None of the following           */
			RED_DOUBLE          = 0x000001U,  /**< Same keyword twice              */
			RED_DOUBLE_LINE     = 0x000002U,  /**< Same keyword line twice         */
			RED_MIXED           = 0x000004U,  /**< Weaker and stronger keyword     */
			RED_WEAKER          = 0x000008U,  /**< Unnecessarily strong keyword    */
			RED_STRANGE         = 0x000010U,  /**< Unrecognized OTHERARCH or -OTHERARCH */
			RED_NO_CHANGE       = 0x000020U,  /**< No change in keyword status     */
			RED_IN_KEYWORDS     = 0x000040U,  /**< Some entry in package.keywords  */
			RED_ALL_KEYWORDS    = RED_DOUBLE|RED_DOUBLE_LINE|RED_MIXED|RED_WEAKER|RED_STRANGE|RED_NO_CHANGE|RED_IN_KEYWORDS,
			RED_MASK            = 0x000080U,  /**< No change in mask status        */
			RED_DOUBLE_MASK     = 0x000100U,  /**< Double mask entry               */
			RED_IN_MASK         = 0x000200U,  /**< Some entry in package.mask      */
			RED_UNMASK          = 0x000400U,  /**< No change in unmask status      */
			RED_DOUBLE_UNMASK   = 0x000800U,  /**< Double unmask entry             */
			RED_IN_UNMASK       = 0x001000U,  /**< Some entry in package.umask     */
			RED_ALL_MASK        = RED_MASK|RED_DOUBLE_MASK|RED_IN_MASK,
			RED_ALL_UNMASK      = RED_UNMASK|RED_DOUBLE_UNMASK|RED_IN_UNMASK,
			RED_ALL_MASKSTUFF   = RED_ALL_MASK|RED_ALL_UNMASK,
			RED_DOUBLE_USE      = 0x002000U,  /**< Double entry in package.use     */
			RED_IN_USE          = 0x004000U,  /**< Some entry in package.use       */
			RED_ALL_USE         = RED_DOUBLE_USE|RED_IN_USE,
			RED_DOUBLE_ENV      = 0x008000U,  /**< Double entry in package.env     */
			RED_IN_ENV          = 0x010000U,  /**< Some entry in package.env       */
			RED_ALL_ENV         = RED_DOUBLE_ENV|RED_IN_ENV,
			RED_DOUBLE_LICENSE  = 0x020000U,  /**< Some entry in package.license   */
			RED_IN_LICENSE      = 0x040000U,  /**< Double entry in package.license */
			RED_ALL_LICENSE     = RED_DOUBLE_LICENSE|RED_IN_LICENSE,
			RED_DOUBLE_RESTRICT = 0x080000U,  /**< Some entry in package.accept_restrict */
			RED_IN_RESTRICT     = 0x100000U,  /**< Double entry in package.accept_restrict */
			RED_ALL_RESTRICT    = RED_DOUBLE_RESTRICT|RED_IN_RESTRICT,
			RED_DOUBLE_CFLAGS   = 0x200000U,  /**< Some entry in package.cflags    */
			RED_IN_CFLAGS       = 0x400000U,  /**< Double entry in package.cflags  */
			RED_ALL_CFLAGS      = RED_DOUBLE_CFLAGS|RED_IN_CFLAGS;

		KeywordsFlags keyflags;
		MaskFlags maskflags;

		Keywords()
			: keyflags(KeywordsFlags::KEY_EMPTY), maskflags(MaskFlags::MASK_NONE),
			redundant(RED_NOTHING), red_mask(0x00U) {
		}

		Keywords(KeywordsFlags::KeyType k, MaskFlags::MaskType m)
			: keyflags(k), maskflags(m), redundant(RED_NOTHING), red_mask(0x00U) {
		}

		/** Add/substract modify keys to/from original to obtain result.
		 *  @return false if certainly no modifications occur. In this case
		 *  the result is not modified */
		static bool modify_keywords(std::string *result, const std::string &original, const std::string &modify_keys);

		void set_redundant(Redundant or_redundant) {
			redundant |= or_redundant;
		}

		Redundant get_redundant() const {
			return redundant;
		}

		void set_was_masked() {
			red_mask |= 0x01U;
		}

		bool was_masked() const {
			return (red_mask & 0x01U);
		}

		void set_was_unmasked() {
			red_mask |= 0x02U;
		}

		bool was_unmasked() const {
			return (red_mask & 0x02U);
		}

		void set_wanted_masked() {
			red_mask |= 0x04U;
		}

		bool wanted_masked() const {
			return (red_mask & 0x04U);
		}

		void set_wanted_unmasked() {
			red_mask |= 0x08U;
		}

		bool wanted_unmasked() const {
			return (red_mask & 0x08U);
		}

	protected:
		Redundant redundant;
		char red_mask;  /**< temporary redundant-related stuff during mask testing */
};

class KeywordSave {
		KeywordsFlags saved_keyflags;
		MaskFlags     saved_maskflags;
		bool have_data;

	public:
		KeywordSave() : have_data(false) {
		}

		explicit KeywordSave(const Keywords *k) {
			store(k);
		}

		void store(const Keywords *k) {
			if(k == NULLPTR) {
				have_data = false;
				return;
			}
			have_data = true;
			saved_keyflags  = k->keyflags;
			saved_maskflags = k->maskflags;
		}

		void restore(Keywords *k) const {
			if(!have_data) {
				return;
			}
			k->keyflags  = saved_keyflags;
			k->maskflags = saved_maskflags;
		}
};

enum LocalMode {
	LOCALMODE_DEFAULT = 0,
	LOCALMODE_LOCAL,
	LOCALMODE_NONLOCAL
};

#endif  // SRC_PORTAGE_KEYWORDS_H_
