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

#ifndef EIX__IO_H__
#define EIX__IO_H__

#include <string>

namespace io {

	/** Read any POD. */
	template<typename _Tp> _Tp read(FILE *fp) {
		_Tp ret;
		fread((void*)&(ret), sizeof(_Tp), 1, fp);
		return ret;
	}

	/** Write any POD. */
	template<typename _Tp> void write(FILE *fp, const _Tp t) {
		fwrite((const void*)&(t), sizeof(_Tp), 1, fp);
	}

	/** Read a string of the format { unsigned short len; char[] string; (without the 0) } */
	inline std::string read_string(FILE *fp) {
		unsigned short len = read<short>(fp);
		auto_ptr<char> buf(new char[len + 1]);
		buf.get()[len] = 0;
		fread((void*)(buf.get()), sizeof(char), len, (fp));
		return std::string(buf.get());
	}

	/** Write a string in the format { unsigned short len; char[] string; (without the 0) } */
	inline void write_string(FILE *fp, const std::string &str) {
		unsigned short len = str.size();
		write<short>(fp, len);
		fwrite((const void*)(str.c_str()), sizeof(char), len, (fp));
	}

}

#endif /* EIX__IO_H__ */
