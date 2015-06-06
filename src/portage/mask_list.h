// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#ifndef SRC_PORTAGE_MASK_LIST_H_
#define SRC_PORTAGE_MASK_LIST_H_ 1

#include <fnmatch.h>

#include <list>
#include <map>
#include <string>
#include <vector>

#include "eixTk/likely.h"
#include "eixTk/null.h"
#include "eixTk/ptr_list.h"
#include "eixTk/stringtypes.h"
#include "eixTk/stringutils.h"
#include "portage/keywords.h"
#include "portage/mask.h"
#include "portage/package.h"

class Package;
class Version;

template<typename m_Type> class Masks : public std::list<m_Type> {
	public:
		typedef typename std::list<m_Type> MasksList;
		using MasksList::begin;
		using MasksList::end;
		using MasksList::insert;
		using MasksList::push_back;
		typedef typename MasksList::iterator iterator;
		typedef typename MasksList::const_iterator const_iterator;

		Masks() : MasksList() {
		}

		void add(const m_Type& m) {
			for(iterator it(begin()); it != end(); ++it) {
				if(m.priority < it->priority) {
					insert(it, m);
					return;
				}
			}
			push_back(m);
		}
};

template<typename m_Type> class MaskList {
	private:
		typedef typename Masks<m_Type>::const_iterator m_const_iterator;
		typedef typename std::map<std::string, Masks<m_Type> > FullType;
		typedef typename FullType::const_iterator full_const_iterator;
		typedef typename std::map<std::string, Masks<m_Type> > ExactType;
		typedef typename ExactType::const_iterator exact_const_iterator;

		ExactType exact_name;
		FullType full_name;

	public:
		typedef typename eix::ptr_list<const m_Type> Get;

		bool empty() const {
			return (exact_name.empty() && full_name.empty());
		}

		void clear() {
			exact_name.clear();
			full_name.clear();
		}

		inline static bool match_full(const std::string& mask, const std::string& name) {
			return !fnmatch(mask.c_str(), name.c_str(), FNM_PATHNAME);
		}

		bool match_full(const std::string& full) const {
			if(exact_name.find(full) != exact_name.end()) {
				return true;
			}
			for(full_const_iterator it(full_name.begin());
				likely(it != full_name.end()); ++it) {
				if(unlikely(match_full(it->first, full))) {
					return true;
				}
			}
			return false;
		}

		bool match_name(const Package *p) const ATTRIBUTE_NONNULL_ {
			return match_full(p->category + "/" + p->name);
		}

		inline static void push_result(Get **l, const Masks<m_Type>& r) ATTRIBUTE_NONNULL_ {
			if(*l == NULLPTR) {
				*l = new Get;
			}
			for(m_const_iterator m(r.begin()); likely(m != r.end()); ++m) {
				(*l)->push_back(&*m);
			}
		}

		Get *get_full(const std::string& full) const {
			Get *l(NULLPTR);
			for(full_const_iterator it(full_name.begin());
				likely(it != full_name.end()); ++it) {
				if(unlikely(match_full(it->first, full))) {
					push_result(&l, it->second);
				}
			}
			exact_const_iterator it(exact_name.find(full));
			if(it != exact_name.end()) {
				push_result(&l, it->second);
			}
			return l;
		}

		Get *get_setname(const std::string& setname) const {
			return get_full(std::string(SET_CATEGORY) + "/" + setname);
		}

		Get *get(const Package *p) const ATTRIBUTE_NONNULL_ {
			return get_full(p->category + "/" + p->name);
		}

		void add(const m_Type& m) {
			std::string full(m.getCategory());
			full.append(1, '/');
			full.append(m.getName());
			if(full.find_first_of("*?[") == std::string::npos) {
				exact_name[full].add(m);
				return;
			}
			full_name[full].add(m);
		}

		/* return true if something was added */
		bool add_file(const char *file, Mask::Type mask_type, bool recursive, bool keep_commentlines) ATTRIBUTE_NONNULL_;
		bool add_file(const char *file, Mask::Type mask_type, bool recursive) ATTRIBUTE_NONNULL_ {
			return add_file(file, mask_type, recursive, false);
		}

		/** This can be optionally called after the last add():
		 *  It will release memory. */
		void finalize() {
		}

		void applyListItems(Package *p) const ATTRIBUTE_NONNULL_ {
			Get *masks(get(p));
			if(masks == NULLPTR) {
				return;
			}
			for(typename Get::const_iterator it(masks->begin());
				likely(it != masks->end()); ++it) {
				it->applyItem(p);
			}
			delete masks;
		}

		void applyListSetItems(Version *v, const std::string& set_name) const ATTRIBUTE_NONNULL_ {
			Get *masks(get_setname(set_name));
			if(masks == NULLPTR) {
				return;
			}
			for(typename Get::const_iterator it(masks->begin());
				likely(it != masks->end()); ++it) {
				it->applyItem(v);
			}
			delete masks;
		}

		// return true if some mask potentially applied
		bool applyMasks(Package *p, Keywords::Redundant check) const ATTRIBUTE_NONNULL_;
		bool applyMasks(Package *p) const ATTRIBUTE_NONNULL_ {
			return applyMasks(p, Keywords::RED_NOTHING);
		}

		// return true if some mask matches
		bool MaskMatches(Package *p) const ATTRIBUTE_NONNULL_;

		void applySetMasks(Version *v, const std::string& set_name) const ATTRIBUTE_NONNULL_;
};

// This is only needed for PreList
class PreListEntry {
	public:
		typedef WordVec::size_type FilenameIndex;
		typedef LineVec::size_type LineNumber;
		std::string name;
		WordVec args;
		FilenameIndex filename_index;
		LineNumber linenumber;
		bool locally_double;
};

// This is only needed for PreList
class PreListOrderEntry : public LineVec {
	public:
		typedef PreListEntry::FilenameIndex FilenameIndex;
		typedef PreListEntry::LineNumber LineNumber;
		typedef LineVec super;
		typedef super::const_iterator const_iterator;
		using super::begin;
		using super::end;
		using super::operator[];
		FilenameIndex filename_index;
		LineNumber linenumber;
		bool removed, locally_double;

		PreListOrderEntry(const super& line, FilenameIndex file, LineNumber number)
			: super(line), filename_index(file), linenumber(number), removed(false), locally_double(false) {
		}
};

class PreListFilename {
	private:
		std::string filename, m_repo;
		bool know_repo, honour_repo;

	public:
		PreListFilename(const std::string& n, const char *label, bool only_repo);

		const std::string& name() const {
			return filename;
		}

		const char *repo() const ATTRIBUTE_PURE;

		const char *repo_if_only() const ATTRIBUTE_PURE;
};

/* The PreList is needed to Prepare a MaskList:
 *
 * Until we call finalize() or initialize(), one can insert and delete lines.
 * (A line is a std::vector<std::string>). Duplicate lines are recognized, too.
 * However, the original order is preserved.
 * Moreover, after finalize() the entries are collected: For the lines
 *   foo/bar 1
 *   foo/bar 2
 *   =foo/bar-1 3
 *   =foo/bar-1 4
 *
 * the result looks like this
 *   foo/bar    -> 1 2
 *   =foo/bar-1 -> 3 4
 *
 * This corresponds to portage's sorting.
 */
class PreList : public std::vector<PreListEntry> {
	public:
		typedef PreListEntry::FilenameIndex FilenameIndex;
		typedef PreListEntry::LineNumber LineNumber;
		typedef std::vector<PreListEntry> super;
		typedef super::const_iterator const_iterator;
		using super::begin;
		using super::end;
		using super::size;
		using super::empty;
		using super::clear;

	private:
		using super::push_back;

		typedef std::vector<PreListOrderEntry> Order;
		Order order;
		typedef std::vector<PreListFilename> FileNames;
		FileNames filenames;
		typedef std::map<std::vector<std::string>, Order::size_type> Have;
		Have have;
		bool finalized;

	public:
		void clear() {
			finalize();
			filenames.clear();
			super::clear();
		}

		const std::string& file_name(FilenameIndex file) const {
			return filenames[file].name();
		}

		const char *repo(FilenameIndex file) const {
			return filenames[file].repo();
		}

		const char *repo_if_only(FilenameIndex file) const {
			return filenames[file].repo_if_only();
		}

		FilenameIndex push_name(const std::string& filename, const char *reponame, bool only_repo) {
			FilenameIndex i(filenames.size());
			filenames.push_back(PreListFilename(filename, reponame, only_repo));
			return i;
		}

		PreList() : finalized(false) {
		}

		PreList(const std::vector<std::string>& lines, const std::string& filename, const char *reponame, bool only_add, bool only_repo) : finalized(false) {
			handle_file(lines, filename, reponame, only_add, false, only_repo);
		}

		/// return true if something was changed
		bool handle_file(const std::vector<std::string>& lines, const std::string& filename, const char *reponame, bool only_add, bool keep_commentlines, bool only_repo) {
			return handle_lines(lines, push_name(filename, reponame, only_repo), only_add, NULLPTR, keep_commentlines);
		}

		/// return true if something was changed
		bool handle_lines(const std::vector<std::string>& lines, FilenameIndex file, bool only_add, LineNumber *num, bool keep_commentlines);

		/// return true if something was changed
		bool handle_line(const std::string& line, FilenameIndex file, LineNumber number, bool only_add, bool keep_commentlines);

		/// return true if something was changed
		bool add_line(const std::string& line, FilenameIndex file, LineNumber number, bool keep_commentlines);

		/// return true if something was changed
		bool remove_line(const std::string& line);

		/// return true if something was changed
		bool add_splitted(const std::vector<std::string>& line, FilenameIndex file, LineNumber number);

		/// return true if something was changed
		bool remove_splitted(const std::vector<std::string>& line);

		void finalize();

		void initialize(MaskList<Mask> *l, Mask::Type t, bool keep_commentlines) ATTRIBUTE_NONNULL_;
		void initialize(MaskList<Mask> *l, Mask::Type t) ATTRIBUTE_NONNULL_ {
			initialize(l, t, false);
		}

		void initialize(MaskList<KeywordMask> *l, std::string raised_arch) ATTRIBUTE_NONNULL_;

		void initialize(MaskList<PKeywordMask> *l) ATTRIBUTE_NONNULL_;
};

#endif  // SRC_PORTAGE_MASK_LIST_H_
