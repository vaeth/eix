// vim:set noet cinoptions=g0,t0,(0 sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include "basicversion.h"

#include <iterator>

#include <eixTk/compare.h>
#include <eixTk/exceptions.h>
#include <eixTk/stringutils.h>

using namespace std;

const string::size_type BasicVersion::max_type;

int
BasicVersion::compare(const Part& left, const Part& right)
{
	// There is some documentation online at http://dev.gentoo.org/~spb/pms.pdf,
	// but I suppose this is not yet sanctioned by gentoo.
	// We are going to follow the text from section 2.3 "Version Comparison" here.
	int ret = 0;

	if ((ret = eix::default_compare(left.first, right.first)))
		return ret;

	// We can short-circuit numeric_compare(..) and use string comparison if both parts have
	// the same length and just do string.
	if (left.second.size() == right.second.size()) {
		return left.second.compare(right.second);
	}
	else if (left.first == primary) {
		// "[..] if a component has a leading zero, any trailing zeroes in that
		//  component are stripped (if this makes the component empty, proceed
		//  as if it were 0 instead), and the components are compared using a
		//  stringwise comparison."

		if (left.second.at(0) == '0' || right.second.at(0) == '0') {
			string l1 = left.second;
			string l2 = right.second;

			rtrim(&l1, "0");
			rtrim(&l2, "0");

			return l1.compare(l2);
		}
		// "If neither component has a leading zero, components are compared
		//  using strict integer comparison."
	}
	else if (left.first == garbage) {
		// garbage gets string comparison.
		return left.second.compare(left.second);
	}

	// "The first component of the number part is compared using strict integer
	//  comparison."
	return eix::numeric_compare(left.second, right.second);
}

BasicVersion::BasicVersion(const char *str) {
	if(str)
		parseVersion(str);
}

inline
ostream& operator<<(ostream& s, const BasicVersion::Part& part)
{
	switch (part.first) {
		case BasicVersion::garbage:
			return s << part.second;
		case BasicVersion::alpha:
			return s << "_alpha" << part.second;
		case BasicVersion::beta:
			return s << "_beta" << part.second;
		case BasicVersion::pre:
			return s << "_pre" << part.second;
		case BasicVersion::rc:
			return s << "_rc" << part.second;
		case BasicVersion::revision:
			return s << "-r" << part.second;
		case BasicVersion::inter_rev:
			return s << "." << part.second;
		case BasicVersion::patch:
			return s << "_p" << part.second;
		case BasicVersion::character:
			return s << part.second;
		default:
		// case BasicVersion::primary:
		// case BasicVersion::first:
			return s << "." << part.second;
	}
	throw ExBasic("internal error: unknown PartType on (%r,%r)")
		% int(part.first) % part.second;
}

void
BasicVersion::rebuild() const
{
	stringstream ss;
	copy(m_parts.begin(), m_parts.end(), ostream_iterator<Part>(ss));
	m_full = ss.str().substr(1);
}

void
BasicVersion::parseVersion(const string& str)
{
	m_full = str;

	string::size_type pos = 0;
	string::size_type len = str.find_first_not_of("0123456789", pos);
	if (len == pos || pos == str.size()) {
		m_parts.push_back(Part(garbage, str.substr(pos)));
		throw ExBasic("malformed (first primary at %r) version string %r, "
					  "adding version anyways") % pos % str;
	}
	m_parts.push_back(Part(first, str.substr(pos, len - pos)));

	if (len == string::npos)
		return;

	pos += len;

	while(str.at(pos) == '.') {
		len = str.find_first_not_of("0123456789", ++pos);
		if (len == pos || pos == str.size()) {
			m_parts.push_back(Part(garbage, str.substr(pos)));
			throw ExBasic("malformed (primary at %r) version string %r, "
						  "adding version anyways") % pos % str;
		}
		m_parts.push_back(Part(primary, str.substr(pos, len - pos)));

		if (len == string::npos)
			return;

		pos = len;
	}

	if(isalpha(str.at(pos)))
		m_parts.push_back(Part(character, str.substr(pos++, 1)));

	if (pos == str.size())
		return;

	while (str.at(pos) == '_') {
		PartType suffix;
		pos += 1;
		if (str.compare(pos, 5, "alpha") == 0) {
			pos += 5;
			suffix = alpha;
		}
		else if (str.compare(pos, 4, "beta") == 0) {
			pos += 4;
			suffix = beta;
		}
		else if (str.compare(pos, 3, "pre") == 0) {
			pos += 3;
			suffix = pre;
		}
		else if (str.compare(pos, 2, "rc") == 0) {
			pos += 2;
			suffix = rc;
		}
		else if (str.compare(pos, 1, "p") == 0) {
			pos += 1;
			suffix = patch;
		}
		else {
			m_parts.push_back(Part(garbage, str.substr(pos-1)));
			throw ExBasic("malformed (suffix at %r) version string %r, "
						  "adding version anyways") % pos % str;
		}

		len = str.find_first_not_of("0123456789", pos);
		m_parts.push_back(Part(suffix, str.substr(pos, len - pos)));

		if (len == string::npos)
			return;

		pos = len;
	}

	// get optional gentoo revision
	if(str.compare(pos, 2, "-r") == 0) {
		len = str.find_first_not_of("0123456789", pos+=2);
		m_parts.push_back(Part(revision, str.substr(pos, len-pos)));

		if (len == string::npos)
			return;
		pos = len;

		if (str.at(pos) == '.') {
			// inter-revision used by prefixed portage.
			// for example foo-1.2-r02.2
			len = str.find_first_not_of("0123456789", ++pos);
			m_parts.push_back(Part(inter_rev, str.substr(pos, len-pos)));
			if (len == string::npos)
				return;
			pos = len;
		}
	}

	// warn about garbage, but accept it
	cerr << (eix::format("garbage (%r) at end of version %r, "
							  "adding version anyways")
				  % str.substr(pos) % str) << endl;
	m_parts.push_back(Part(garbage, str.substr(pos)));
}

int
BasicVersion::compare(const BasicVersion& left, const BasicVersion &right)
{
	vector<Part>::const_iterator
		it_left(left.m_parts.begin()),
		it_right(right.m_parts.begin());

	for(int ret = 0;; ++it_left, ++it_right) {
		if (it_left == left.m_parts.end()) {
			if (it_right == right.m_parts.end())
				return 0;
			else if (it_right->first < revision)
				return 1;
			return -1;
		}
		else if (it_right == right.m_parts.end()) {
			if (it_left->first < revision)
				return -1;
			return 1;
		}
		else if ((ret = compare(*it_left, *it_right)))
			return ret;

	}
	return 0;
}

int
BasicVersion::compareTilde(const BasicVersion& left, const BasicVersion &right)
{
	vector<Part>::const_iterator
		it_left(left.m_parts.begin()),
		it_right(right.m_parts.begin());

	for(int ret = 0;; ++it_left, ++it_right)
	{
		bool left_end = (it_left == left.m_parts.end()
				|| it_left->first == revision);
		bool right_end = (it_right == right.m_parts.end()
				|| it_right->first == revision);
		if (left_end) {
			return right_end ? 0 : -1;
		}
		else if (right_end) {
			return 1;
		}
		else if ((ret = compare(*it_left, *it_right)))
			return ret;
	}
	return 0;
}

const ExtendedVersion::Restrict
	ExtendedVersion::RESTRICT_NONE,
	ExtendedVersion::RESTRICT_FETCH,
	ExtendedVersion::RESTRICT_MIRROR;

ExtendedVersion::Restrict
ExtendedVersion::calcRestrict(const string &str)
{
	Restrict r = RESTRICT_NONE;
	vector<string> restrict_words = split_string(str);
	for(vector<string>::const_iterator it = restrict_words.begin();
		it != restrict_words.end(); ++it) {
		if(strcasecmp(it->c_str(), "fetch") == 0)
			r |= RESTRICT_FETCH;
		else if(strcasecmp(it->c_str(), "mirror") == 0)
			r |= RESTRICT_MIRROR;
	}
	return r;
}
