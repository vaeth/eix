// vim:set noet cinoptions=g0,t0,(0 sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include "basicversion.h"
#include <eixTk/compare.h>
#include <eixTk/exceptions.h>
#include <eixTk/formated.h>
#include <eixTk/i18n.h>
#include <eixTk/likely.h>
#include <eixTk/stringutils.h>

#include <algorithm>
#include <iostream>
#include <iterator>
#include <list>
#include <ostream>
#include <sstream>
#include <string>

using namespace std;

const string::size_type BasicPart::max_type;

short
BasicPart::compare(const BasicPart& left, const BasicPart& right)
{
	// There is some documentation online at http://dev.gentoo.org/~spb/pms.pdf,
	// but I suppose this is not yet sanctioned by gentoo.
	// We are going to follow the text from section 2.3 "Version Comparison" here.

	short ret(eix::default_compare(left.parttype, right.parttype));
	if (ret)
		return ret;

	// We can short-circuit numeric_compare(..) and cutting of trailing 0
	// by using string comparison if both parts have the same length.
	if (left.partcontent.size() == right.partcontent.size()) {
		return left.partcontent.compare(right.partcontent);
	}
	else if (left.parttype == BasicPart::primary) {
		// "[..] if a component has a leading zero, any trailing zeroes in that
		//  component are stripped (if this makes the component empty, proceed
		//  as if it were 0 instead), and the components are compared using a
		//  stringwise comparison."

		if ((left.partcontent)[0] == '0' || (right.partcontent)[0] == '0') {
			string l1(left.partcontent);
			string l2(right.partcontent);

			rtrim(l1, "0");
			rtrim(l2, "0");

			/* No need to check if stripping zeros makes the component empty,
			 * since that only happens for components that _only_ contain zeros
			 * and comparing to "0" or "" will yield the same result - every
			 * other possible value is bigger.
			 */

			return l1.compare(l2);
		}
		// "If neither component has a leading zero, components are compared
		//  using strict integer comparison."
	}
	else if (left.parttype == BasicPart::garbage) {
		// garbage gets string comparison.
		return left.partcontent.compare(left.partcontent);
	}

	// "The first component of the number part is compared using strict integer
	//  comparison."
	return eix::numeric_compare(left.partcontent, right.partcontent);
}

static ostream&
operator<<(ostream& s, const BasicPart& part)
{
	switch (part.parttype) {
		case BasicPart::first:
		case BasicPart::character:
		case BasicPart::garbage:
			return s << part.partcontent;
		case BasicPart::alpha:
			return s << "_alpha" << part.partcontent;
		case BasicPart::beta:
			return s << "_beta" << part.partcontent;
		case BasicPart::pre:
			return s << "_pre" << part.partcontent;
		case BasicPart::rc:
			return s << "_rc" << part.partcontent;
		case BasicPart::patch:
			return s << "_p" << part.partcontent;
		case BasicPart::revision:
			return s << "-r" << part.partcontent;
		case BasicPart::inter_rev:
		case BasicPart::primary:
			return s << "." << part.partcontent;
		default:
			break;
	}
	throw ExBasic(_("internal error: unknown PartType on (%r,%r)"))
		% int(part.parttype) % part.partcontent;
}

void
BasicVersion::rebuild() const
{
	stringstream ss;
	copy(m_parts.begin(), m_parts.end(), ostream_iterator<BasicPart>(ss));
	m_cached_full = ss.str();
}

string
BasicVersion::getPlain() const
{
	stringstream ss;
	for(list<BasicPart>::const_iterator it(m_parts.begin());
		it != m_parts.end(); ++it) {
		if(it->parttype == BasicPart::revision) {
			break;
		}
		ss << *it;
	}
	return ss.str();
}

string
BasicVersion::getRevision() const
{
	stringstream ss;
	bool started(false);
	for(list<BasicPart>::const_iterator it(m_parts.begin());
		it != m_parts.end(); ++it) {
		if(it->parttype == BasicPart::revision) {
			started = true;
		}
		if(started) {
			ss << *it;
		}
	}
	return (started ? ss.str().substr(1) : emptystring);
}

void
BasicVersion::parseVersion(const string& str, bool garbage_fatal)
{
	m_cached_full = str;

	string::size_type pos(0);
	string::size_type len(str.find_first_not_of("0123456789", pos));
	if(unlikely(len == pos || pos == str.size())) {
		m_parts.push_back(BasicPart(BasicPart::garbage, str, pos));
		throw ExBasic(_(
			"malformed (first primary at %r) version string %r"))
			% pos % str;
	}
	m_parts.push_back(BasicPart(BasicPart::first, str, pos, len - pos));

	if (len == string::npos)
		return;

	pos += len;

	while(str[pos] == '.') {
		len = str.find_first_not_of("0123456789", ++pos);
		if(unlikely(len == pos || pos == str.size())) {
			m_parts.push_back(BasicPart(BasicPart::garbage, str, pos));
			throw ExBasic(_(
				"malformed (primary at %r) version string %r"))
				% pos % str;
		}
		m_parts.push_back(BasicPart(BasicPart::primary, str, pos, len - pos));

		if (len == string::npos)
			return;

		pos = len;
	}

	if(isalpha(str[pos], localeC))
		m_parts.push_back(BasicPart(BasicPart::character, str[pos++]));

	if (pos == str.size())
		return;

	while (str[pos] == '_') {
		BasicPart::PartType suffix;
		++pos;
		if (str.compare(pos, 5, "alpha") == 0) {
			pos += 5;
			suffix = BasicPart::alpha;
		}
		else if (str.compare(pos, 4, "beta") == 0) {
			pos += 4;
			suffix = BasicPart::beta;
		}
		else if (str.compare(pos, 3, "pre") == 0) {
			pos += 3;
			suffix = BasicPart::pre;
		}
		else if (str.compare(pos, 2, "rc") == 0) {
			pos += 2;
			suffix = BasicPart::rc;
		}
		else if (str.compare(pos, 1, "p") == 0) {
			++pos;
			suffix = BasicPart::patch;
		}
		else {
			m_parts.push_back(BasicPart(BasicPart::garbage, str, pos-1));
			throw ExBasic(_(
				"malformed (suffix at %r) version string %r"))
				% pos % str;
		}

		len = str.find_first_not_of("0123456789", pos);
		m_parts.push_back(BasicPart(suffix, str, pos, len - pos));

		if (len == string::npos)
			return;

		pos = len;
	}

	// get optional gentoo revision
	if(str.compare(pos, 2, "-r") == 0) {
		len = str.find_first_not_of("0123456789", pos+=2);
		m_parts.push_back(BasicPart(BasicPart::revision, str, pos, len-pos));

		if (len == string::npos)
			return;
		pos = len;

		if (str[pos] == '.') {
			// inter-revision used by prefixed portage.
			// for example foo-1.2-r02.2
			len = str.find_first_not_of("0123456789", ++pos);
			m_parts.push_back(BasicPart(BasicPart::inter_rev, str, pos, len-pos));
			if (len == string::npos)
				return;
			pos = len;
		}
	}

	m_parts.push_back(BasicPart(BasicPart::garbage, str, pos));
	if(garbage_fatal) {
		throw ExBasic(_("garbage (%s) at end of version %r"))
			% str.substr(pos) % str;
	}
	else {
		// warn about garbage, but accept it
		cerr << eix::format(_(
			"garbage (%s) at end of version %r\n"
			"accepting version anyway"))
			% str.substr(pos) % str << endl;
	}
}

short
BasicVersion::compare(const BasicVersion& left, const BasicVersion &right)
{
	list<BasicPart>::const_iterator
		it_left(left.m_parts.begin()),
		it_right(right.m_parts.begin());

	for(short ret(0); ; ++it_left, ++it_right) {
		if (it_left == left.m_parts.end()) {
			if (it_right == right.m_parts.end())
				return 0;
			else if (it_right->parttype < BasicPart::revision)
				return 1;
			return -1;
		}
		else if (it_right == right.m_parts.end()) {
			if (it_left->parttype < BasicPart::revision)
				return -1;
			return 1;
		}
		else if ((ret = BasicPart::compare(*it_left, *it_right)))
			return ret;

	}
	return 0;
}

short
BasicVersion::compareTilde(const BasicVersion& left, const BasicVersion &right)
{
	list<BasicPart>::const_iterator
		it_left(left.m_parts.begin()),
		it_right(right.m_parts.begin());

	for(short ret(0); ; ++it_left, ++it_right) {
		bool left_end = (it_left == left.m_parts.end()
				|| it_left->parttype == BasicPart::revision);
		bool right_end = (it_right == right.m_parts.end()
				|| it_right->parttype == BasicPart::revision);
		if (left_end) {
			return right_end ? 0 : -1;
		}
		else if (right_end) {
			return 1;
		}
		else if ((ret = BasicPart::compare(*it_left, *it_right)))
			return ret;
	}
	return 0;
}
