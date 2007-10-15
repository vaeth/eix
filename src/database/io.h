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
#include <cstdlib>

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
	typedef unsigned char UChar;
	typedef size_t UNumber;

	typedef UNumber Catsize;
	typedef UNumber Versize;
	typedef UNumber Treesize;

	typedef off_t OffsetType;
	extern OffsetType counter;

	inline static UChar
	readUChar(FILE *fp)
	{ return fgetc(fp); }

	inline static void
	writeUChar(FILE *fp, UChar c)
	{
		if(fp)
			fputc(c, fp);
		else
			counter++;
	}

	/// Read an unsigned number type
	template<typename _Tp> _Tp
	read(FILE *fp)
	{
		_Tp ret = readUChar(fp);
		// Test the most common case explicitly to speed up:
		if(ret != 0xFF)
			return ret;
		size_t toget = 1;
		while((ret = readUChar(fp)) == 0xFF)
			toget++;
		for(; toget ; --toget) {
			ret = (ret << 8) | _Tp(readUChar(fp));
		}
		return ret;
	}

	/// Write an unsigned number type
	template<typename _Tp> void
	write(FILE *fp, _Tp t)
	{
		if(t < 0xFF) {
			writeUChar(fp, t);
			return;
		}
		writeUChar(fp, 0xFF);
		// We shift right 1 byte to avoid overflow
		_Tp lowest = (t & 0xFF);
		t >>= 8;
		_Tp mask = 0xFF;
		unsigned int shift = 0;
		while(t >= mask) {
			writeUChar(fp, 0xFF);
			mask <<= 8;
			shift += 8;
		}
		for(; shift ; shift -= 8) {
			writeUChar(fp, (t >> shift) & 0xFF);
		}
		// do not rely that ( t >> 0 ) == t
		writeUChar(fp, t & 0xFF);
		writeUChar(fp, lowest);
	}

	/// Read a string
	std::string read_string(FILE *fp);

	/// Write a string
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
	void write_package_pure(FILE *fp, const Package &pkg);

	void write_header(FILE *fp, const DBHeader &hdr);
	void read_header(FILE *fp, DBHeader &hdr);

	void write_packagetree(FILE *fp, const PackageTree &pkg);
	void read_packagetree(FILE *fp, io::Treesize size, PackageTree &tree);
}

#endif /* EIX__IO_H__ */
