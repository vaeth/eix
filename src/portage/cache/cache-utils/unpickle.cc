/***************************************************************************
 *   eix is a small utility for searching ebuilds in the                   *
 *   Gentoo Linux portage system. It uses indexing to allow quick searches *
 *   in package descriptions with regular expressions.                     *
 *                                                                         *
 *   https://sourceforge.net/projects/eix                                  *
 *                                                                         *
 *   Copyright (c)                                                         *
 *     Wolfgang Frisch <xororand@users.sourceforge.net>                    *
 *     Emil Beinroth <emilbeinroth@gmx.net>                                *
 *     Martin Väth <vaeth@mathematik.uni-wuerzburg.de>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "unpickle.h"
#include <iostream>

using namespace std;

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

void static unpickle_check_dict(bool &waiting_for_value, map<string,string> &unpickled, string &buf, string &key)
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
	int ret = 1;
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
					unsigned char len;
					len = *(data)++;
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
			default:
				ret = 0;
				continue;
		}
	}
	return true;
}

/** For portage-2.1.2 cache */
/*
bool unpickle_get(const char **datas, const char *data_end, std::map<std::string,std::string> &unpickled, bool first)
{
	int ret = 1;
	const char *data = *datas;
	int i;
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
				i = NEXT;
				cout << "BINPUT: " << i << "\n";
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
					unsigned char len;
					len = *(data)++;
					string buf(data, len);
					MOVE(len);
					cout << "STRING: " << buf << "\n";
				}
				continue;
			case BINSTRING:
				{
					uint32_t len;
					UINT32_UNPACK(data, &len);
					MOVE(4);
					string buf(data, len);
					MOVE(len);
					cout << "STRING: " << buf << "\n";
				}
				continue;
			case SETITEMS:
				i = NEXT;
				cout << "SETITEMS: " << i << "\n";
				continue;
			case MARK:
				cout << "MARK\n";
				continue;
			case EMPTY_DICT:
				cout << "EMPTY_DICT\n";
				continue;
			case LONG:
				cout << "LONG\n";
				MOVE(sizeof(long));
				continue;
			case LONG1:
				i = NEXT;
				cout << "LONG1: " << i << "\n";
				continue;
			case UNICODE:
				MOVE(4);
				cout << "UNICODE\n";
				continue;
			case BINGET:
				i = NEXT;
				cout << "BINGET: " << i << "\n";
				continue;
			case STOP:
				data = data_end;
				continue;
			default:
				cout << "UNKNOWN: " << ret << "\"" << (char)ret << "\"\n";
				*datas = data_end;
				return false;
		}
	}
	*datas = data;
	return true;
}
*/
