// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>

#ifndef __EXCEPTIONS_H__
#define __EXCEPTIONS_H__

#include <iostream>
#include <string>
#include <vector>

#include <cerrno>
#include <cstdarg>

#include <eixTk/stringutils.h>

/// Simple exception class with printf-like formating.
class ExBasic 
: public std::exception
{
	public:
		/// Set name of function where exception is constructed.
		ExBasic(const char *func)
			: m_func(func), m_msg()
		{ }

		virtual ~ExBasic() throw() { }

		/// Set the actual exception string.
		ExBasic& format(const std::string &str) 
		{
			m_msg = str; 
			return *this;
		}

		/// Set a printf-like formated exception string.
		ExBasic& format(const char *fmt, ...) 
		{
			va_list ap;
			va_start(ap, fmt);
			char buf[1025];
			vsnprintf(buf, 1024, fmt, ap);
			va_end(ap);
			m_msg  = buf;
			return *this;
		}

		/// Return reference to message.
		const std::string &getMessage() const throw()
		{ return m_msg; }

		/// @see std::exception::what()
		const char *what() const throw()
		{ return m_msg.c_str(); }

		friend std::ostream& operator<< (std::ostream& os, const ExBasic& e)
		{ return os << e.m_func << ": " << e.m_msg; }

	protected:
		std::string m_func; ///< Function that threw us.
		std::string m_msg;  ///< The actual message.
};

/// Automatically fill in the argument for our exceptions.
#define ExBasic() ExBasic(__PRETTY_FUNCTION__)

/// Provide a common look for error-messages for parse-errors in
/// portage.{mask,keywords,..}.
inline void
portage_parse_error(const std::string &file, const int line_nr, const std::string& line, const ExBasic &e)
{
	std::cerr << "-- Invalid line in "<< file << "("<< line_nr <<"): \""
	     << line << "\"" << std::endl;

	// Indent the message correctly
	std::vector<std::string> lines = split_string(e.what(), "\n", false);
	for(std::vector<std::string>::iterator i = lines.begin();
		i != lines.end();
		++i)
	{
		std::cerr << "    " << *i << std::endl;
	}
	std::cerr << std::endl;
}

/// Provide a common look for error-messages for parse-errors in
/// portage.{mask,keywords,..}.
template<class Iterator>
inline void
portage_parse_error(const std::string &file, const Iterator &begin, const Iterator &line, const ExBasic &e)
{
  portage_parse_error(file, std::distance(begin, line) + 1, *line, e);
}

#endif /* __EXCEPTIONS_H__ */
