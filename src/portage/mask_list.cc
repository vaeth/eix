// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#include "portage/mask_list.h"
#include <config.h>

#include <string>

#include "eixTk/dialect.h"
#include "eixTk/likely.h"
#include "eixTk/null.h"
#include "eixTk/parseerror.h"
#include "eixTk/stringlist.h"
#include "eixTk/stringtypes.h"
#include "eixTk/stringutils.h"
#include "eixTk/unordered_map.h"
#include "eixTk/utils.h"
#include "portage/basicversion.h"
#include "portage/keywords.h"
#include "portage/mask.h"
#include "portage/package.h"

class Version;

using std::string;

template<> bool MaskList<Mask>::add_file(const char *file, Mask::Type mask_type, bool recursive, bool keep_commentlines, const ParseError *parse_error) {
	LineVec lines;
	if(!pushback_lines(file, &lines, recursive, true, (keep_commentlines ? (-1) : 0))) {
		return false;
	}
	bool added(false), finishcomment(false);
	StringList *comments(NULLPTR);
	for(LineVec::iterator it(lines.begin()); likely(it != lines.end()); ++it) {
		if(it->empty()) {
			if(keep_commentlines) {
				finishcomment = false;
				delete comments;
				comments = NULLPTR;
			}
			continue;
		}
		if(keep_commentlines) {
			if((*it)[0] == '#') {
				string line(it->substr(1));
				trim(&line);
				if(finishcomment) {
					finishcomment = false;
					delete comments;
					comments = NULLPTR;
				}
				if(comments == NULLPTR) {
					comments = new StringList;
				}
				comments->push_back(MOVE(line));
				continue;
			}
		}
		finishcomment = true;
		Mask m(mask_type);
		string errtext;
		BasicVersion::ParseResult r(m.parseMask(it->c_str(), &errtext));
		if(unlikely(r != BasicVersion::parsedOK)) {
			parse_error->output(file, lines.begin(), it, errtext);
		}
		if(likely(r != BasicVersion::parsedError)) {
			if(keep_commentlines && (comments != NULLPTR)) {
				comments->finalize();
				if(!(comments->empty())) {
					m.comments = *comments;
				}
			}
			add(m);
			added = true;
		}
	}
	if(keep_commentlines) {
		delete comments;
	}
	return added;
}

// return true if some mask matches
template<> bool MaskList<Mask>::MaskMatches(Package *p) const {
	Get *masks(get(p));
	if(likely(masks == NULLPTR)) {
		return false;
	}
	bool ret(false);
	for(Get::const_iterator it(masks->begin());
		likely(it != masks->end()); ++it) {
		if(it->ismatch(*p)) {
			ret = true;
			break;
		}
	}
	delete masks;
	return ret;
}

// return true if some mask potentially applied
template<> bool MaskList<Mask>::applyMasks(Package *p, Keywords::Redundant check) const {
	Get *masks(get(p));
	if(masks == NULLPTR) {
		return false;
	}
	bool had_mask(false);
	bool had_unmask(false);
	for(Get::const_iterator it(masks->begin());
		likely(it != masks->end()); ++it) {
		it->checkMask(p, check);
		switch(it->get_type()) {
			case Mask::maskMask:
				had_mask = true;
				break;
			case Mask::maskUnmask:
				had_unmask = true;
				break;
			default:
				break;
		}
	}
	delete masks;
	if(!(check & Keywords::RED_MASK)) {
		had_mask = false;
	}
	if(!(check & Keywords::RED_UNMASK)) {
		had_unmask = false;
	}
	if(had_mask || had_unmask) {
		for(Package::iterator i(p->begin());
			likely(i != p->end()); ++i) {
			if(had_mask) {
				if(!i->was_masked())
					i->set_redundant(Keywords::RED_MASK);
			}
			if(had_unmask) {
				if(!i->was_unmasked())
					i->set_redundant(Keywords::RED_UNMASK);
			}
		}
	}
	return true;
}

template<> void MaskList<Mask>::applySetMasks(Version *v, const string& set_name) const {
	Get *masks(get_setname(set_name));
	if(masks == NULLPTR) {
		return;
	}
	for(Get::const_iterator it(masks->begin());
		likely(it != masks->end()); ++it) {
		it->apply(v, false, Keywords::RED_NOTHING);
	}
	delete masks;
}

PreListFilename::PreListFilename(const string& n, const char *label, bool only_repo) {
	filename = n;
	if(label == NULLPTR) {
		know_repo = honour_repo = false;
		return;
	}
	know_repo = true;
	m_repo = label;
	honour_repo = only_repo;
}

const char *PreListFilename::repo() const {
	if(know_repo) {
		return m_repo.c_str();
	}
	return NULLPTR;
}

const char *PreListFilename::repo_if_only() const {
	if(honour_repo) {
		return m_repo.c_str();
	}
	return NULLPTR;
}

bool PreList::handle_lines(const LineVec& lines, FilenameIndex file, const bool only_add, LineNumber *num, bool keep_commentlines) {
	bool changed(false);
	LineNumber number((num == NULLPTR) ? 1 : (*num));
	for(LineVec::const_iterator it(lines.begin());
		likely(it != lines.end()); ++it) {
		if(handle_line(*it, file, number++, only_add, keep_commentlines)) {
			changed = true;
		}
	}
	if(num != NULLPTR) {
		*num = number;
	}
	return changed;
}

bool PreList::handle_line(const std::string& line, FilenameIndex file, LineNumber number, bool only_add, bool keep_commentlines) {
	if(line.empty()) {
		if(keep_commentlines) {
			return add_line(line, file, number, keep_commentlines);
		}
		return false;
	}
	if(only_add || (line[0] != '-')) {
		return add_line(line, file, number, keep_commentlines);
	}
	return remove_line(line.c_str() + 1);
}

bool PreList::add_line(const std::string& line, FilenameIndex file, LineNumber number, bool keep_commentlines) {
	static string *unique = NULLPTR;
	LineVec l;
	if(keep_commentlines) {
		if(line.empty() || (line[0] == '#')) {
			if(unique == NULLPTR) {
				unique = new string("#a");
			} else {
				for(string::size_type i(unique->length() - 1);;) {
					char a((*unique)[i]);
					if(a != 'z') {
						(*unique)[i] = ++a;
						break;
					} else {
						(*unique)[i] = 'a';
						if(--i == 0) {
							unique->insert(1, 1, 'a');
							break;
						}
					}
				}
			}
			l.PUSH_BACK(*unique);
			l.PUSH_BACK(line);
			add_splitted(l, file, number);
			return false;
		}
	}
	split_string(&l, line);
	return add_splitted(l, file, number);
}

bool PreList::remove_line(const std::string& line) {
	LineVec l;
	split_string(&l, line);
	return remove_splitted(l);
}

bool PreList::add_splitted(const LineVec& line, FilenameIndex file, LineNumber number) {
	if(line.empty()) {
		return false;
	}
	Have::iterator it(have.find(line));
	if(it == have.end()) {
		have[line] = order.size();
		order.EMPLACE_BACK(PreListOrderEntry, (line, file, number));
		return true;
	}
	PreListOrderEntry& e(order[it->second]);
	e.filename_index = file;
	e.linenumber = number;
	if(e.removed) {
		e.removed = false;
		return true;
	} else {
		e.locally_double = true;
		return false;
	}
}

bool PreList::remove_splitted(const LineVec& line) {
	if(line.empty()) {
		return false;
	}
	Have::iterator it(have.find(line));
	if(it == have.end()) {
		return false;
	}
	bool& removed = order[it->second].removed;
	bool ret(!removed);
	removed = true;
	return ret;
}

bool PreList::remove_all() {
	bool ret(false);
	for(Order::iterator it(order.begin()); it != order.end(); ++it) {
		bool& removed = it->removed;
		if(!removed) {
			ret = removed = true;
		}
	}
	return ret;
}

void PreList::finalize() {
	if(finalized) {
		return;
	}
	finalized = true;
	if(order.empty()) {
		return;
	}

	// We first build a map of the result and
	// set the duplicate names to removed.
	typedef UNORDERED_MAP<string, PreListEntry> Res;
	Res result;
	for(Order::const_iterator it(order.begin());
		likely(it != order.end()); ++it) {
		if(unlikely(it->removed)) {
			continue;
		}
		if(it->empty()) {
			continue;
		}
		PreListOrderEntry::const_iterator curr(it->begin());
		PreListEntry *e;
		Res::iterator r(result.find(*curr));
		if(likely(r == result.end())) {
			e = &(result[*curr]);
		} else {
			e = &(r->second);
		}
		e->filename_index = it->filename_index;
		e->linenumber     = it->linenumber;
		e->locally_double = it->locally_double;
		e->name = *curr;
		for(++curr; curr != it->end(); ++curr) {
			e->args.PUSH_BACK(*curr);
		}
	}

	// Now we sort the result according to the order.
	for(Order::const_iterator it(order.begin()); likely(it != order.end()); ++it) {
		if(likely(!it->removed)) {
			PUSH_BACK(result[(*it)[0]]);
		}
	}

	order.clear();
	have.clear();
}

void PreList::initialize(MaskList<Mask> *l, Mask::Type t, bool keep_commentlines, const ParseError *parse_error) {
	finalize();
	StringList *comments(NULLPTR);
	bool finishcomment(false);
	PreListEntry::FilenameIndex lastfile(0);
	for(const_iterator it(begin()); likely(it != end()); ++it) {
		if(it->name.empty()) {
			continue;
		}
		PreListEntry::FilenameIndex currfile(it->filename_index);
		if(lastfile != currfile) {
			// In the first iteration, this may be executed by accident but does not hurt
			delete comments;
			comments = NULLPTR;
		}
		lastfile = currfile;
		if(it->name[0] == '#') {
			if(!keep_commentlines) {
				continue;
			}
			const string& comment(it->args[0]);
			if(comment.empty()) {
				finishcomment = false;
				delete comments;
				comments = NULLPTR;
				continue;
			}
			string line(comment.substr(1));
			trim(&line);
			if(finishcomment) {
				finishcomment = false;
				delete comments;
				comments = NULLPTR;
			}
			if(comments == NULLPTR) {
				comments = new StringList;
			}
			comments->push_back(MOVE(line));
			continue;
		}
		finishcomment = true;
		Mask m(t);
		string errtext;
		BasicVersion::ParseResult r(m.parseMask(it->name.c_str(),
			&errtext, 1, repo_if_only(it->filename_index)));
		if(unlikely(r != BasicVersion::parsedOK)) {
			parse_error->output(file_name(it->filename_index),
				it->linenumber, it->name + " ...", errtext);
		}
		if(likely(r != BasicVersion::parsedError)) {
			if(keep_commentlines && (comments != NULLPTR)) {
				comments->finalize();
				if(!(comments->empty())) {
					m.comments = *comments;
				}
			}
			l->add(m);
		}
	}
	if(keep_commentlines) {
		delete comments;
	}
	l->finalize();
	clear();
}

void PreList::initialize(MaskList<KeywordMask> *l, string raised_arch, const ParseError *parse_error) {
	finalize();
	for(const_iterator it(begin()); likely(it != end()); ++it) {
		KeywordMask m;
		string errtext;
		BasicVersion::ParseResult r(m.parseMask(it->name.c_str(),
			&errtext, 1, repo_if_only(it->filename_index)));
		if(unlikely(r != BasicVersion::parsedOK)) {
			parse_error->output(file_name(it->filename_index),
				it->linenumber, it->name + " ...", errtext);
		}
		if(likely(r != BasicVersion::parsedError)) {
			if(it->args.empty()) {
				m.keywords = raised_arch;
			} else {
				join_to_string(&(m.keywords), it->args);
			}
			m.locally_double = it->locally_double;
			l->add(m);
		}
	}
	l->finalize();
	clear();
}

void PreList::initialize(MaskList<PKeywordMask> *l, const ParseError *parse_error) {
	finalize();
	for(const_iterator it(begin()); likely(it != end()); ++it) {
		PKeywordMask m;
		string errtext;
		BasicVersion::ParseResult r(m.parseMask(it->name.c_str(),
			&errtext, 1, repo_if_only(it->filename_index)));
		if(unlikely(r != BasicVersion::parsedOK)) {
			parse_error->output(file_name(it->filename_index),
				it->linenumber, it->name + " ...", errtext);
		}
		if(likely(r != BasicVersion::parsedError)) {
			join_to_string(&(m.keywords), it->args);
			l->add(m);
		}
	}
	l->finalize();
	clear();
}
