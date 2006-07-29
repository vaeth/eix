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

#include "keywords.h"

const Keywords::Type
	Keywords::KEY_MISSINGKEYWORD = 0x00,
	Keywords::KEY_STABLE         = 0x01, /*  ARCH */
	Keywords::KEY_UNSTABLE       = 0x02, /* ~ARCH */
	Keywords::KEY_MINUSASTERISK  = 0x04, /*  -*   */
	Keywords::KEY_MINUSKEYWORD   = 0x08, /* -ARCH */
	Keywords::KEY_ALL            = KEY_MISSINGKEYWORD|KEY_STABLE|KEY_UNSTABLE|KEY_MINUSASTERISK|KEY_MINUSKEYWORD,
	Keywords::PACKAGE_MASK       = 0x10,
	Keywords::PROFILE_MASK       = 0x20,
	Keywords::SYSTEM_PACKAGE     = 0x40;

const Keywords::Redundant
	Keywords::RED_NOTHING      = 0x000,
	Keywords::RED_DOUBLE       = 0x001,
	Keywords::RED_MIXED        = 0x002,
	Keywords::RED_WEAKER       = 0x004,
	Keywords::RED_STRANGE      = 0x008,
	Keywords::RED_NO_CHANGE    = 0x010,
	Keywords::RED_ALL_KEYWORDS = RED_DOUBLE|RED_MIXED|RED_WEAKER|RED_STRANGE|RED_NO_CHANGE,
	Keywords::RED_MASK         = 0x100,
	Keywords::RED_UNMASK       = 0x200,
	Keywords::RED_DOUBLE_MASK  = 0x400,
	Keywords::RED_DOUBLE_UNMASK= 0x800,
	Keywords::RED_ALL_MASK     = RED_MASK|RED_DOUBLE_MASK,
	Keywords::RED_ALL_UNMASK   = RED_UNMASK|RED_DOUBLE_UNMASK,
	Keywords::RED_ALL_MASKSTUFF= RED_ALL_MASK|RED_ALL_UNMASK;

