// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef SRC_EIXTK_OUTPUTSTRING_H_
#define SRC_EIXTK_OUTPUTSTRING_H_ 1

#include <string>
#include <vector>

class OutputString {
private:
	std::string m_string;
	std::string::size_type m_size;
	std::vector<std::string::size_type> m_insert;
	bool absolute;

	void append_internal(const std::string &t, std::string::size_type ts = 0, std::string::size_type s = 0, bool a = false);

public:
	OutputString() : m_size(0), absolute(false)
	{ }

	explicit OutputString(const std::string &t) : m_string(t)
	{ append_internal(t); }

	explicit OutputString(const char *s) : m_string(s)
	{ append_internal(s); }

	explicit OutputString(const OutputString &s) :
		m_string(s.m_string), m_size(s.m_size), m_insert(s.m_insert), absolute(s.absolute)
	{ }

	explicit OutputString(const std::string &t, std::string::size_type s) :
		m_string(t), m_size(s), absolute(false)
	{ }

	OutputString& operator=(const OutputString &t)
	{
		assign(t);
		return *this;
	}

	OutputString& operator=(const std::string &t)
	{
		assign_smart(t);
		return *this;
	}

	bool empty() const
	{ return (m_string.empty() && m_insert.empty()); }

	const std::string &as_string() const
	{ return m_string; }

	bool is_equal(const OutputString &t) const ATTRIBUTE_PURE;
	void assign(const OutputString &t);
	void assign(const std::string &t, std::string::size_type s);
	void assign_smart(const std::string &t);
	void assign_smart(const char *s);
	void assign_fast(const std::string &t);
	void assign_fast(const char *s);
	void assign_fast(const char s);
	void clear();
	void set_one();
	void append_smart(char s);
	void append_fast(char s);
	void append_column(std::string::size_type s);
	void append_escape(const char **pos) ATTRIBUTE_NONNULL_;
	void append(const std::string &t, std::string::size_type s);
	void append_smart(const std::string &t);
	void append_fast(const std::string &t);
	void append_fast(const char *c);
	void append(const OutputString &a);
	void print(std::string *dest, std::string::size_type *s) const ATTRIBUTE_NONNULL_;
	void print(std::string::size_type *s) const ATTRIBUTE_NONNULL_;
};

#endif  // SRC_EIXTK_OUTPUTSTRING_H_
