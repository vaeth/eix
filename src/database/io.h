// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef EIX__IO_H__
#define EIX__IO_H__ 1

#include <database/types.h>
#include <eixTk/exceptions.h>
#include <eixTk/i18n.h>
#include <eixTk/likely.h>
#include <eixTk/stringutils.h>

#include <string>
#include <vector>

#include <cstdio>

class DBHeader;
class IUseSet;
class Package;
class PackageTree;
class PortageSettings;
class Version;

#define MAGICNUMCHAR 0xFF

namespace io {
	inline static UChar
	readUChar(FILE *fp)
	{
		int c(fgetc(fp));
		if(unlikely(c == EOF)) {
			if (feof(fp))
				throw ExBasic(_("error while reading from database: end of file"));
			else
				throw SysError(_("error while reading from database"));
		}
		return UChar(c);
	}

	inline static void
	writeUChar(FILE *fp, UChar c)
	{
		if(fp != NULL) {
			if (fputc(c, fp) == EOF)
				throw SysError(_("error while writing to database"));
		}
		else
			++counter;
	}

	/// Read a nonnegative number from fp (m_Tp must be big enough)
	template<typename m_Tp> m_Tp
	read(FILE *fp)
	{
		UChar c = readUChar(fp);
		// The one-byte case is exceptional w.r.t. to leading 0:
		if(c != MAGICNUMCHAR)
			return c;
		unsigned int toget(1);
		while((c = readUChar(fp)) == MAGICNUMCHAR)
			++toget;
		m_Tp ret;
		if(c)
			ret = c;
		else { // leading 0 after MAGICNUMCHAR:
			ret = m_Tp(MAGICNUMCHAR);
			--toget;
		}
		for(; toget ; --toget) {
			ret = (ret << 8) | m_Tp(readUChar(fp));
		}
		return ret;
	}

	/// Write nonnegative number t to fp (undefined behaviour if t < 0)
	template<typename m_Tp> void
	write(FILE *fp, m_Tp t)
	{
		UChar c(t & 0xFF);
		// Test the most common case explicitly to speed up:
		if(t == m_Tp(c)) {
			writeUChar(fp, c);
			if(c == MAGICNUMCHAR) // write leading 0 as flag:
				writeUChar(fp, 0);
			return;
		}
		m_Tp mask(0xFF);
		unsigned int count(0);
		do {
			writeUChar(fp, MAGICNUMCHAR);
			mask <<= 8;
			mask |= 0xFF;
			++count;
		} while((t & mask) != t);
		// We have count > 0 here
		UChar d((t >> (8*(count--))) & 0xFF);
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

	inline void write_hash_string(FILE *fp, const StringHash& hash, const std::string &s)
	{ io::write<StringHash::size_type>(fp, hash.get_index(s)); }

	inline std::string read_hash_string(FILE *fp, const StringHash& hash)
	{ return hash[io::read<StringHash::size_type>(fp)]; }

	void write_hash_words(FILE *fp, const StringHash& hash, const std::vector<std::string>& words);
	inline void write_hash_words(FILE *fp, const StringHash& hash, const std::string& words)
	{ write_hash_words(fp, hash, split_string(words)); }

	std::string read_hash_words(FILE *fp, const StringHash& hash);

	void read_iuse(FILE *fp, const StringHash& hash, IUseSet &iuse);

	/// Read a version from fp
	Version *read_version(FILE *fp, const DBHeader &hdr);

	// Write a version to fp
	void write_version(FILE *fp, const Version *v, const DBHeader &hdr);

	// Read a category-header from fp
	io::Treesize read_category_header(FILE *fp, std::string &name);

	// Write a category-header to fp
	void write_category_header(FILE *fp, const std::string &name, io::Treesize size);

	// Write package to stream
	void write_package(FILE *fp, const Package &pkg, const DBHeader &hdr);
	void write_package_pure(FILE *fp, const Package &pkg, const DBHeader &hdr);

	void write_hash(FILE *fp, const StringHash& hash);
	void read_hash(FILE *fp, StringHash& hash);
	void prep_header_hashs(DBHeader &hdr, const PackageTree &tree);
	void write_header(FILE *fp, const DBHeader &hdr);
	// return false if version does not match. However, fp is not closed.
	bool read_header(FILE *fp, DBHeader &hdr);

	void write_packagetree(FILE *fp, const PackageTree &pkg, const DBHeader &hdr);
	void read_packagetree(FILE *fp, PackageTree &tree, const DBHeader &hdr, PortageSettings *ps);
}

#endif /* EIX__IO_H__ */
