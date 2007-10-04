// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)                                                         
//   Wolfgang Frisch <xororand@users.sourceforge.net>                    
//   Emil Beinroth <emilbeinroth@gmx.net>                                
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>                     

#ifndef EIX__IO_H__
#define EIX__IO_H__

#include <string>

#if !defined(__OpenBSD__)
#include <stdint.h>
#endif

#include <unistd.h>

class Package;
class Version;
class LeadNum;
class PackageTree;
class DBHeader;

namespace io {
	typedef uint8_t Char;
	const unsigned short Charsize    = 1;
	typedef uint16_t Short;
	const unsigned short Shortsize   = 2;
	typedef uint32_t Int;
	const unsigned short Intsize     = 4;
//	Better do not use this (might not work on e.g. 16 bit archs):
//	typedef uint64_t Long;
	const unsigned short Longsize    = 8;

	typedef io::Short Catsize;
	const unsigned short Catsizesize = io::Shortsize;
	typedef io::Short Versize;
	const unsigned short Versizesize = io::Shortsize;
	typedef io::Int Treesize;
	const unsigned short Treesizesize = io::Intsize;

	/// Read any POD.
	template<typename _Tp> _Tp
	read(const unsigned short size, FILE *fp)
	{
		_Tp ret = _Tp(fgetc(fp)) & 0xFF;
		for(unsigned short i = 1, shift = 8; i<size; i++, shift += 8) {
			ret |= (_Tp(fgetc(fp)) & 0xFF) << shift;
		}
		return ret;
	}

	/// Write any POD.
	template<typename _Tp> void
	write(const unsigned short size, FILE *fp, _Tp t)
	{
		for(unsigned short i = 1; ; i++)
		{
			fputc(t & 0xFF, fp);
			if(i < size)
				t >>= 8;
			else
				return;
		}
	}

	/// Read a string of the format { unsigned short len; char[] string;
	// (without the 0) }
	std::string read_string(FILE *fp);

	/// Write a string in the format { unsigned short len; char[] string;
	// (without the 0) }
	void write_string(FILE *fp, const std::string &str);

	/// Read a number with leading zero's
	LeadNum read_LeadNum(FILE *fp);

	/// Read a version from fp
	Version *read_version(FILE *fp);

	/// Write a number with leading zero's
	void write_LeadNum(FILE *fp, const LeadNum &n);

	// Write a version to fp
	void write_version(FILE *fp, const Version *v);


	// Read a category-header from fp
	io::Treesize read_category_header(FILE *fp, std::string &name);

	// Write a category-header to fp
	void write_category_header(FILE *fp, const std::string &name, io::Treesize size);


	// Write package to stream
	void write_package(FILE *fp, const Package &pkg);

	void write_header(FILE *fp, const DBHeader &hdr);
	void read_header(FILE *fp, DBHeader &hdr);

	void write_packagetree(FILE *fp, const PackageTree &pkg);
	void read_packagetree(FILE *fp, io::Treesize size, PackageTree &tree);
}

#endif /* EIX__IO_H__ */
