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

#include <eixTk/formated.h>
#include <eixTk/stringutils.h>

/// Simple exception class with printf-like formating.
class ExBasic : public std::exception
{
	public:
		/// Set name of function where exception is constructed.
		ExBasic(const char *func, const std::string &fmt)
			: m_func(func), formated(fmt)
		{ }

		virtual ~ExBasic() throw() { }

		/// Return reference to message.
		std::string getMessage() const throw()
		{ return formated.str(); }

		/// @see std::exception::what()
		const char *what() const throw()
		{ return formated.str().c_str(); }

		/// Replace placeholder in format string.
		template<typename T>
		ExBasic &operator % (const T& t)
		{
			formated % t;
			return *this;
		}

		friend std::ostream& operator<< (std::ostream& os, const ExBasic& e)
		{ return os << e.m_func << ": " << e.formated.str(); }

	protected:
		std::string m_func;   ///< Function that threw us.
		eix::format formated; ///< Message formating.
};

/// Automatically fill in the argument for our exceptions.
#define ExBasic(s) ExBasic(__PRETTY_FUNCTION__, s)

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
