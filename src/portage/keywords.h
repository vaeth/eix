/***************************************************************************
 *   eix is a small utility for searching ebuilds in the                   *
 *   Gentoo Linux portage system. It uses indexing to allow quick searches *
 *   in package descriptions with regular expressions.                     *
 *                                                                         *
 *   https://sourceforge.net/projects/eix                                  *
 *                                                                         *
 *   Copyright (c)                                                         *
 *     Wolfgang Frisch <xororand@users.sourceforge.net>                    *
 *     Emil Beinroth <emilbeinroth@gmx.net>                                *
 *     Martin Väth <vaeth@mathematik.uni-wuerzburg.de>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef __STABILITY_H__
#define __STABILITY_H__

#include <set>
#include <vector>

#include <eixTk/stringutils.h>
#include <database/io.h>

class MaskFlags {
	public:
		typedef io::Char MaskType;
		static const unsigned short MaskTypesize = io::Charsize;
		static const MaskType
			MASK_NONE    = 0x00,
			MASK_PACKAGE = 0x01,
			MASK_PROFILE = 0x02,
			MASK_HARD    = MASK_PACKAGE|MASK_PROFILE,
			MASK_SYSTEM  = 0x04;

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

	protected:
		MaskType m_mask;
};

class KeywordsFlags {
	public:
		typedef io::Char KeyType;
		static const unsigned short KeyTypesize = io::Charsize;
		static const KeyType
			KEY_EMPTY          = 0x00,
			KEY_STABLE         = 0x01, /**< stabilized */
			KEY_ARCHSTABLE     = 0x02, /**<  ARCH  */
			KEY_ARCHUNSTABLE   = 0x04, /**< ~ARCH  */
			KEY_ALIENSTABLE    = 0x08, /**<  ALIEN */
			KEY_ALIENUNSTABLE  = 0x10, /**< ~ALIEN */
			KEY_MINUSKEYWORD   = 0x20, /**< -ARCH  */
			KEY_MINUSASTERISK  = 0x40, /**<  -*    */
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
		{ return havesome(KeywordsFlags::KEY_STABLE); }
		/** @return true if version is unstable. */
		bool isUnstable() const
		{ return havesome(KeywordsFlags::KEY_ARCHUNSTABLE); }
		/** @return true if version is masked by -* keyword. */
		bool isMinusAsterisk() const
		{ return havesome(KeywordsFlags::KEY_MINUSASTERISK); }
		/** @return true if version is masked by -keyword. */
		bool isMinusKeyword() const
		{ return havesome(KeywordsFlags::KEY_MINUSKEYWORD); }
		/** @return true if version is masked by ALIENARCH */
		bool isAlienStable() const
		{ return havesome(KeywordsFlags::KEY_ALIENSTABLE); }
		/** @return true if version is masked by ~ALIENARCH */
		bool isAlienUnstable() const
		{ return havesome(KeywordsFlags::KEY_ALIENUNSTABLE); }

	protected:
		KeyType m_keyword;
};

class Keywords
{
	public:
		typedef uint32_t Redundant;
		static const Redundant
			RED_NOTHING       = 0x00000, /**< None of the following           */
			RED_DOUBLE        = 0x00001, /**< Same keyword twice              */
			RED_DOUBLE_LINE   = 0x00002, /**< Same keyword line twice         */
			RED_MIXED         = 0x00004, /**< Weaker and stronger keyword     */
			RED_WEAKER        = 0x00008, /**< Unnecessarily strong keyword    */
			RED_STRANGE       = 0x00010, /**< Unrecognized OTHERARCH or -OTHERARCH */
			RED_NO_CHANGE     = 0x00020, /**< No change in keyword status     */
			RED_MINUSASTERISK = 0x00040, /**< Usage of -* in package.keywords */
			RED_IN_KEYWORDS   = 0x00080, /**< Some entry in package.keywords  */
			RED_KEYWORD_CARESET = RED_MIXED|RED_WEAKER,
			RED_ALL_KEYWORDS  = RED_DOUBLE|RED_DOUBLE_LINE|RED_MIXED|RED_WEAKER|RED_STRANGE|RED_NO_CHANGE|RED_MINUSASTERISK|RED_IN_KEYWORDS,
			RED_MASK          = 0x00100, /**< No change in mask status        */
			RED_DOUBLE_MASK   = 0x00200, /**< Double mask entry               */
			RED_IN_MASK       = 0x00400, /**< Some entry in package.mask      */
			RED_UNMASK        = 0x00800, /**< No change in unmask status      */
			RED_DOUBLE_UNMASK = 0x01000, /**< Double unmask entry             */
			RED_IN_UNMASK     = 0x02000, /**< Some entry in package.umask     */
			RED_ALL_MASK      = RED_MASK|RED_DOUBLE_MASK|RED_IN_MASK,
			RED_ALL_UNMASK    = RED_UNMASK|RED_DOUBLE_UNMASK|RED_IN_UNMASK,
			RED_ALL_MASKSTUFF = RED_ALL_MASK|RED_ALL_UNMASK,
			RED_DOUBLE_USE    = 0x04000, /**< Double entry in package.use     */
			RED_IN_USE        = 0x08000, /**< Some entry in package.use       */
			RED_ALL_USE       = RED_DOUBLE_USE|RED_IN_USE,
			RED_DOUBLE_CFLAGS = 0x10000, /**< Some entry in package.cflags    */
			RED_IN_CFLAGS     = 0x20000, /**< Double entry in package.cflags  */
			RED_ALL_CFLAGS    = RED_DOUBLE_CFLAGS|RED_IN_CFLAGS;

		KeywordsFlags keyflags;
		MaskFlags maskflags;
	public:
		Keywords(KeywordsFlags::KeyType k = KeywordsFlags::KEY_EMPTY, MaskFlags::MaskType m = MaskFlags::MASK_NONE) :
			keyflags(k), maskflags(m)
		{
			full_keywords = "";
			redundant = RED_NOTHING;
			red_mask = 0x00;
		}

		void set_full_keywords(const std::string &keywords)
		{ full_keywords = keywords; }

		std::string get_full_keywords() const
		{ return full_keywords; }

		void set_keyflags(const std::set<std::string> &accepted_keywords, bool obsolete_minus)
		{ keyflags.set(get_keyflags(accepted_keywords, obsolete_minus)); }

		KeywordsFlags::KeyType get_keyflags(const std::set<std::string> &accepted_keywords, bool obsolete_minus)
		{ return KeywordsFlags::get_keyflags(accepted_keywords, full_keywords, obsolete_minus); }

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
		std::string full_keywords;
		Redundant redundant;
		char red_mask; ///< temporary redundant-related stuff during mask testing

};

enum LocalMode { LOCALMODE_DEFAULT=0, LOCALMODE_LOCAL, LOCALMODE_NONLOCAL };

#endif /* __STABILITY_H__ */
