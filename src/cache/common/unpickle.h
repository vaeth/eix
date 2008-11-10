// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#if !defined(EIX__UNPICKLE_H__)
#define EIX__UNPICKLE_H__

#include <config.h>

#include <eixTk/inttypes.h>
#include <eixTk/exceptions.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

#include <unistd.h>

#define UINT32_PACK(out,in)   uint32_pack(out,in)
#define UINT32_UNPACK(in,out) uint32_unpack(in,out)

inline static void uint32_pack(char *out, uint32_t in);
inline static void uint32_unpack(const char *in, uint32_t *out);

#if defined(WORDS_BIGENDIAN)
/* For the big-endian machines */

/* Adopted from python-cdb (which adopted it from libowfat :) */
inline static void uint32_pack(char *out, uint32_t in) {
	*out=in&0xff; in>>=8;
	*++out=in&0xff; in>>=8;
	*++out=in&0xff; in>>=8;
	*++out=in&0xff;
}

inline static void uint32_unpack(const char *in, uint32_t *out) {
	*out = (((uint32_t(uint8_t(in[3])))<<24) |
		((uint32_t(uint8_t(in[2])))<<16) |
		((uint32_t(uint8_t(in[1])))<<8) |
		(uint32_t(uint8_t(in[0]))));
}

#else  /* defined(WORDS_BIGENDIAN) */
/* This is for the little-endian pppl */

/* Adopted from python-cdb (which adopted it from libowfat) */
inline static void uint32_pack(char *out, uint32_t in) {
	*(reinterpret_cast<uint32_t*>(out)) = in;
}

inline static void uint32_unpack(const char *in, uint32_t *out) {
	*out = *(reinterpret_cast<const uint32_t*>(in));
}

#endif /* defined(WORDS_BIGENDIAN) */

/** For cdb cache */
bool unpickle_get_mapping(char *data, unsigned int data_len, std::map<std::string,std::string> &unpickled);

/** For portage-2.1.2 cache */
class Unpickler {
	private:
		bool firsttime;
		std::vector<std::string> wasput;
		const char *data, *data_end;
		void insert(std::vector<std::string>::size_type index, std::string string);
	public:
		void clear()
		{ wasput.clear(); }

		void finish()
		{ data = data_end; clear(); }

		bool is_finished()
		{ return (data >= data_end); }

		Unpickler(const char *start, const char *end)
		{ data = start ; data_end = end; clear(); firsttime = true; }

		void get(std::map<std::string,std::string> &unpickled) throw(ExBasic);
};

#endif /* EIX__UNPICKLE_H__ */
