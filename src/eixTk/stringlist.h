// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef SRC_EIXTK_STRINGLIST_H_
#define SRC_EIXTK_STRINGLIST_H_

#undef STRINGLIST_COUNTER
// Setting STRINGLIST_COUNTER should save memory but crashes for some reason
// #define STRINGLIST_COUNTER 1

#include <string>
#include <vector>

#ifdef STRINGLIST_COUNTER
#include "eixTk/inttypes.h"
#endif
#include "eixTk/null.h"

class StringList;

class StringListContent {
		friend class StringList;
	private:
		std::vector<std::string> m_list;

	protected:
#ifdef STRINGLIST_COUNTER
		uint32_t usage;
#endif
		void finalize();

		bool empty()
		{ return m_list.empty(); }

		void push_back(const std::string &s)
		{ m_list.push_back(s); }

		void append_to_string(std::string *s, const std::string &skip) const ATTRIBUTE_NONNULL_;
};

inline static
bool operator<(const StringList &a, const StringList &b) ATTRIBUTE_PURE;

class StringList {
		friend bool operator<(const StringList &a, const StringList &b);

	private:
		StringListContent *ptr;

	public:
		StringList() : ptr(NULLPTR)
		{ }

#ifdef STRINGLIST_COUNTER
		explicit StringList(const StringList &s);

		~StringList();
#endif
		void finalize();

		bool empty() const
		{ return (ptr == NULLPTR); }

		void push_back(const std::string &s);

		void append_to_string(std::string *s, const std::string &skip) const ATTRIBUTE_NONNULL_
		{
			if(ptr != NULLPTR) {
				ptr->append_to_string(s, skip);
			}
		}
};

inline static bool
operator<(const StringList &a, const StringList &b)
{
	return (a.ptr < b.ptr);
}

#endif  // SRC_EIXTK_STRINGLIST_H_
