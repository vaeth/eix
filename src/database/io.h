// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef SRC_DATABASE_IO_H_
#define SRC_DATABASE_IO_H_ 1

#include <cstdio>

#include <string>
#include <vector>

#include "eixTk/eixint.h"
#include "eixTk/likely.h"
#include "eixTk/null.h"
#include "eixTk/stringutils.h"

class DBHeader;
class IUseSet;
class Package;
class PackageTree;
class PortageSettings;
class Depend;
class Version;

#define MAGICNUMCHAR 0xFFU

namespace io {
	extern eix::OffsetType counter;

	void readError(FILE *fp, std::string *errtext);
	void writeError(std::string *errtext);

	inline static bool
	readUChar(eix::UChar *c, FILE *fp, std::string *errtext)
	{
		int ch(fgetc(fp));
		if(likely(ch != EOF)) {
			*c = eix::UChar(ch);
			return true;
		}
		io::readError(fp, errtext);
		return false;
	}

	inline static bool
	writeUChar(eix::UChar c, FILE *fp, std::string *errtext)
	{
		if(fp != NULLPTR) {
			if(unlikely(fputc(c, fp) == EOF)) {
				writeError(errtext);
				return false;
			}
		} else {
			++io::counter;
		}
		return true;
	}

	/// Read a nonnegative number from fp (m_Tp must be big enough)
	template<typename m_Tp> bool
	read_num(m_Tp *ret, FILE *fp, std::string *errtext)
	{
		int ch(fgetc(fp));
		if(likely(ch != EOF)) {
			eix::UChar c = eix::UChar(ch);
			// The one-byte case is exceptional w.r.t. to leading 0:
			if(c != MAGICNUMCHAR) {
				*ret = m_Tp(c);
				return true;
			}
			unsigned int toget(1);
			while(likely((ch = fgetc(fp)) != EOF)) {
				if((c = eix::UChar(ch)) == MAGICNUMCHAR) {
					++toget;
					continue;
				}
				if(c != 0) {
					*ret = m_Tp(c);
				} else {  // leading 0 after MAGICNUMCHAR:
					*ret = m_Tp(MAGICNUMCHAR);
					--toget;
				}
				for(;;) {
					if(toget == 0) {
						return true;
					}
					if(unlikely((ch = fgetc(fp)) == EOF)) {
						break;
					}
					*ret = ((*ret) << 8) | m_Tp(eix::UChar(ch));
					--toget;
				}
				break;
			}
		}
		io::readError(fp, errtext);
		return false;
	}

	/// Write nonnegative number t to fp (undefined behaviour if t < 0)
	template<typename m_Tp> bool
	write_num(m_Tp t, FILE *fp, std::string *errtext)
	{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
		eix::UChar c(t & 0xFFU);
#pragma GCC diagnostic pop
		// Test the most common case explicitly to speed up:
		if(t == m_Tp(c)) {
			if(fp == NULLPTR) {
				if(unlikely(c == MAGICNUMCHAR)) {
					io::counter += 2;
				} else {
					++io::counter;
				}
				return true;
			}
			if(likely(fputc(c, fp) != EOF)) {
				if(likely(c != MAGICNUMCHAR)) {
					return true;
				}
				// write leading 0 as flag:
				if(likely(fputc(0, fp) != EOF)) {
					return true;
				}
			}
		} else {
			m_Tp mask(0xFFU);
			unsigned int count(0);
			do {
				mask <<= 8;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
				mask |= 0xFFU;
#pragma GCC diagnostic pop
				++count;
			} while((t & mask) != t);
			// We have count > 0 here
			if(fp == NULLPTR) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
				eix::UChar d((t >> (8*count)) & 0xFFU);
#pragma GCC diagnostic pop
				io::counter += ((unlikely(d == MAGICNUMCHAR)) ? 2 : 1) + (2 * count);
				return true;
			}
			for(unsigned int r(count); ;) {
				if(unlikely(fputc(MAGICNUMCHAR, fp) == EOF)) {
					break;
				}
				if(--r == 0) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
					eix::UChar d((t >> (8*count)) & 0xFFU);
#pragma GCC diagnostic pop
					if(unlikely(fputc(d, fp) == EOF)) {
						break;
					}
					if(unlikely(d == MAGICNUMCHAR)) {
						// write leading 0 as flag:
						if(unlikely(fputc(0, fp) == EOF)) {
							break;
						}
					}
					// neither rely on (t>>0)==t nor use count-- when count==0:
					while(--count != 0) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
						if(unlikely(fputc((t >> (8*count)) & 0xFFU, fp) == EOF)) {
#pragma GCC diagnostic pop
							break;
						}
					}
					if(likely(count == 0)) {
						if(likely(fputc(c, fp) != EOF)) {
							return true;
						}
					}
					break;
				}
			}
		}
		writeError(errtext);
		return false;
	}

	/// Read a string
	bool read_string(std::string *s, FILE *fp, std::string *errtext);

	/// Write a string
	bool write_string(const std::string &str, FILE *fp, std::string *errtext);

	inline static bool write_hash_string(const StringHash& hash, const std::string &s, FILE *fp, std::string *errtext)
	{ return io::write_num(hash.get_index(s), fp, errtext); }

	inline static bool read_hash_string(const StringHash& hash, std::string *s, FILE *fp, std::string *errtext)
	{
		StringHash::size_type i;
		if(likely(io::read_num(&i, fp, errtext))) {
			*s = hash[i];
			return true;
		}
		return false;
	}

	bool write_hash_words(const StringHash& hash, const std::vector<std::string>& words, FILE *fp, std::string *errtext);
	inline static bool write_hash_words(const StringHash& hash, const std::string& words, FILE *fp, std::string *errtext)
	{ return write_hash_words(hash, split_string(words), fp, errtext); }

	bool read_hash_words(const StringHash& hash, std::vector<std::string> *s, FILE *fp, std::string *errtext);
	bool read_hash_words(const StringHash& hash, std::string *s, FILE *fp, std::string *errtext);

	bool read_iuse(const StringHash& hash, IUseSet *iuse, FILE *fp, std::string *errtext);

	/// Read a version from fp
	bool read_version(Version *&v, const DBHeader &hdr, FILE *fp, std::string *errtext);

	// Write a version to fp
	bool write_version(const Version *v, const DBHeader &hdr, FILE *fp, std::string *errtext);

	// Write dependency information to fp
	bool read_depend(Depend *dep, const DBHeader &hdr, FILE *fp, std::string *errtext);

	// Write dependency information to fp
	bool write_depend(const Depend &dep, const DBHeader &hdr, FILE *fp, std::string *errtext);

	// Read a category-header from fp
	bool read_category_header(std::string *name, eix::Treesize *h, FILE *fp, std::string *errtext);

	// Write a category-header to fp
	bool write_category_header(const std::string &name, eix::Treesize size, FILE *fp, std::string *errtext);

	// Write package to stream
	bool write_package(const Package &pkg, const DBHeader &hdr, FILE *fp, std::string *errtext);
	bool write_package_pure(const Package &pkg, const DBHeader &hdr, FILE *fp, std::string *errtext);

	bool write_hash(const StringHash& hash, FILE *fp, std::string *errtext);
	bool read_hash(StringHash *hash, FILE *fp, std::string *errtext);
	void prep_header_hashs(DBHeader *hdr, const PackageTree& tree);
	bool write_header(const DBHeader &hdr, FILE *fp, std::string *errtext);
	// return false if version does not match. However, fp is not closed.
	bool read_header(DBHeader *hdr, FILE *fp, std::string *errtext);

	bool write_packagetree(const PackageTree &pkg, const DBHeader &hdr, FILE *fp, std::string *errtext);
	bool read_packagetree(PackageTree *tree, const DBHeader &hdr, PortageSettings *ps, FILE *fp, std::string *errtext);
}

#endif  // SRC_DATABASE_IO_H_
