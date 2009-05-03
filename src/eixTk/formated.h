// vim:set et cinoptions=g0,t0,^-2 sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef EIX__FORMATED_H__
#define EIX__FORMATED_H__ 1

#include <eixTk/i18n.h>

#include <string>
#include <sstream>
#include <ostream>
#include <cassert>
#include <cstdlib>

namespace eix {

/** printf-like, typesafe text formating that replaces tokens in the given
    string with textual representations of values.

Specifier syntax is just like basic printf:
- @b %N is the specifier of type @b N
- @b %% is a literal @b %

Recognized specifiers are:
- @b s  convert argument to string by using the <<-operator of std::ostream.
- @b r  like %s, but if argument is a string type (std::string or char *) it
        is enclosed in single quotes. For string::size_type, this will
        print "<string::npos>" if the argument equals to std::string::npos.

Example usage:

\code
  std::string file("/etc/make.conf"), message("something bad happend");
  std::cout << eix::format(_("problems while parsing %r -- %s")) % file % message << std::endl;
  // problems while parsing "/etc/make.conf" -- something bad happend

  int line = 10, column = 20;
  std::cout << eix::format(_("problems while parsing %r in line %r, column %r -- %s"))
          % file % message % line % column << std::endl;
  // problems while parsing "/etc/make.conf" in line 10, column 20 -- something bad happend
\endcode */

class format
{
	private:
		/// Currently processed specifier, 0 if there are no more specifiers.
		char m_spec;
		/// The current state of the formated string.
		std::ostringstream m_stream;
		/// What is left of the format string.
		std::string m_format;
	public:
		/// Set the template string.
		explicit format(const std::string& format_string) : m_spec(0),
			m_stream(), m_format(format_string)
		{ goto_next_spec(); }

		/// Copy current state into new formater.
		format(const format& e) : m_spec(e.m_spec),
			m_stream(e.m_stream.str()), m_format(e.m_format)
		{ }

		/// Copy current state of formater.
		format& operator=(const format& e);

		/// Reset the internal state and use format_string as the new format string.
		format& reset(const std::string& format_string);

		/// Insert the value for the next placeholder.
		template<typename T>
		format& operator%(const T& s)
		{
			switch(m_spec) {
				case 's':
					m_stream << s;
					break;
				case 'r':
					write_representation(m_stream, s);
					break;
#ifndef NDEBUG
				case 0:
					std::cerr << formated(_("format specifier missing"))
						<< std::endl;
					exit(1);
#endif
				default:
#ifndef NDEBUG
					std::cerr << formated(_("unknown format specifier '%%%s'")) % m_spec
						<< std::endl;
					exit(1);
#endif
					break;
			}
			goto_next_spec();
			return *this;
		}

		/// Return the formated string.
		std::string str() const
		{
			assert(m_spec == 0);
			return m_stream.str();
		}

		/// Convenience wrapper for str().
		operator std::string ()
		{ return str(); }

		/// Write formated string to ostream os.
		friend std::ostream& operator<<(std::ostream& os, const format& formater)
		{ return os << formater.str(); }

	protected:
		/// Find the next specifiers in the format string.
		void goto_next_spec();

		/// Write string t enclosed in single quotes into stream s.
		std::ostream& write_representation(std::ostream& s, const char *t)
		{ return s << "'" << t << "'"; }

		/// Write string t enclosed in single quotes into stream s.
		std::ostream& write_representation(std::ostream& s, const std::string &t)
		{ return s << "'" << t << "'"; }

		/// Write size_type or "<string::npos>" to stream.
		std::ostream& write_representation(std::ostream& s, const std::string::size_type &t)
		{
			if(t == std::string::npos)
				return s << "<string::npos>";
			return s << t;
		}

		/// Write t into stream s.
		template<typename T>
		std::ostream& write_representation(std::ostream& s, const T& t)
		{ return s << t; }
};

}/* namespace eix */

#endif /* EIX__FORMATED_H__ */
