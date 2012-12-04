// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef SRC_EIXTK_STRINGLIST_H_
#define SRC_EIXTK_STRINGLIST_H_

// Without STRINGLIST_FREE, stringlists cannot be completely destructed.
// However, using STRINGLIST_FREE has a slight memory and code overhead.
#define STRINGLIST_FREE 1

#include <string>
#include <vector>

#ifdef STRINGLIST_FREE
#include "eixTk/inttypes.h"
#endif
#include "eixTk/null.h"

class StringList;

class StringListContent {
		friend class StringList;
	private:
		std::vector<std::string> m_list;

	protected:
#ifdef STRINGLIST_FREE
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

#ifdef STRINGLIST_FREE
		StringList& operator=(const StringList &s);

		StringList(const StringList &s)
		{ *this = s; }

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
