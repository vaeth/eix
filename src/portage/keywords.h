// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef EIX__STABILITY_H__
#define EIX__STABILITY_H__ 1

#include <database/types.h>
#include <eixTk/inttypes.h>

#include <set>
#include <string>

#include <cstddef>

class MaskFlags {
	public:
		typedef io::UChar MaskType;
		static const MaskType
			MASK_NONE               = 0x00,
			MASK_PACKAGE            = 0x01,
			MASK_PROFILE            = 0x02,
			MASK_HARD               = MASK_PACKAGE|MASK_PROFILE,
			MASK_SYSTEM             = 0x04,
			MASK_WORLD              = 0x08,
			MASK_WORLD_SETS         = 0x10;

		MaskFlags(MaskType t = MASK_NONE)
		{ m_mask = t; }

		void set(MaskType t)
		{ m_mask = t; }

		MaskType get() const
		{ return m_mask; }

		MaskType getall(MaskType t) const
		{ return (m_mask & t); }

		bool havesome(MaskType t) const
		{ return (m_mask & t); }

		bool haveall(MaskType t) const
		{ return ((m_mask & t) == t); }

		void setbits(MaskType t)
		{ m_mask |= t; }

		void clearbits(MaskType t)
		{ m_mask &= ~t; }

		bool isHardMasked() const
		{ return havesome(MaskFlags::MASK_HARD); }
		/** @return true if version is masked by profile. */
		bool isProfileMask() const
		{ return havesome(MaskFlags::MASK_PROFILE); }
		/** @return true if version is masked by a package.mask. */
		bool isPackageMask() const
		{ return havesome(MaskFlags::MASK_PACKAGE); }
		/** @return true if version is part of a package that is a system-package. */
		bool isSystem() const
		{ return havesome(MaskFlags::MASK_SYSTEM); }
		/** @return true if version is part of world. */
		bool isWorld() const
		{ return havesome(MaskFlags::MASK_WORLD); }
		/** @return true if version is part of world sets. */
		bool isWorldSets() const
		{ return havesome(MaskFlags::MASK_WORLD_SETS); }

	protected:
		MaskType m_mask;
};

inline bool operator == (MaskFlags const& left, MaskFlags const& right)
{ return (left.get() == right.get()); }

inline bool operator != (MaskFlags const& left, MaskFlags const& right)
{ return (left.get() != right.get()); }

class KeywordsFlags {
	public:
		typedef io::UChar KeyType;
		static const KeyType
			KEY_EMPTY          = 0x00,
			KEY_STABLE         = 0x01, /**< stabilized */
			KEY_ARCHSTABLE     = 0x02, /**<  ARCH  */
			KEY_ARCHUNSTABLE   = 0x04, /**< ~ARCH  */
			KEY_ALIENSTABLE    = 0x08, /**<  ALIEN */
			KEY_ALIENUNSTABLE  = 0x10, /**< ~ALIEN */
			KEY_MINUSKEYWORD   = 0x20, /**< -ARCH  */
			KEY_MINUSUNSTABLE  = 0x40, /**< -~*    */
			KEY_MINUSASTERISK  = 0x80, /**<  -*    */
			KEY_SOMESTABLE     = KEY_ARCHSTABLE|KEY_ALIENSTABLE,
			KEY_SOMEUNSTABLE   = KEY_ARCHUNSTABLE|KEY_ALIENUNSTABLE,
			KEY_TILDESTARMATCH = KEY_SOMESTABLE|KEY_ARCHUNSTABLE|KEY_ALIENUNSTABLE;

		static KeyType get_keyflags(const std::set<std::string> &accepted_keywords, const std::string &keywords, bool obsolete_minus);

		KeywordsFlags(KeyType t = KEY_EMPTY)
		{ m_keyword = t; }

		void set(KeyType t)
		{ m_keyword = t; }

		KeyType get() const
		{ return m_keyword; }

		KeyType getall(KeyType t) const
		{ return (m_keyword & t); }

		bool havesome(KeyType t) const
		{ return (m_keyword & t); }

		bool haveall(KeyType t) const
		{ return ((m_keyword & t) == t); }

		void setbits(KeyType t)
		{ m_keyword |= t; }

		void clearbits(KeyType t)
		{ m_keyword &= ~t; }

		/** @return true if version is marked stable. */
		bool isStable() const
		{ return havesome(KEY_STABLE); }

		/** @return true if version is unstable. */
		bool isUnstable() const
		{ return havesome(KEY_ARCHUNSTABLE); }

		/** @return true if version is masked by -* keyword. */
		bool isMinusAsterisk() const
		{ return havesome(KEY_MINUSASTERISK); }

		/** @return true if version is masked by -~* keyword. */
		bool isMinusUnstable() const
		{ return havesome(KEY_MINUSUNSTABLE); }

		/** @return true if version is masked by -keyword. */
		bool isMinusKeyword() const
		{ return havesome(KEY_MINUSKEYWORD); }

		/** @return true if version is masked by ALIENARCH */
		bool isAlienStable() const
		{ return havesome(KEY_ALIENSTABLE); }

		/** @return true if version is masked by ~ALIENARCH */
		bool isAlienUnstable() const
		{ return havesome(KEY_ALIENUNSTABLE); }

		/** @return true if version is masked (only) by missing keyword */
		bool isMissingKeyword() const
		{ return ((m_keyword == KEY_EMPTY) || (m_keyword == KEY_ARCHSTABLE)); }

	protected:
		KeyType m_keyword;
};

inline bool operator == (const KeywordsFlags& left, const KeywordsFlags& right)
{ return (left.get() == right.get()); }

inline bool operator != (const KeywordsFlags& left, const KeywordsFlags& right)
{ return (left.get() != right.get()); }

class Keywords
{
	public:
		typedef uint32_t Redundant;
		static const Redundant
			RED_NOTHING       = 0x000000, /**< None of the following           */
			RED_DOUBLE        = 0x000001, /**< Same keyword twice              */
			RED_DOUBLE_LINE   = 0x000002, /**< Same keyword line twice         */
			RED_MIXED         = 0x000004, /**< Weaker and stronger keyword     */
			RED_WEAKER        = 0x000008, /**< Unnecessarily strong keyword    */
			RED_STRANGE       = 0x000010, /**< Unrecognized OTHERARCH or -OTHERARCH */
			RED_NO_CHANGE     = 0x000020, /**< No change in keyword status     */
			RED_MINUSASTERISK = 0x000040, /**< Usage of -* in package.keywords */
			RED_IN_KEYWORDS   = 0x000080, /**< Some entry in package.keywords  */
			RED_ALL_KEYWORDS  = RED_DOUBLE|RED_DOUBLE_LINE|RED_MIXED|RED_WEAKER|RED_STRANGE|RED_NO_CHANGE|RED_MINUSASTERISK|RED_IN_KEYWORDS,
			RED_MASK          = 0x000100, /**< No change in mask status        */
			RED_DOUBLE_MASK   = 0x000200, /**< Double mask entry               */
			RED_IN_MASK       = 0x000400, /**< Some entry in package.mask      */
			RED_UNMASK        = 0x000800, /**< No change in unmask status      */
			RED_DOUBLE_UNMASK = 0x001000, /**< Double unmask entry             */
			RED_IN_UNMASK     = 0x002000, /**< Some entry in package.umask     */
			RED_ALL_MASK      = RED_MASK|RED_DOUBLE_MASK|RED_IN_MASK,
			RED_ALL_UNMASK    = RED_UNMASK|RED_DOUBLE_UNMASK|RED_IN_UNMASK,
			RED_ALL_MASKSTUFF = RED_ALL_MASK|RED_ALL_UNMASK,
			RED_DOUBLE_USE    = 0x004000, /**< Double entry in package.use     */
			RED_IN_USE        = 0x008000, /**< Some entry in package.use       */
			RED_ALL_USE       = RED_DOUBLE_USE|RED_IN_USE,
			RED_DOUBLE_ENV    = 0x010000, /**< Double entry in package.env     */
			RED_IN_ENV        = 0x020000, /**< Some entry in package.env       */
			RED_ALL_ENV       = RED_DOUBLE_ENV|RED_IN_ENV,
			RED_DOUBLE_LICENSE= 0x040000, /**< Some entry in package.license   */
			RED_IN_LICENSE    = 0x080000, /**< Double entry in package.license */
			RED_ALL_LICENSE   = RED_DOUBLE_LICENSE|RED_IN_LICENSE,
			RED_DOUBLE_CFLAGS = 0x100000, /**< Some entry in package.cflags    */
			RED_IN_CFLAGS     = 0x200000, /**< Double entry in package.cflags  */
			RED_ALL_CFLAGS    = RED_DOUBLE_CFLAGS|RED_IN_CFLAGS;

		KeywordsFlags keyflags;
		MaskFlags maskflags;

		Keywords(KeywordsFlags::KeyType k = KeywordsFlags::KEY_EMPTY, MaskFlags::MaskType m = MaskFlags::MASK_NONE) :
			keyflags(k), maskflags(m)
		{
			redundant = RED_NOTHING;
			red_mask = 0x00;
		}

		/** Add/substract modify keys to/from original to obtain result.
		 *  @return false if certainly no modifications occur. In this case
		 *  the result is not modified */
		static bool modify_keywords(std::string &result, const std::string &original, const std::string &modify_keys);

		void set_redundant(Redundant or_redundant = true)
		{ redundant |= or_redundant; }

		Redundant get_redundant () const
		{ return redundant; }

		void set_was_masked(bool value = true)
		{ if(value) red_mask |= 0x01; else red_mask &= ~0x01; }

		bool was_masked () const
		{ return (red_mask & 0x01); }

		void set_was_unmasked(bool value = true)
		{ if(value) red_mask |= 0x02; else red_mask &= ~0x02; }

		bool was_unmasked () const
		{ return (red_mask & 0x02); }

		void set_wanted_masked(bool value = true)
		{ if(value) red_mask |= 0x04; else red_mask &= ~0x04; }

		bool wanted_masked () const
		{ return (red_mask & 0x04); }

		void set_wanted_unmasked(bool value = true)
		{ if(value) red_mask |= 0x08; else red_mask &= ~0x08; }

		bool wanted_unmasked () const
		{ return (red_mask & 0x08); }

	protected:
		Redundant redundant;
		char red_mask; ///< temporary redundant-related stuff during mask testing

};

class KeywordSave {
		KeywordsFlags saved_keyflags;
		MaskFlags     saved_maskflags;
		bool have_data;
	public:
		KeywordSave(const Keywords *k = NULL)
		{ store(k); }

		void store(const Keywords *k = NULL)
		{
			if(!k) {
				have_data = false;
				return;
			}
			have_data = true;
			saved_keyflags  = k->keyflags;
			saved_maskflags = k->maskflags;
		}

		void restore(Keywords *k) const
		{
			if(!have_data)
				return;
			k->keyflags  = saved_keyflags;
			k->maskflags = saved_maskflags;
		}
};


enum LocalMode { LOCALMODE_DEFAULT=0, LOCALMODE_LOCAL, LOCALMODE_NONLOCAL };

#endif /* EIX__STABILITY_H__ */
