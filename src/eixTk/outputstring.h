// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>

#ifndef SRC_EIXTK_OUTPUTSTRING_H_
#define SRC_EIXTK_OUTPUTSTRING_H_ 1

#include <string>
#include <vector>

#include "eixTk/stringtypes.h"

class OutputString {
	private:
		typedef std::vector<WordSize> InsertType;
		std::string m_string;
		/**
		apparent size (within the line in absolute mode)
		**/
		WordSize m_size;
		InsertType m_insert;
		/**
		absolute means string is known to start from beginning of line
		**/
		bool absolute;

		/**
		Append std::string to OutputString
		@param ts true/physical size of the current OutputString
		@param s  apparent/optical size within the line
		@param a  are we currently in absolute mode?
		**/
		void append_internal(const std::string& t, WordSize ts, WordSize s, bool a);

		void append_internal(const std::string& t) {
			append_internal(t, 0, 0, false);
		}

	public:
		OutputString() : m_size(0), absolute(false) {
		}

		explicit OutputString(const std::string& t) : m_string(t) {
			append_internal(t);
		}

		explicit OutputString(const char *s) : m_string(s) {
			append_internal(s);
		}

		OutputString(const OutputString& s) :
			m_string(s.m_string), m_size(s.m_size), m_insert(s.m_insert), absolute(s.absolute) {
		}

		/**
		@arg s is the size in visible characters
		**/
		OutputString(const std::string& t, WordSize s) :
			m_string(t), m_size(s), absolute(false) {
		}

		OutputString& operator=(const OutputString& t) {
			assign(t);
			return *this;
		}

		OutputString& operator=(const std::string& t) {
			assign_smart(t);
			return *this;
		}

		bool empty() const {
			return (m_string.empty() && m_insert.empty());
		}

		const std::string& as_string() const {
			return m_string;
		}

		ATTRIBUTE_PURE bool is_equal(const OutputString& t) const;
		void assign(const OutputString& t);
		/**
		@arg s is the size in visible characters
		**/
		void assign(const std::string& t, WordSize s);
		void assign_smart(const std::string& t);
		void assign_smart(const char *s);

		/**
		assign_fast must not be used with utf8 or tabs/newlines
		**/
		void assign_fast(const std::string& t);
		/**
		assign_fast must not be used with utf8 or tabs/newlines
		**/
		void assign_fast(const char *s);
		/**
		assign_fast must not be used with utf8 or tabs/newlines
		**/
		void assign_fast(const char s);
		void clear();
		void set_one();
		void append_smart(char s);
		/**
		append_fast must not be used with utf8 or tabs/newlines
		**/
		void append_fast(char s);
		void append_column(WordSize s);
		ATTRIBUTE_NONNULL_ void append_escape(const char **pos);
		/**
		@arg s is the size in visible characters
		**/
		void append(const std::string& t, WordSize s);
		void append_smart(const std::string& t);
		/**
		append_fast must not be used with utf8 or tabs/newlines
		**/
		void append_fast(const std::string& t);
		/**
		append_fast must not be used with utf8 or tabs/newlines
		**/
		void append_fast(const char *c);
		void append(const OutputString& a);
		ATTRIBUTE_NONNULL_ void print(std::string *dest, WordSize *s) const;
		ATTRIBUTE_NONNULL_ void print(WordSize *s) const;
};

#endif  // SRC_EIXTK_OUTPUTSTRING_H_
