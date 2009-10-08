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
#include <eixTk/sysutils.h>
#include <portage/conf/portagesettings.h>
#include <portage/package.h>

#include <iterator>

using namespace std;

const string::size_type BasicVersion::max_type;

int
BasicVersion::compare(const Part& left, const Part& right)
{
	// There is some documentation online at http://dev.gentoo.org/~spb/pms.pdf,
	// but I suppose this is not yet sanctioned by gentoo.
	// We are going to follow the text from section 2.3 "Version Comparison" here.

	int ret = eix::default_compare(left.first, right.first);
	if (ret)
		return ret;

	// We can short-circuit numeric_compare(..) and cutting of trailing 0
	// by using string comparison if both parts have the same length.
	if (left.second.size() == right.second.size()) {
		return left.second.compare(right.second);
	}
	else if (left.first == primary) {
		// "[..] if a component has a leading zero, any trailing zeroes in that
		//  component are stripped (if this makes the component empty, proceed
		//  as if it were 0 instead), and the components are compared using a
		//  stringwise comparison."

		if ((left.second)[0] == '0' || (right.second)[0] == '0') {
			string l1 = left.second;
			string l2 = right.second;

			rtrim(&l1, "0");
			rtrim(&l2, "0");

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
		case BasicVersion::primary:
		case BasicVersion::first:
		default:
			return s << "." << part.second;
	}
	throw ExBasic(_("internal error: unknown PartType on (%r,%r)"))
		% int(part.first) % part.second;
}

void
BasicVersion::rebuild() const
{
	stringstream ss;
	copy(m_parts.begin(), m_parts.end(), ostream_iterator<Part>(ss));
	m_cached_full = ss.str().substr(1);
}

void
BasicVersion::parseVersion(const string& str)
{
	m_cached_full = str;

	string::size_type pos = 0;
	string::size_type len = str.find_first_not_of("0123456789", pos);
	if (len == pos || pos == str.size()) {
		m_parts.push_back(Part(garbage, str.substr(pos)));
		throw ExBasic(_(
			"malformed (first primary at %r) version string %r\n"))
			% pos % str;
	}
	m_parts.push_back(Part(first, str.substr(pos, len - pos)));

	if (len == string::npos)
		return;

	pos += len;

	while(str[pos] == '.') {
		len = str.find_first_not_of("0123456789", ++pos);
		if (len == pos || pos == str.size()) {
			m_parts.push_back(Part(garbage, str.substr(pos)));
			throw ExBasic(_(
				"malformed (primary at %r) version string %r\n"))
				% pos % str;
		}
		m_parts.push_back(Part(primary, str.substr(pos, len - pos)));

		if (len == string::npos)
			return;

		pos = len;
	}

	if(isalpha(str[pos]))
		m_parts.push_back(Part(character, str.substr(pos++, 1)));

	if (pos == str.size())
		return;

	while (str[pos] == '_') {
		PartType suffix;
		pos ++;
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
			pos ++;
			suffix = patch;
		}
		else {
			m_parts.push_back(Part(garbage, str.substr(pos-1)));
			throw ExBasic(_(
				"malformed (suffix at %r) version string %r"))
				% pos % str;
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

		if (str[pos] == '.') {
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
	cerr << eix::format(_(
		"garbage (%s) at end of version %r\n"
		"accepting version anyway"))
		% str.substr(pos) % str << endl;
	m_parts.push_back(Part(garbage, str.substr(pos)));
}

int
BasicVersion::compare(const BasicVersion& left, const BasicVersion &right)
{
	list<Part>::const_iterator
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
	list<Part>::const_iterator
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
	ExtendedVersion::RESTRICT_BINCHECKS,
	ExtendedVersion::RESTRICT_STRIP,
	ExtendedVersion::RESTRICT_TEST,
	ExtendedVersion::RESTRICT_USERPRIV,
	ExtendedVersion::RESTRICT_INSTALLSOURCES,
	ExtendedVersion::RESTRICT_FETCH,
	ExtendedVersion::RESTRICT_MIRROR,
	ExtendedVersion::RESTRICT_PRIMARYURI,
	ExtendedVersion::RESTRICT_BINDIST;
const ExtendedVersion::Properties
	ExtendedVersion::PROPERTIES_NONE,
	ExtendedVersion::PROPERTIES_INTERACTIVE,
	ExtendedVersion::PROPERTIES_LIVE,
	ExtendedVersion::PROPERTIES_VIRTUAL,
	ExtendedVersion::PROPERTIES_SET;

static class RestrictMap : public map<string,ExtendedVersion::Restrict> {
	private:
		void mapinit(const char *s, ExtendedVersion::Restrict r)
		{ (*this)[s] = r; }
	public:
		RestrictMap()
		{
			mapinit("fetch",          ExtendedVersion::RESTRICT_FETCH);
			mapinit("mirror",         ExtendedVersion::RESTRICT_MIRROR);
			mapinit("primaryuri",     ExtendedVersion::RESTRICT_PRIMARYURI);
			mapinit("binchecks",      ExtendedVersion::RESTRICT_BINCHECKS);
			mapinit("bindist",        ExtendedVersion::RESTRICT_BINDIST);
			mapinit("installsources", ExtendedVersion::RESTRICT_INSTALLSOURCES);
			mapinit("strip",          ExtendedVersion::RESTRICT_STRIP);
			mapinit("test",           ExtendedVersion::RESTRICT_TEST);
			mapinit("userpriv",       ExtendedVersion::RESTRICT_USERPRIV);
		}

		ExtendedVersion::Restrict getRestrict(const string& s) const
		{
			RestrictMap::const_iterator i = find(s);
			if(i != end())
				return i->second;
			return ExtendedVersion::RESTRICT_NONE;
		}
} restrict_map;

static class PropertiesMap : public map<string,ExtendedVersion::Properties> {
	private:
		void mapinit(const char *s, ExtendedVersion::Properties p)
		{ (*this)[s] = p; }
	public:
		PropertiesMap()
		{
			mapinit("interactive", ExtendedVersion::PROPERTIES_INTERACTIVE);
			mapinit("live",        ExtendedVersion::PROPERTIES_LIVE);
			mapinit("virtual",     ExtendedVersion::PROPERTIES_VIRTUAL);
			mapinit("set",         ExtendedVersion::PROPERTIES_SET);
		}

		ExtendedVersion::Properties getProperties(const string& s) const
		{
			PropertiesMap::const_iterator i = find(s);
			if(i != end())
				return i->second;
			return ExtendedVersion::PROPERTIES_NONE;
		}
} properties_map;

ExtendedVersion::Restrict
ExtendedVersion::calcRestrict(const string &str)
{
	Restrict r = RESTRICT_NONE;
	vector<string> restrict_words = split_string(str);
	for(vector<string>::const_iterator it = restrict_words.begin();
		it != restrict_words.end(); ++it)
		r |= restrict_map.getRestrict(*it);
	return r;
}

ExtendedVersion::Properties
ExtendedVersion::calcProperties(const string &str)
{
	Properties p = PROPERTIES_NONE;
	vector<string> properties_words = split_string(str);
	for(vector<string>::const_iterator it = properties_words.begin();
		it != properties_words.end(); ++it)
		p |= properties_map.getProperties(*it);
	return p;
}

const ExtendedVersion::HaveBinPkg
	ExtendedVersion::HAVEBINPKG_UNKNOWN,
	ExtendedVersion::HAVEBINPKG_NO,
	ExtendedVersion::HAVEBINPKG_YES;

bool
ExtendedVersion::have_bin_pkg(PortageSettings *ps, const Package *pkg)
{
	switch(have_bin_pkg_m) {
		case HAVEBINPKG_UNKNOWN:
			{
				string &s = (*ps)["PKGDIR"];
				if((s.empty()) || !is_file((s + "/" + pkg->category + "/" + pkg->name + "-" + getFull() + ".tbz2").c_str())) {
					have_bin_pkg_m = HAVEBINPKG_NO;
					return false;
				}
				have_bin_pkg_m = HAVEBINPKG_YES;
			}
			break;
		case HAVEBINPKG_NO:
			return false;
		default:
		// case HAVEBINPKG_YES;
			break;
	}
	return true;
}
