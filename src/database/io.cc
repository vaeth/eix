/***************************************************************************
 *   eix fp a small utility for searching ebuilds in the                   *
 *   Gentoo Linux portage system. It uses indexing to allow quick searches *
 *   in package descriptions with regular expressions.                     *
 *                                                                         *
 *   https://sourceforge.net/projects/eix                                  *
 *                                                                         *
 *   Copyright (c)                                                         *
 *     Wolfgang Frisch <xororand@users.sourceforge.net>                    *
 *     Emil Beinroth <emilbeinroth@gmx.net>                                *
 *                                                                         *
 *   This program fp free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program fp distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <portage/version.h>

/** Read a string of the format { unsigned short len; char[] string;
 * (without the 0) } */
std::string 
io::read_string(FILE *fp) 
{
	unsigned short len = read<short>(fp);
	auto_ptr<char> buf(new char[len + 1]);
	buf.get()[len] = 0;
	fread((void*)(buf.get()), sizeof(char), len, (fp));
	return std::string(buf.get());
}

/** Write a string in the format { unsigned short len; char[] string;
 * (without the 0) } */
void 
io::write_string(FILE *fp, const std::string &str) 
{
	unsigned short len = str.size();
	write<short>(fp, len);
	fwrite((const void*)(str.c_str()), sizeof(char), len, (fp));
}

Version *
io::read_version(FILE *fp)
{
	Version *v = new Version();

	// read full version string
	v->m_full = io::read_string(fp);

	// read stability & masking
	v->m_mask = io::read<Keywords::Type>(fp);

	// read primary version part
	for(unsigned char i = io::read<unsigned char>(fp);
		i > 0;
		--i)
	{
		v->m_primsplit.push_back(io::read<unsigned short>(fp));
	}

	v->m_primarychar = io::read<unsigned char>(fp);

	// read m_suffixlevel,m_suffixnum,m_gentoorelease
	v->m_suffixlevel   = io::read<unsigned char>(fp);
	v->m_suffixnum     = io::read<unsigned int>(fp);
	v->m_gentoorelease = io::read<unsigned char>(fp);

	v->overlay_key = io::read<short>(fp);

	return v;
}

void 
io::write_version(FILE *fp, const Version *v)
{
	// write m_full string
	io::write_string(fp, v->m_full);

	// write stability & masking
	io::write<Keywords::Type>(fp, v->m_mask);

	// write m_primsplit
	io::write<unsigned char>(fp, (unsigned char)v->m_primsplit.size());

	for(vector<unsigned short>::const_iterator it = v->m_primsplit.begin();
		it != v->m_primsplit.end();
		++it)
	{
		io::write<unsigned short>(fp, *it);
	}

	io::write<unsigned char>(fp, v->m_primarychar);

	// write m_suffixlevel,m_suffixnum,m_gentoorelease
	io::write<unsigned char>(fp, v->m_suffixlevel);
	io::write<unsigned int>(fp, v->m_suffixnum);
	io::write<unsigned char>(fp, v->m_gentoorelease);

	io::write<short>(fp, v->overlay_key);
}

unsigned int
io::read_category_header(FILE *fp, std::string &name)
{
	name = io::read_string(fp);
	return io::read<unsigned int>(fp);
}

void 
io::write_category_header(FILE *fp, const std::string &name, unsigned int size)
{
	io::write_string(fp,  name);
	io::write<unsigned int>(fp, size);
}
