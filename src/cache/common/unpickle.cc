// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include "unpickle.h"
#include <iostream>

using namespace std;

typedef unsigned char unsigned_char;

/*
 * Pickle opcodes.  These must be kept in synch with pickle.py.  Extensive
 * docs are in pickletools.py.
 */
#define MARK        '('
#define STOP        '.'
#define POP         '0'
#define POP_MARK    '1'
#define DUP         '2'
#define FLOAT       'F'
#define BINFLOAT    'G'
#define INT         'I'
#define BININT      'J'
#define BININT1     'K'
#define LONG        'L'
#define BININT2     'M'
#define NONE        'N'
#define PERSID      'P'
#define BINPERSID   'Q'
#define REDUCE      'R'
#define STRING      'S'
#define BINSTRING   'T'
#define SHORT_BINSTRING 'U'
#define UNICODE     'V'
#define BINUNICODE  'X'
#define APPEND      'a'
#define BUILD       'b'
#define GLOBAL      'c'
#define DICT        'd'
#define EMPTY_DICT  '}'
#define APPENDS     'e'
#define GET         'g'
#define BINGET      'h'
#define INST        'i'
#define LONG_BINGET 'j'
#define LIST        'l'
#define EMPTY_LIST  ']'
#define OBJ         'o'
#define PUT         'p'
#define BINPUT      'q'
#define LONG_BINPUT 'r'
#define SETITEM     's'
#define TUPLE       't'
#define EMPTY_TUPLE ')'
#define SETITEMS    'u'

#define PROTO	 '\x80' /* identify pickle protocol */
#define NEWOBJ   '\x81' /* build object by applying cls.__new__ to argtuple */
#define EXT1     '\x82' /* push object from extension registry; 1-byte index */
#define EXT2     '\x83' /* ditto, but 2-byte index */
#define EXT4     '\x84' /* ditto, but 4-byte index */
#define TUPLE1   '\x85' /* build 1-tuple from stack top */
#define TUPLE2   '\x86' /* build 2-tuple from two topmost stack items */
#define TUPLE3   '\x87' /* build 3-tuple from three topmost stack items */
#define NEWTRUE  '\x88' /* push True */
#define NEWFALSE '\x89' /* push False */
#define LONG1    '\x8a' /* push long from < 256 bytes */
#define LONG4    '\x8b' /* push really big long */

#define NEXT        (*data++)
#define MOVE(x)     (data += x)
#define IS_END      (data >= data_end)

static void unpickle_check_dict(bool &waiting_for_value, map<string,string> &unpickled, string &buf, string &key)
{
	if(waiting_for_value) {
		unpickled[key] = buf;
		waiting_for_value = false;
	}
	else {
		key = buf;
		waiting_for_value = true;
	}
}

/** For cdb cache */
bool unpickle_get_mapping(char *data, unsigned int data_len, map<string,string> &unpickled)
{
	char ret = 1;
	char *data_end = data + data_len;
	bool waiting_for_value = false;
	string key;

	if(NEXT != PROTO)
		return false;

	if(NEXT != 0x2)
		return false;

	if(NEXT != EMPTY_DICT)
		return false;

	while( ! IS_END && (ret = NEXT) ) {
		switch(ret) {
			case BINPUT:
				MOVE(1);
				continue;
			case BININT:
				{
					string buf;
					MOVE(sizeof(int));
					unpickle_check_dict(waiting_for_value, unpickled, buf, key);
				}
				continue;
			case SHORT_BINSTRING:
				{
					unsigned char len = NEXT;
					string buf(data, len);
					MOVE(len);
					unpickle_check_dict(waiting_for_value, unpickled, buf, key);
				}
				continue;
			case BINSTRING:
				{
					uint32_t len;
					UINT32_UNPACK(data, &len);
					MOVE(4);
					string buf(data, len);
					MOVE(len);
					unpickle_check_dict(waiting_for_value, unpickled, buf, key);
				}
				continue;
			case SETITEMS:
				ret = 0;
				continue;
			default:
				break;
		}
	}
	return true;
}

//#define TEST_UNPICKLE

/** For portage-2.1.2 cache */
void
Unpickler::get(map<string,string> &unpickled) throw(ExBasic)
{
	vector<string>::size_type index;
	char prev = 0, curr = 1;
	bool waiting_for_value = false;
	string buf, key;

	unpickled.clear();
	if(firsttime) {
		if( ! is_finished() && (NEXT == PROTO) )
			if( ! is_finished() && (NEXT == 0x2) )
				firsttime = false;
		if(firsttime) {
			throw ExBasic("wrong cpickle header");
			finish();
			return;
		}
	}

	while( ! is_finished() && (curr = NEXT) ) {
		switch(curr) {
			case BINGET:
				index = unsigned_char(NEXT);
#ifdef TEST_UNPICKLE
				cout << "BINGET: " << index << "\n";
#endif
				if(prev == STRING)
					buf = wasput.at(index);
				else
					prev = BINGET;
				continue;
			case LONG_BINGET:
				{
					uint32_t arg;
					UINT32_UNPACK(data, &arg);
					index = arg;
					MOVE(4);
				}
#ifdef TEST_UNPICKLE
				cout << "BINGET: " << index << "\n";
#endif
				if(prev == STRING)
					buf = wasput.at(index);
				else
					prev = BINGET;
				continue;
			case BINPUT:
				index = unsigned_char(NEXT);
#ifdef TEST_UNPICKLE
				cout << "BINPUT: " << (int)index << "\n";
#endif
				if(prev == STRING)
					insert(index, buf);
				continue;
			case LONG_BINPUT:
				{
					uint32_t arg;
					UINT32_UNPACK(data, &arg);
					index = arg;
					MOVE(4);
				}
#ifdef TEST_UNPICKLE
				cout << "BINPUT: " << index << "\n";
#endif
				if(prev == STRING)
					insert(index, buf);
				continue;
			case BININT:
#ifdef TEST_UNPICKLE
				cout << "BININT\n";
#endif
				MOVE(sizeof(int));
				prev = BININT;
				continue;
			case SHORT_BINSTRING:
				{
					unsigned char len = NEXT;
					buf = string(data, len);
					MOVE(len);
#ifdef TEST_UNPICKLE
					cout << "STRING: " << buf << "\n";
#endif
				}
				prev = STRING;
				break;
			case BINSTRING:
				{
					uint32_t len;
					UINT32_UNPACK(data, &len);
					MOVE(4);
					buf = string(data, len);
					MOVE(len);
#ifdef TEST_UNPICKLE
					cout << "STRING: " << buf << "\n";
#endif
				}
				prev = STRING;
				break;
			case SETITEMS:
#ifdef TEST_UNPICKLE
				{
					unsigned char i = *data;
					cout << "SETITEMS: " << (int)i << "\n";
				}
#endif
				MOVE(1);
				// prev = SETITEMS;
				return;
			case MARK:
#ifdef TEST_UNPICKLE
				cout << "MARK\n";
#endif
				prev = MARK;
				continue;
			case EMPTY_DICT:
#ifdef TEST_UNPICKLE
				cout << "EMPTY_DICT\n";
#endif
				prev = EMPTY_DICT;
				continue;
			case LONG1:
				{
					unsigned char len = NEXT;
					MOVE(len);
				}
#ifdef TEST_UNPICKLE
				cout << "LONG1\n";
#endif
				prev = LONG;
				break;
			case STOP:
#ifdef TEST_UNPICKLE
				cout << "STOP\n";
#endif
				finish();
				// prev = STOP;
				continue;
			default:
				finish();
				{
					throw ExBasic("Unsupported pickle value %r") % uint(curr);
					// prev = curr;
					return;
				}
		}
		if(prev == STRING)
			unpickle_check_dict(waiting_for_value, unpickled, buf, key);
		// This is a hack: If a long follows, we have the key string:
		else if(prev == LONG) {
			unpickled["KEY"] = buf;
			waiting_for_value = false;
		}
	}
}

void Unpickler::insert(vector<string>::size_type index, string string)
{
	if(index >= wasput.size())
		wasput.resize(index + 100);
	wasput[index] = string;
}
