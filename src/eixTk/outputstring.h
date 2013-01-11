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

public:
	OutputString() : m_size(0), absolute(false)
	{ }

	explicit OutputString(const std::string &t) :
		m_string(t), m_size(t.size()), absolute(false)
	{ }

	OutputString(const std::string &t, std::string::size_type s) :
		m_string(t), m_size(s), absolute(false)
	{ }

	OutputString(const OutputString &s) :
		m_string(s.m_string), m_size(s.m_size), m_insert(s.m_insert), absolute(s.absolute)
	{ }

	OutputString& operator=(const OutputString &t)
	{
		assign(t);
		return *this;
	}

	OutputString& operator=(const std::string &t)
	{
		assign(t);
		return *this;
	}

	bool empty() const
	{ return (m_string.empty() && m_insert.empty()); }

	const std::string &as_string() const
	{ return m_string; }

	bool is_equal(const OutputString &t) const
	{ return (m_string == t.m_string); }

	void assign(const OutputString &t);
	void assign(const std::string &t);
	void assign(const std::string &t, std::string::size_type s);
	void clear();
	void set_one();
	void append(char s);
	void append_column(std::string::size_type s);
	void append_escape(const char **pos) ATTRIBUTE_NONNULL_;
	void append_smart(char s);
	void append(const std::string &t);
	void append(const std::string &t, std::string::size_type s);
	void append(const OutputString &a);
	void print(std::string *dest, std::string::size_type *s) const ATTRIBUTE_NONNULL_;
	void print(std::string::size_type *s) const ATTRIBUTE_NONNULL_;
};

#endif // SRC_EIXTK_OUTPUTSTRING_H_
