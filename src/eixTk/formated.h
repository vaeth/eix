// vim:set et cinoptions=g0,t0,^-2 sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#ifndef SRC_EIXTK_FORMATED_H_
#define SRC_EIXTK_FORMATED_H_ 1

#include <config.h>  // IWYU pragma: keep

#include <cstdio>

#include <ostream>
#include <sstream>
#include <string>
#include <vector>

#include "eixTk/attribute.h"
#include "eixTk/dialect.h"
#include "eixTk/eixint.h"
#include "eixTk/likely.h"
#include "eixTk/null.h"

// When EIX_DEBUG_FORMAT is defined, check about too many/few arguments.
// In this case, too many arguments are an error; otherwise they are admissible
// but ignored.
// In the error case, a diagnostic message is printed, and the program halts.
// Without EIX_DEBUG_FORMAT, the output string in case of too few arguments
// is unspecified, but it does not lead to a segfault or memory leak.

#ifndef NDEBUG
#ifdef EIX_PARANOIC_ASSERT
#ifndef EIX_DEBUG_FORMAT
#define EIX_DEBUG_FORMAT 1
#endif
#endif
#endif

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

The constructor with an omitted format string acts like the format %s.
An exception is if the stream is specified last - after newline (and flush):
With this exceptional order an omitted format acts like the empty format.

The constructor with a newline (true) argument will add a newline at the end
The constructor with a stream will output the result to a stream.
The constructor with a stream and flush (true) will flush the stream.

The variants print/print_error use the default stream stdout.
The variants say/say_error use the default stream stdout and set newline.
The variant say_error has flush set by default.
The variants print_empty/print_error_empty/say_empty/say_error_empty use
the empty format string.

Example usage:

\code
  #include <iostream>
  #include "eixTk/formated.h"
  #include "eixTk/i18n.h"
  std::string file("/etc/make.conf"), message("something bad happend");
  std::cout << eix::format(_("problems while parsing %s -- %s")) % file % message << std::endl;
  // problems while parsing /etc/make.conf -- something bad happend

  int line = 10;
  std::string::size_type column = 20;
  eix::say_error(_("problems while parsing %s in line %3$s, column %2$d -- %s"))
          % file % column % line % message)
  // problems while parsing /etc/make.conf in line 10, column 20 -- something bad happend
\endcode
**/

class FormatManip {
	protected:
		friend class format;

		typedef TinyUnsigned ArgType;
		static CONSTEXPR const ArgType
			NONE   = 0x00,
			STRING = 0x01,
			DIGIT  = 0x02,
			BOTH   = (STRING|DIGIT);

		typedef bool ManipType;
		typedef std::vector<ArgType>::size_type ArgCount;
		std::string::size_type m_index;
		ArgCount argnum;
		ManipType m_type;  // true if string desired

	public:
		FormatManip(std::string::size_type ind, ArgCount anum, ArgType typ) NOEXCEPT :
			m_index(ind), argnum(anum), m_type((typ & DIGIT) == NONE) {  // NOLINT(runtime/references)
		}
};

class FormatReplace {
	protected:
		friend class format;
		std::string s, d;
};

class format {
	protected:
		typedef FormatManip::ArgType ArgType;
		typedef FormatManip::ArgCount ArgCount;
		bool simple;  // true only if no formatstring given. Set to false as a flag if first argument is passed
		bool add_newline, do_flush;
		FILE *output;
		/**
		The currently parsed args
		**/
		ArgCount current;
		std::vector<FormatReplace> args;
		std::vector<ArgType> wanted;

		/**
		The format string or result
		**/
		std::string m_text;
		std::vector<FormatManip> manip;

		ATTRIBUTE_NORETURN void bad_format() const;
#ifdef EIX_DEBUG_FORMAT
		ATTRIBUTE_NORETURN void too_few_arguments() const;
		ATTRIBUTE_NORETURN void too_many_arguments() const;
#endif

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

		void finalize();

		void newline_output();

		/**
		Set the template string. Set simple = false
		**/
		void init();

	public:
		format(FILE *stream, const std::string& format_string, bool newline, bool flush) : add_newline(newline), do_flush(flush), output(stream), m_text(format_string) {
			init();
		}

		format(FILE *stream, const char *format_string, bool newline, bool flush) : add_newline(newline), do_flush(flush), output(stream), m_text(format_string) {
			init();
		}

		format(FILE *stream, char format_char, bool newline, bool flush) : simple(false), add_newline(newline), do_flush(flush), output(stream), m_text(1, format_char) {
			newline_output();
		}

		format(FILE *stream, const std::string& format_string, bool newline) : add_newline(newline), do_flush(false), output(stream), m_text(format_string) {
			init();
		}

		format(FILE *stream, const char *format_string, bool newline) : add_newline(newline), do_flush(false), output(stream), m_text(format_string) {
			init();
		}

		format(FILE *stream, char format_char, bool newline) : simple(false), add_newline(newline), do_flush(false), output(stream), m_text(1, format_char) {
			newline_output();
		}

		format(FILE *stream, const std::string& format_string) : add_newline(false), do_flush(false), output(stream), m_text(format_string) {
			init();
		}

		format(FILE *stream, const char *format_string) : add_newline(false), do_flush(false), output(stream), m_text(format_string) {
			init();
		}

		format(FILE *stream, char format_char) : simple(false), add_newline(false), do_flush(false), output(stream), m_text(1, format_char) {
			newline_output();
		}

		format(const std::string& format_string, bool newline) : add_newline(newline), output(NULLPTR), m_text(format_string) {
			init();
		}

		format(const char *format_string, bool newline) : add_newline(newline), output(NULLPTR), m_text(format_string) {
			init();
		}

		format(char format_char, bool newline) : simple(false), add_newline(newline), output(NULLPTR), m_text(1, format_char) {
			newline_output();
		}

		explicit format(const std::string& format_string) : add_newline(false), output(NULLPTR), m_text(format_string) {
			init();
		}

		explicit format(const char *format_string) : add_newline(false), output(NULLPTR), m_text(format_string) {
			init();
		}

		explicit format(char format_char) : simple(false), add_newline(false), output(NULLPTR), m_text(1, format_char) {
		}

		format(FILE *stream, bool newline, bool flush) : simple(true), add_newline(newline), do_flush(flush), output(stream) {
		}

		format(FILE *stream, bool newline) : simple(true), add_newline(newline), do_flush(false), output(stream) {
		}

		// Exceptional order in which case we use the empty string as default
		format(bool newline, bool flush, FILE *stream) : simple(false), add_newline(newline), do_flush(flush), output(stream) {
			newline_output();
		}

		// Exceptional order in which case we use the empty string as default
		format(bool newline, FILE *stream) : simple(false), add_newline(newline), do_flush(false), output(stream) {
			newline_output();
		}

		explicit format(FILE *stream) : simple(true), add_newline(false), do_flush(false), output(stream) {
		}

		explicit format(bool newline) : simple(true), add_newline(newline), output(NULLPTR) {
		}

		format() : simple(true), add_newline(false), output(NULLPTR) {
		}

		/**
		Insert the value for the next placeholder
		**/
		template<typename T> format& operator%(const T& s) {
			if(simple) {
				simple = false;
				std::ostringstream os;
				os << s;
				m_text.append(os.str());
				newline_output();
				return *this;
			}
			if(unlikely(manip.empty())) {
#ifdef EIX_DEBUG_FORMAT
				too_many_arguments();
#else
				return *this;
#endif
			}
			ArgType c(wanted[current]);
			if((c & FormatManip::STRING) != FormatManip::NONE) {
				std::ostringstream os;
				os << s;
				args[current].s = os.str();
			}
			if((c & FormatManip::DIGIT) != FormatManip::NONE) {
				std::ostringstream os;
				write_representation(os, s);
				args[current].d = os.str();
			}
			if(unlikely(++current == wanted.size())) {
				finalize();
			}
			return *this;
		}

		/**
		@return the formated string
		**/
		std::string str() const {
#ifdef EIX_DEBUG_FORMAT
			if(unlikely(simple || !manip.empty())) {
				too_few_arguments();
			}
#endif
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

class print : public format {
	public:
		print(const std::string& format_string, bool flush) : format(stdout, format_string, false, flush) {
		}

		print(const char *format_string, bool flush) : format(stdout, format_string, false, flush) {
		}

		print(char format_char, bool flush) : format(stdout, format_char, false, flush) {
		}

		explicit print(const std::string& format_string) : format(stdout, format_string) {
		}

		explicit print(const char *format_string) : format(stdout, format_string) {
		}

		explicit print(char format_char) : format(stdout, format_char) {
		}

		explicit print(bool flush) : format(stdout, false, flush) {
		}

		print() : format(stdout) {
		}
};

class print_empty : public format {
	public:
		explicit print_empty(bool flush) : format(false, flush, stdout) {
		}

		print_empty() : format(false, stdout) {
		}
};

class print_error : public format {
	public:
		print_error(const std::string& format_string, bool flush) : format(stderr, format_string, false, flush) {
		}

		print_error(const char *format_string, bool flush) : format(stderr, format_string, false, flush) {
		}

		print_error(char format_char, bool flush) : format(stderr, format_char, false, flush) {
		}

		explicit print_error(const std::string& format_string) : format(stderr, format_string) {
		}

		explicit print_error(const char *format_string) : format(stderr, format_string) {
		}

		explicit print_error(char format_char) : format(stderr, format_char) {
		}

		explicit print_error(bool flush) : format(stderr, false, flush) {
		}

		print_error() : format(stderr) {
		}
};

class print_error_empty : public format {
	public:
		explicit print_error_empty(bool flush) : format(false, flush, stderr) {
		}

		print_error_empty() : format(false, stderr) {
		}
};


class say : public format {
	public:
		say(const std::string& format_string, bool flush) : format(stdout, format_string, true, flush) {
		}

		say(const char *format_string, bool flush) : format(stdout, format_string, true, flush) {
		}

		say(char format_char, bool flush) : format(stdout, format_char, true, flush) {
		}

		explicit say(const std::string& format_string) : format(stdout, format_string, true) {
		}

		explicit say(const char *format_string) : format(stdout, format_string, true) {
		}

		explicit say(char format_char) : format(stdout, format_char, true) {
		}

		explicit say(bool flush) : format(stdout, true, flush) {
		}

		say() : format(stdout, true) {
		}
};

class say_empty : public format {
	public:
		explicit say_empty(bool flush) : format(true, flush, stdout) {
		}

		say_empty() : format(true, stdout) {
		}
};

class say_error : public format {
	public:
		say_error(const std::string& format_string, bool flush) : format(stderr, format_string, true, flush) {
		}

		say_error(const char *format_string, bool flush) : format(stderr, format_string, true, flush) {
		}

		say_error(char format_char, bool flush) : format(stderr, format_char, true, flush) {
		}

		explicit say_error(const std::string& format_string) : format(stderr, format_string, true, true) {
		}

		explicit say_error(const char *format_string) : format(stderr, format_string, true, true) {
		}

		explicit say_error(char format_char) : format(stderr, format_char, true, true) {
		}

		explicit say_error(bool flush) : format(stderr, true, flush) {
		}

		say_error() : format(stderr, true, true) {
		}
};

class say_error_empty : public format {
	public:
		explicit say_error_empty(bool flush) : format(true, flush, stderr) {
		}

		say_error_empty() : format(true, stderr) {
		}
};

}/* namespace eix */

#endif  // SRC_EIXTK_FORMATED_H_
