// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#if !defined(EIX__EXCEPTIONS_H__)
#define EIX__EXCEPTIONS_H__

#include <eixTk/formated.h>
#include <eixTk/stringutils.h>

#include <iostream>
#include <string>
#include <vector>
#include <cerrno>

/// Simple exception class with printf-like formating.
class ExBasic : public std::exception
{
	public:
		/// Set name of function where exception is constructed.
		ExBasic(const char *func, const std::string &fmt)
			: m_func(func), m_formated(fmt)
		{ }

		virtual ~ExBasic() throw() { }

		/// return signature of throwing function
		virtual const std::string& from() const throw()
		{ return m_func; }

		/// Return reference to message.
		virtual const std::string& getMessage() const throw()
		{
			if (m_cache.empty())
				m_cache = m_formated.str();
			return m_cache;
		}

		/// @see std::exception::what()
		virtual const char *what() const throw()
		{ return getMessage().c_str(); }

		/// Replace placeholder in format string.
		template<typename T>
		ExBasic &operator % (const T& t)
		{
			m_formated % t;
			return *this;
		}

	private:
		std::string m_func;   ///< Function that threw us.
		eix::format m_formated; ///< Message formating.
		mutable std::string m_cache; ///< Message that we got from m_formated;
};

/// Like ExBasic, but append system error message.
class SysError : public ExBasic
{
	public:
		SysError(const char *func, const std::string& format, int e = 0)
			: ExBasic(func, format),
			  m_error(strerror(e ? e : errno))
		{ }

		virtual ~SysError() throw() { }

		/// Return reference to message.
		virtual const std::string& getMessage() const throw();

		/// Replace placeholder in format string.
		template<typename T>
		SysError &operator % (const T& t)
		{
			ExBasic::operator%(t);
			return *this;
		}

	private:
		const std::string m_error; // result of strerror
		mutable std::string m_cache; // formated message from ExBasic
};

/// Automatically fill in the argument for our exceptions.
#define ExBasic(s) ExBasic(__PRETTY_FUNCTION__, s)
#define SysError(s) SysError(__PRETTY_FUNCTION__, s)

inline std::ostream& operator<< (std::ostream& os, const ExBasic& e)
{
	return os << e.from() << ": " << e.getMessage();
}

/// Provide a common look for error-messages for parse-errors in
/// portage.{mask,keywords,..}.
void portage_parse_error(const std::string &file, const int line_nr, const std::string& line, const std::exception &e);

template<class Iterator>
inline void
portage_parse_error(const std::string &file, const Iterator &begin, const Iterator &line, const std::exception &e)
{
	portage_parse_error(file, std::distance(begin, line) + 1, *line, e);
}

namespace eix
{
	template<bool B>
		struct m_StaticAssert;

	template<>
		struct m_StaticAssert<true>
		{ static void empty(void) { } };
}

/** Static assertion of expr.
 *
 * Fail to compile if expr is false because m_StaticAssert<T>::empty is only
 * defined for m_StaticAssert<true>::empty.  empty() is an empty function,
 * and the call should get optimized away be the compiler.
 */
#define EIX_STATIC_ASSERT(expr) eix::m_StaticAssert<expr>::empty()

#endif /* EIX__EXCEPTIONS_H__ */
