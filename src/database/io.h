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
	typedef uint8_t  UChar;
	typedef uint16_t UShort;
	typedef uint32_t UInt;

	typedef io::UShort Catsize;
	typedef io::UShort Versize;
	typedef io::UInt   Treesize;

	/// Read any POD.
	template<typename Type> Type
	read(FILE *fp)
	{
		Type ret = Type(fgetc(fp)) & 0xFF;
		for(unsigned short i = 1, shift = 8; i<sizeof(Type); i++, shift += 8) {
			ret |= (Type(fgetc(fp)) & 0xFF) << shift;
		}
		return ret;
	}

	/// Write any POD.
	template<typename Type> void
	write(FILE *fp, Type t)
	{
		for(unsigned short i = 1; ; i++)
		{
			fputc(t & 0xFF, fp);
			if(i < sizeof(Type))
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
