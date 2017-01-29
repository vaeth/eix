// vim:set et cinoptions=g0,t0,^-2 sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#ifndef SRC_EIXTK_FORMATED_H_
#define SRC_EIXTK_FORMATED_H_ 1

#include <ostream>
#include <sstream>
#include <string>
#include <vector>

#include "eixTk/assert.h"
#include "eixTk/eixint.h"
#include "eixTk/likely.h"

// check_includes: include "eixTk/i18n.h" include <iostream>

namespace eix {

/**
printf-like, typesafe text formating that replaces tokens in the given
string with textual representations of values.

Specifier syntax is just like basic printf:
- @b %N is the specifier of type @b N
- @b %num$N is the specifier of type @b N for argument num
- @b %% is a literal @b %

Recognized specifiers are:
- @b s  convert argument to string by using the <<-operator of std::ostream.
- @b d  like %s, but for string::size_type, this will print <string::npos>
        if the argument equals to std::string::npos.

Example usage:

\code
  include "eixTk/formated.h"
  std::string file("/etc/make.conf"), message("something bad happend");
  std::cout << eix::format(_("problems while parsing %s -- %s")) % file % message << std::endl;
  // problems while parsing /etc/make.conf -- something bad happend

  int line = 10;
  std::string::size_type column = 20;
  std::cout << eix::format(_("problems while parsing %s in line %3$s, column %2$d -- %s"))
          % file % column % line % message << std::endl;
  // problems while parsing /etc/make.conf in line 10, column 20 -- something bad happend
\endcode
**/

class format;

class FormatManip {
	protected:
		friend class format;
		typedef std::vector<eix::SignedBool>::size_type ArgCount;
		std::string::size_type m_index;
		ArgCount argnum;
		bool m_type;
		FormatManip(std::string::size_type ind, ArgCount anum, eix::SignedBool typ) :
			m_index(ind), argnum(anum), m_type(typ >= 0) {
		}
};

class FormatReplace {
	protected:
		friend class format;
		std::string s, d;
};

class format {
	private:
		/**
		The currently parsed args
		**/
		bool simple;
		FormatManip::ArgCount current;
		std::vector<FormatReplace> args;
		std::vector<eix::SignedBool> wanted;

		/**
		The format string or result
		**/
		std::string m_text;
		std::vector<FormatManip> manip;

		/**
		Write size_type or "<string::npos>" to stream
		**/
		static std::ostream& write_representation(std::ostream& s, const std::string::size_type& t) {  // NOLINT(runtime/references)
			if(t == std::string::npos) {
				return (s << "<string::npos>");
			}
			return (s << t);
		}

		/**
		Write t into stream s
		**/
		template<typename T> static std::ostream& write_representation(std::ostream& s, const T& t) {  // NOLINT(runtime/references)
			return s << t;
		}

		void manipulate();

	public:
		/**
		Set the template string
		**/
		explicit format(const std::string& format_string);

		format() : simple(true) {
		}

		/**
		Insert the value for the next placeholder
		**/
		template<typename T> format& operator%(const T& s) {
			if(simple) {
				std::ostringstream os;
				os << s;
				m_text.append(os.str());
				return *this;
			}
			if(unlikely(manip.empty())) {
				return *this;
			}
			eix::SignedBool c(wanted[current]);
			if(c >= 0) {
				std::ostringstream os;
				os << s;
				args[current].s = os.str();
			}
			if(c <= 0) {
				std::ostringstream os;
				write_representation(os, s);
				args[current].d = os.str();
			}
			if(unlikely(++current == wanted.size())) {
				manipulate();
			}
			return *this;
		}

		/**
		@return the formated string
		**/
		std::string str() const {
			eix_assert_paranoic(manip.empty());
			return m_text;
		}

		/**
		Convenience wrapper for str()
		**/
		operator std::string() {
			return str();
		}

		/**
		Write formated string to ostream os
		**/
		friend std::ostream& operator<<(std::ostream& os, const format& formater) {
			return os << formater.str();
		}
};

}/* namespace eix */

#endif  // SRC_EIXTK_FORMATED_H_
