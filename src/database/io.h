// vim:set noet cinoptions= sw=4 ts=4:
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
#include <cstdint>
#endif

#include <unistd.h>

class Package;
class Version;
class LeadNum;
class PackageTree;
class DBHeader;

#define MAGICNUMCHAR 0xFF

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

	/// Read a nonnegative number from fp (_Tp must be big enough)
	template<typename _Tp> _Tp
	read(FILE *fp)
	{
		UChar c = readUChar(fp);
		// The one-byte case is exceptional w.r.t. to leading 0:
		if(c != MAGICNUMCHAR)
			return c;
		size_t toget = 1;
		while((c = readUChar(fp)) == MAGICNUMCHAR)
			++toget;
		_Tp ret;
		if(c)
			ret = c;
		else { // leading 0 after MAGICNUMCHAR:
			ret = _Tp(MAGICNUMCHAR);
			--toget;
		}
		for(; toget ; --toget) {
			ret = (ret << 8) | _Tp(readUChar(fp));
		}
		return ret;
	}

	/// Write nonnegative number t to fp (undefined behaviour if t < 0)
	template<typename _Tp> void
	write(FILE *fp, _Tp t)
	{
		UChar c = (t & 0xFF);
		// Test the most common case explicitly to speed up:
		if(t == _Tp(c)) {
			writeUChar(fp, c);
			if(c == MAGICNUMCHAR) // write leading 0 as flag:
				writeUChar(fp, 0);
			return;
		}
		_Tp mask = 0xFF;
		unsigned int count = 0;
		do {
			writeUChar(fp, MAGICNUMCHAR);
			mask <<= 8;
			mask |= 0xFF;
			++count;
		} while((t & mask) != t);
		// We have count > 0 here
		UChar d = (t >> (8*(count--))) & 0xFF;
		writeUChar(fp, d);
		if(d == MAGICNUMCHAR) // write leading 0 as flag:
			writeUChar(fp, 0);
		while(count)
			writeUChar(fp, (t >> (8*(count--))) & 0xFF);
		// neither rely on (t>>0)==t nor use count-- when count==0:
		writeUChar(fp, c);
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
