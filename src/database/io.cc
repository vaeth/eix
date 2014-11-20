// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>

#include <config.h>

#include <cstdio>

#include <string>
#include <vector>

#include "database/header.h"
#include "database/io.h"
#include "eixTk/auto_list.h"
#include "eixTk/diagnostics.h"
#include "eixTk/eixint.h"
#include "eixTk/i18n.h"
#include "eixTk/likely.h"
#include "eixTk/null.h"
#include "eixTk/stringutils.h"

using std::string;
using std::vector;

bool File::openread(const char *name) {
	return ((fp = fopen(name, "rb")) != NULLPTR);
}

bool File::openwrite(const char *name) {
	return ((fp = fopen(name, "wb")) != NULLPTR);
}

File::~File() {
	if(likely(fp != NULLPTR)) {
		fclose(fp);
	}
}

bool File::seek(eix::OffsetType offset, int whence, std::string *errtext) {
#ifdef HAVE_FSEEKO
	if(likely(fseeko(fp, offset, whence) == 0))
#else
	if(likely(fseek(fp, offset, whence) == 0))
#endif
		return true;
	if(errtext != NULLPTR) {
		*errtext = _("fseek failed");
	}
	return false;
}

eix::OffsetType File::tell() {
#ifdef HAVE_FSEEKO
	// We rely on autoconf whose documentation states:
	// All systems with fseeko() also supply ftello()
	return ftello(fp);
#else
	return ftell(fp);
#endif
}

bool File::read_string_plain(char *s, string::size_type len, string *errtext) {
	if(likely(read(s, len))) {
		return true;
	}
	readError(errtext);
	return false;
}

bool File::write_string_plain(const string& str, string *errtext) {
	if(likely(write(str))) {
		return true;
	}
	writeError(errtext);
	return false;
}

void File::readError(string *errtext) {
	if(errtext != NULLPTR) {
		*errtext = (feof(fp) ?
			_("error while reading from database: end of file") :
			_("error while reading from database"));
	}
}

void File::writeError(string *errtext) {
	if(errtext != NULLPTR) {
		*errtext = _("error while writing to database");
	}
}

bool Database::readUChar(eix::UChar *c, string *errtext) {
	int ch(getch());
	if(likely(ch != EOF)) {
		*c = static_cast<eix::UChar>(ch);
		return true;
	}
	readError(errtext);
	return false;
}

bool Database::writeUChar(eix::UChar c, string *errtext) {
	if(counting) {
		++counter;
	} else if(unlikely(!putch(c))) {
		writeError(errtext);
		return false;
	}
	return true;
}

bool Database::write_string_plain(const string& str, string *errtext) {
	if(counting) {
GCC_DIAG_OFF(sign-conversion)
		counter += str.size();
GCC_DIAG_ON(sign-conversion)
		return true;
	}
	return File::write_string_plain(str, errtext);
}

bool Database::read_string(string *s, string *errtext) {
	string::size_type len;
	if(unlikely(!read_num(&len, errtext))) {
		return false;
	}
	eix::auto_list<char> buf(new char[len + 1]);
	buf.get()[len] = 0;
	if(likely(read_string_plain(buf.get(), len, errtext))) {
		*s = buf.get();
		return true;
	}
	return false;
}

bool Database::write_string(const string& str, string *errtext) {
	return (likely(write_num(str.size(), errtext)) &&
		likely(write_string_plain(str, errtext)));
}

bool Database::write_hash_words(const StringHash& hash, const vector<string>& words, string *errtext) {
	if(unlikely(!write_num(words.size(), errtext))) {
		return false;
	}
	for(vector<string>::const_iterator i(words.begin()); likely(i != words.end()); ++i) {
		if(unlikely(!write_hash_string(hash, *i, errtext))) {
			return false;
		}
	}
	return true;
}

bool Database::read_hash_words(const StringHash& hash, vector<string> *s, string *errtext) {
	vector<string>::size_type e;
	if(unlikely(!read_num(&e, errtext))) {
		return false;
	}
	s->resize(e);
	for(vector<string>::size_type i(0); likely(i != e); ++i) {
		if(unlikely(!read_hash_string(hash, &((*s)[i]), errtext))) {
			return false;
		}
	}
	return true;
}

bool Database::read_hash_words(const StringHash& hash, string *s, string *errtext) {
	s->clear();
	vector<string>::size_type e;
	if(unlikely(!read_num(&e, errtext))) {
		return false;
	}
	for(; e != 0; --e) {
		string r;
		if(unlikely(!read_hash_string(hash, &r, errtext))) {
			return false;
		}
		if(!s->empty()) {
			s->append(1, ' ');
		}
		s->append(r);
	}
	return true;
}
