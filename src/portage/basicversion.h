// vim:set noet cinoptions=g0,t0,^-2 sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#ifndef SRC_PORTAGE_BASICVERSION_H_
#define SRC_PORTAGE_BASICVERSION_H_ 1

#include <list>
#include <string>

#include "eixTk/constexpr.h"
#include "eixTk/eixint.h"

class Database;

// include "portage/basicversion.h" make check_includes happy

class BasicPart {
	public:
		enum PartType {
			garbage,
			alpha,
			beta,
			pre,
			rc,
			revision,
			inter_rev,
			patch,
			character,
			primary,
			first
		};

		// This must be larger than PartType elements and should be a power of 2.
		static CONSTEXPR std::string::size_type max_type = 32;
		PartType parttype;
		std::string partcontent;

		BasicPart() {
		}

		explicit BasicPart(PartType p) : parttype(p), partcontent() {
		}

		BasicPart(PartType p, const std::string& s) : parttype(p), partcontent(s) {
		}

		BasicPart(PartType p, const std::string& s, std::string::size_type start)
			: parttype(p), partcontent(s, start) {
		}

		BasicPart(PartType p, const std::string& s, std::string::size_type start, std::string::size_type end)
			: parttype(p), partcontent(s, start, end) {
		}

		BasicPart(PartType p, char c) : parttype(p), partcontent(1, c) {
		}

		BasicPart(PartType p, const char *s) ATTRIBUTE_NONNULL_ : parttype(p), partcontent(s) {
		}

		static eix::SignedBool compare(const BasicPart& left, const BasicPart& right) ATTRIBUTE_PURE;
};


/** Parse and represent a portage version-string. */
class BasicVersion {
		friend class Database;
	public:
		enum ParseResult {
			parsedOK,
			parsedError,
			parsedGarbage
		};

		virtual ~BasicVersion() { }

		/// Parse the version-string pointed to by str.
		BasicVersion::ParseResult parseVersion(const std::string& str, std::string *errtext, eix::SignedBool accept_garbage);
		BasicVersion::ParseResult parseVersion(const std::string& str, std::string *errtext) {
			return parseVersion(str, errtext, 1);
		}

		/// Compare all except gentoo revisions
		static eix::SignedBool compareTilde(const BasicVersion& right, const BasicVersion& left) ATTRIBUTE_PURE;

		/// Compare the version.
		static eix::SignedBool compare(const BasicVersion& right, const BasicVersion& left) ATTRIBUTE_PURE;

		std::string getFull() const;

		std::string getPlain() const;

		std::string getRevision() const;

	protected:
		/// Splitted m_primsplit-version.
		typedef std::list<BasicPart> PartsType;
		PartsType m_parts;
};


// Short compare-stuff
inline static bool operator<(const BasicVersion& left, const BasicVersion& right) ATTRIBUTE_PURE;
inline static bool operator<(const BasicVersion& left, const BasicVersion& right) {
	return BasicVersion::compare(left, right) < 0;
}

inline static bool operator>(const BasicVersion& left, const BasicVersion& right) ATTRIBUTE_PURE;
inline static bool operator>(const BasicVersion& left, const BasicVersion& right) {
	return BasicVersion::compare(left, right) > 0;
}

inline static bool operator==(const BasicVersion& left, const BasicVersion& right) ATTRIBUTE_PURE;
inline static bool operator==(const BasicVersion& left, const BasicVersion& right) {
	return BasicVersion::compare(left, right) == 0;
}

inline static bool operator!=(const BasicVersion& left, const BasicVersion& right) ATTRIBUTE_PURE;
inline static bool operator!=(const BasicVersion& left, const BasicVersion& right) {
	return BasicVersion::compare(left, right) != 0;
}

inline static bool operator>=(const BasicVersion& left, const BasicVersion& right) ATTRIBUTE_PURE;
inline static bool operator>=(const BasicVersion& left, const BasicVersion& right) {
	return BasicVersion::compare(left, right) >= 0;
}

inline static bool operator<=(const BasicVersion& left, const BasicVersion& right) ATTRIBUTE_PURE;
inline static bool operator<=(const BasicVersion& left, const BasicVersion& right) {
	return BasicVersion::compare(left, right) <= 0;
}

#endif  // SRC_PORTAGE_BASICVERSION_H_
