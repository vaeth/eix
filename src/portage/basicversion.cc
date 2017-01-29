// vim:set noet cinoptions=g0,t0,(0 sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#include "portage/basicversion.h"
#include <config.h>

#include <algorithm>
#include <iostream>
#include <iterator>
#include <ostream>
#include <sstream>
#include <string>

#include "eixTk/compare.h"
#include "eixTk/eixint.h"
#include "eixTk/formated.h"
#include "eixTk/i18n.h"
#include "eixTk/likely.h"
#include "eixTk/null.h"
#include "eixTk/stringutils.h"

using std::string;

using std::cerr;
using std::endl;
using std::ostream;
using std::stringstream;

const string::size_type BasicPart::max_type;

bool BasicPart::equal_but_right_is_cut(const BasicPart& left, const BasicPart& right) {
	return ((left.parttype == right.parttype) && right.partcontent.empty());
}

eix::SignedBool BasicPart::compare(const BasicPart& left, const BasicPart& right) {
	/*
	There is some documentation online at http://dev.gentoo.org/~spb/pms.pdf,
	but I suppose this is not yet sanctioned by gentoo.
	We are going to follow the text from section 2.3 "Version Comparison" here
	*/
	eix::SignedBool ret(eix::default_compare(left.parttype, right.parttype));
	if(ret != 0) {
		return ret;
	}

	/*
	We can short-circuit numeric_compare(..) and cutting of trailing 0
	by using string comparison if both parts have the same length.
	*/
	if(left.partcontent.size() == right.partcontent.size()) {
		return eix::toSignedBool(left.partcontent.compare(right.partcontent));
	}
	if(left.parttype == BasicPart::primary) {
		/*
		"[..] if a component has a leading zero, any trailing zeroes in that
		component are stripped (if this makes the component empty, proceed
		as if it were 0 instead), and the components are compared using a
		stringwise comparison."
		*/

		if(((left.partcontent)[0] == '0') || ((right.partcontent)[0] == '0')) {
			string l1(left.partcontent);
			string l2(right.partcontent);

			rtrim(&l1, "0");
			rtrim(&l2, "0");

			/*
			No need to check if stripping zeros makes the component empty,
			since that only happens for components that _only_ contain zeros
			and comparing to "0" or "" will yield the same result - every
			other possible value is bigger.
			*/

			return eix::toSignedBool(l1.compare(l2));
		}
		/*
		"If neither component has a leading zero, components are compared
		using strict integer comparison."
		*/
	} else if(left.parttype == BasicPart::garbage) {
		// garbage gets string comparison.
		return eix::toSignedBool(left.partcontent.compare(left.partcontent));
	}

	/*
	"The first component of the number part is compared using strict integer
	comparison."
	*/
	return eix::numeric_compare(left.partcontent, right.partcontent);
}

static ostream& operator<<(ostream& s, const BasicPart& part) {
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
	cerr << eix::format(_("internal error: unknown PartType on (\"%s\",\"%s\")"))
		% static_cast<int>(part.parttype) % part.partcontent << endl;
}

string BasicVersion::getFull() const {
	stringstream ss;
	copy(m_parts.begin(), m_parts.end(), std::ostream_iterator<BasicPart>(ss));
	return ss.str();
}

string BasicVersion::getPlain() const {
	stringstream ss;
	for(PartsType::const_iterator it(m_parts.begin());
		likely(it != m_parts.end()); ++it) {
		if(unlikely(it->parttype == BasicPart::revision)) {
			break;
		}
		ss << *it;
	}
	return ss.str();
}

string BasicVersion::getRevision() const {
	stringstream ss;
	for(PartsType::const_iterator it(m_parts.begin());
		likely(it != m_parts.end()); ++it) {
		if(unlikely(it->parttype == BasicPart::revision)) {
			ss << "r" << it->partcontent;
			if(unlikely(++it != m_parts.end())) {
				copy(it, m_parts.end(), std::ostream_iterator<BasicPart>(ss));
			}
			break;
		}
	}
	return ss.str();
}

BasicVersion::ParseResult BasicVersion::parseVersion(const string& str, string *errtext, eix::SignedBool accept_garbage) {
	m_parts.clear();
	string::size_type pos(0);
	string::size_type endpos(str.find_first_not_of("0123456789", pos));
	if(unlikely((endpos == pos) || (pos == str.size()))) {
		m_parts.push_back(BasicPart(BasicPart::garbage, str, pos));
		if(errtext != NULLPTR) {
			*errtext = eix::format(_(
			"malformed (first primary at position %s) version string \"%s\""))
			% pos % str;
		}
		return parsedError;
	}
	m_parts.push_back(BasicPart(BasicPart::first, str, pos, endpos - pos));

	if(endpos == string::npos) {
		return parsedOK;
	}

	pos += endpos;

	while(str[pos] == '.') {
		endpos = str.find_first_not_of("0123456789", ++pos);
		if(unlikely((endpos == pos) || (pos == str.size()))) {
			m_parts.push_back(BasicPart(BasicPart::garbage, str, pos));
			if(errtext != NULLPTR) {
				*errtext = eix::format(_(
				"malformed (primary at position %s) version string \"%s\""))
				% pos % str;
			}
			return parsedError;
		}
		m_parts.push_back(BasicPart(BasicPart::primary, str, pos, endpos - pos));

		if(endpos == string::npos) {
			return parsedOK;
		}

		pos = endpos;
	}

	if(my_isalpha(str[pos])) {
		m_parts.push_back(BasicPart(BasicPart::character, str[pos++]));
	}

	if(pos == str.size()) {
		return parsedOK;
	}

	while(str[pos] == '_') {
		BasicPart::PartType suffix;
		++pos;
		if(str.compare(pos, 5, "alpha") == 0) {
			pos += 5;
			suffix = BasicPart::alpha;
		} else if(str.compare(pos, 4, "beta") == 0) {
			pos += 4;
			suffix = BasicPart::beta;
		} else if(str.compare(pos, 3, "pre") == 0) {
			pos += 3;
			suffix = BasicPart::pre;
		} else if(str.compare(pos, 2, "rc") == 0) {
			pos += 2;
			suffix = BasicPart::rc;
		} else if(str.compare(pos, 1, "p") == 0) {
			++pos;
			suffix = BasicPart::patch;
		} else {
			m_parts.push_back(BasicPart(BasicPart::garbage, str, pos-1));
			if(errtext != NULLPTR) {
				*errtext = eix::format(_(
				"malformed (suffix at position %s) version string \"%s\""))
				% pos % str;
			}
			return parsedError;
		}

		endpos = str.find_first_not_of("0123456789", pos);
		m_parts.push_back(BasicPart(suffix, str, pos, endpos - pos));

		if(endpos == string::npos) {
			return parsedOK;
		}

		pos = endpos;
	}

	// get optional gentoo revision
	if(str.compare(pos, 2, "-r") == 0) {
		endpos = str.find_first_not_of("0123456789", pos+=2);
		m_parts.push_back(BasicPart(BasicPart::revision, str, pos, endpos - pos));

		if(endpos == string::npos) {
			return parsedOK;
		}
		pos = endpos;

		if(str[pos] == '.') {
			// inter-revision used by prefixed portage.
			// for example foo-1.2-r02.2
			endpos = str.find_first_not_of("0123456789", ++pos);
			m_parts.push_back(BasicPart(BasicPart::inter_rev, str, pos, endpos - pos));
			if(endpos == string::npos)
				return parsedOK;
			pos = endpos;
		}
	}

	if(accept_garbage >= 0) {
		m_parts.push_back(BasicPart(BasicPart::garbage, str, pos));
	}
	if(errtext != NULLPTR) {
		*errtext = eix::format(accept_garbage ?
			_("garbage (%s) at end of version \"%s\"\n"
				"accepting version anyway") :
			_("garbage (%s) at end of version \"%s\""))
				% str.substr(pos) % str;
	}
	return parsedGarbage;
}

eix::SignedBool BasicVersion::compare(const BasicVersion& left, const BasicVersion& right, bool right_maybe_shorter) {
	for(PartsType::const_iterator it_left(left.m_parts.begin()),
		it_right(right.m_parts.begin()); ; ++it_left) {
		if(it_left == left.m_parts.end()) {
			if(it_right == right.m_parts.end()) {
				break;
			}
			return ((it_right->parttype < BasicPart::revision) ? 1 : -1);
		} else if(it_right == right.m_parts.end()) {
			if(unlikely(right_maybe_shorter)) {
				break;
			}
			return ((it_left->parttype < BasicPart::revision) ? -1 : 1);
		}
		eix::SignedBool ret(BasicPart::compare(*it_left, *it_right));
		if(ret != 0) {
			// alpha2 must match alpha if right_maybe_shorter
			if(unlikely(right_maybe_shorter)) {
				if(unlikely(++it_right == right.m_parts.end())) {
					if(BasicPart::equal_but_right_is_cut(*it_left, right.m_parts.back())) {
						return 0;
					}
				}
			}
			return ret;
		} else {
			++it_right;
		}
	}
	return 0;
}

eix::SignedBool BasicVersion::compareTilde(const BasicVersion& left, const BasicVersion& right) {
	for(PartsType::const_iterator it_left(left.m_parts.begin()),
		it_right(right.m_parts.begin()); ; ++it_left, ++it_right) {
		bool right_end((it_right == right.m_parts.end())
				|| (it_right->parttype == BasicPart::revision));
		if((it_left == left.m_parts.end())
				|| (it_left->parttype == BasicPart::revision)) {
			return (right_end ? 0 : -1);
		} else if(right_end) {
			return 1;
		} else {
			eix::SignedBool ret(BasicPart::compare(*it_left, *it_right));
			if(ret != 0) {
				return ret;
			}
		}
	}
}
