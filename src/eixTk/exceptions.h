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
			: m_func(func), m_formated(fmt)
		{ }

		virtual ~ExBasic() throw() { }

		/// Return reference to message.
		const std::string& getMessage() const throw()
		{
			if (m_cache.empty())
				m_cache = m_formated.str();
			return m_cache;
		}

		/// @see std::exception::what()
		const char *what() const throw()
		{ return getMessage().c_str(); }

		/// Replace placeholder in format string.
		template<typename T>
		ExBasic &operator % (const T& t)
		{
			m_formated % t;
			return *this;
		}

		friend std::ostream& operator<< (std::ostream& os, const ExBasic& e)
		{ return os << e.m_func << ": " << e.m_formated.str(); }

	protected:
		std::string m_func;   ///< Function that threw us.
		eix::format m_formated; ///< Message formating.
		mutable std::string m_cache; ///< Mesage that we got from m_formated;
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

namespace eix 
{ 
	template<bool B> 
		struct _StaticAssert; 

	template<> 
		struct _StaticAssert<true> 
		{ static void empty(void) { } }; 
} 

/** Static assertion of expr. 
 * 
 * Fail to compile if expr is false because _StaticAssert<T>::empty is only 
 * defined for _StaticAssert<true>::empty.  empty() is a empty function and the 
 * call should get optimized away be the compiler. 
 */ 
#define EIX_STATIC_ASSERT(expr) eix::_StaticAssert<expr>::empty() 

#endif /* __EXCEPTIONS_H__ */
