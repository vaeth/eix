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

#include <eixTk/stringutils.h>
#include <database/io.h>
#include <vector>

class KeywordsFlags {
	public:
		typedef io::Short Type;
		static const unsigned short Typesize = io::Shortsize;
		static const Type
			KEY_EMPTY          = 0x0000,
			KEY_STABLE         = 0x0001, /**<  ARCH  */
			KEY_UNSTABLE       = 0x0002, /**< ~ARCH  */
			KEY_ALIENSTABLE    = 0x0004, /**<  ALIEN */
			KEY_ALIENUNSTABLE  = 0x0008, /**< ~ALIEN */
			KEY_MINUSKEYWORD   = 0x0010, /**< -ARCH  */
			KEY_MINUSASTERISK  = 0x0020, /**<  -*    */
			KEY_ALL            = KEY_STABLE|KEY_UNSTABLE|KEY_ALIENSTABLE|KEY_ALIENUNSTABLE|KEY_MINUSASTERISK|KEY_MINUSKEYWORD,
			PACKAGE_MASK       = 0x0100,
			PROFILE_MASK       = 0x0200,
			SYSTEM_PACKAGE     = 0x0400;

	protected:
		Type m_mask;

	public:
		static Type get_type(std::string arch, std::string keywords);

		KeywordsFlags(Type t = KEY_MINUSKEYWORD)
		{ m_mask = t; }

		void set(Type t)
		{ m_mask = t; }

		Type get() const
		{ return m_mask; }

		/** @return true if version is marked stable. */
		bool isStable() const
		{ return m_mask & KEY_STABLE; }
		/** @return true if version is unstable. */
		bool isUnstable() const
		{ return m_mask & KEY_UNSTABLE; }
		/** @return true if version is masked by -* keyword. */
		bool isMinusAsterisk() const
		{ return m_mask & KEY_MINUSASTERISK; }
		/** @return true if version is masked by -keyword. */
		bool isMinusKeyword() const
		{ return m_mask & KEY_MINUSKEYWORD; }
		/** @return true if version is masked by ALIENARCH */
		bool isAlienStable() const
		{ return m_mask & KEY_ALIENSTABLE; }
		/** @return true if version is masked by ~ALIENARCH */
		bool isAlienUnstable() const
		{ return m_mask & KEY_ALIENUNSTABLE; }

		bool isHardMasked() const
		{ return isPackageMask() || isProfileMask(); }
		/** @return true if version is masked by profile. */
		bool isProfileMask() const
		{ return m_mask & PROFILE_MASK; }
		/** @return true if version is masked by a package.mask. */
		bool isPackageMask() const
		{ return m_mask & PACKAGE_MASK; }
		/** @return true if version is part of a package that is a system-package. */
		bool isSystem() const
		{ return m_mask & SYSTEM_PACKAGE; }

		void operator |= (const KeywordsFlags::Type t)
		{ m_mask |= t; }
		void operator &= (const KeywordsFlags::Type t)
		{ m_mask &= t; }
};

class Keywords : public KeywordsFlags
{
	public:
		typedef uint32_t Redundant;
		static const Redundant
			RED_NOTHING,       /**< None of the following           */
			RED_DOUBLE,        /**< Same keyword twice              */
			RED_DOUBLE_LINE,   /**< Same keyword line twice         */
			RED_MIXED,         /**< Weaker and stronger keyword     */
			RED_WEAKER,        /**< Unnecessarily strong keyword    */
			RED_STRANGE,       /**< Unrecognized OTHERARCH or -OTHERARCH */
			RED_NO_CHANGE,     /**< No change in keyword status     */
			RED_MASK,          /**< No change in mask status        */
			RED_UNMASK,        /**< No change in unmask status      */
			RED_DOUBLE_MASK,   /**< Double mask entry               */
			RED_DOUBLE_UNMASK, /**< Double unmask entry             */
			RED_MINUSASTERISK, /**< Usage of -* in package.keywords */
			RED_IN_KEYWORDS,   /**< Some entry in package.keywords  */
			RED_IN_MASK,       /**< Some entry in package.mask      */
			RED_IN_UNMASK,     /**< Some entry in package.umask     */
			RED_IN_USE,        /**< Some entry in package.use       */
			RED_IN_CFLAGS,     /**< Some entry in package.cflags    */
			RED_DOUBLE_USE,    /**< Double entry in package.use     */
			RED_DOUBLE_CFLAGS, /**< Double entry in package.cflags  */
			RED_ALL_KEYWORDS,
			RED_ALL_MASK,
			RED_ALL_UNMASK,
			RED_ALL_MASKSTUFF,
			RED_ALL_USE,
			RED_ALL_CFLAGS;

	protected:
		std::string full_keywords;
		Redundant redundant;
		char red_mask; ///< temporary redundant-related stuff during mask testing

	public:
		Keywords(Type t = KEY_MINUSKEYWORD) : KeywordsFlags(t)
		{
			full_keywords = "";
			redundant = RED_NOTHING;
			red_mask = 0x00;
		}

		void set(Type t)
		{ KeywordsFlags::set(t); }

		void set(std::string arch, std::string keywords)
		{ full_keywords = keywords; set(get_type(arch, keywords)); }

		std::string get_full_keywords() const
		{ return full_keywords; }

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
};

enum LocalMode { LOCALMODE_DEFAULT=0, LOCALMODE_LOCAL, LOCALMODE_NONLOCAL };

#endif /* __STABILITY_H__ */
