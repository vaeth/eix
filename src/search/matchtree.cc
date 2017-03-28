// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>

// #define DEBUG_MATCHTREE 1

#include "search/matchtree.h"
#include <config.h>

#ifdef DEBUG_MATCHTREE
#include <cstdlib>
#endif

#include <stack>

#include "eixTk/formated.h"
#include "eixTk/i18n.h"
#include "eixTk/likely.h"
#include "eixTk/null.h"
#include "search/packagetest.h"

bool MatchAtom::match(PackageReader * /* p */) {
#ifdef DEBUG_MATCHTREE
	eix::print(m_negate ? " '!' " : " '' ");
	return false;
#endif
	return !m_negate;
}

MatchAtomOperator::~MatchAtomOperator() {
	delete m_left;
	delete m_right;
}

bool MatchAtomOperator::match(PackageReader *p) {
#ifdef DEBUG_MATCHTREE
	eix::print(m_negate ? " !(" : " (");
	if(m_left == NULLPTR) {
		eix::print(" NULLPTR ");
	} else {
		m_left->match(p);
	}
	eix::print((m_operator == AtomAnd) ? '&' : '|');
	if(m_right == NULLPTR) {
		eix::print(" NULLPTR ");
	} else {
		m_right->match(p);
	}
	eix::print(") ");
	return false;
#endif
	bool is_match((m_left == NULLPTR) || m_left->match(p));
	if(is_match) {
		if((m_operator == AtomAnd) && ((likely(m_right != NULLPTR)) && !m_right->match(p))) {
			is_match = false;
		}
	} else {
		if((m_operator == AtomOr) && ((likely(m_right == NULLPTR)) || m_right->match(p))) {
			is_match = true;
		}
	}
	if(m_negate) {
		return !is_match;
	}
	return is_match;
}

MatchAtomTest::~MatchAtomTest() {
#ifndef DEBUG_MATCHTREE
	delete m_test;
#endif
}

bool MatchAtomTest::match(PackageReader *p) {
#ifdef DEBUG_MATCHTREE
	eix::print(m_negate ? " [!" : " [");
	if(m_pipe != NULLPTR) {
		eix::print('|');
	}
	if(m_test == NULLPTR) {
		eix::print("NULLPTR");
	} else {
		eix::print() % *reinterpret_cast<int*>(m_test);
	}
	eix::print("] ");
	return false;
#else
	bool is_match((likely(m_pipe == NULLPTR)) ||
		(((*m_pipe) != NULLPTR) && (*m_pipe)->match(p)));
	if(is_match && ((likely(m_test != NULLPTR)) && !(m_test->match(p)))) {
		is_match = false;
	}
	if(m_negate) {
		return !is_match;
	}
	return is_match;
#endif
}

void MatchAtomTest::set_test(PackageTest *gtest) {
#ifdef DEBUG_MATCHTREE
	static int t_count(0);
	delete gtest;
	int *a(new int(++t_count));
	m_test = reinterpret_cast<PackageTest *>(a);
#else
	delete m_test;
	m_test = gtest;
	if(gtest != NULLPTR) {
		gtest->finalize();
	}
#endif
}

MatchTree::MatchTree(bool default_is_or) {
	root = piperoot = NULLPTR;
	default_operator = (default_is_or ? MatchAtomOperator::AtomOr : MatchAtomOperator::AtomAnd);
	local_negate = local_finished = false;
	parser_stack.push(MatchParseData(&root));
}

MatchTree::~MatchTree() {
	end_parse();
	delete root;
	delete piperoot;
}

bool MatchTree::match(PackageReader *p) {
	return ((root == NULLPTR) || root->match(p));
}

void MatchTree::set_pipetest(PackageTest *gtest) {
	MatchAtomTest *p(new MatchAtomTest);
	p->set_test(gtest);
	if(unlikely(piperoot == NULLPTR)) {
		piperoot = p;
		return;
	}
	MatchAtomOperator *o(new MatchAtomOperator(MatchAtomOperator::AtomOr));
	o->m_left = piperoot;
	o->m_right = p;
	piperoot = o;
}

MatchAtomTest *MatchTree::parse_new_leaf() {
	MatchParseData& top(parser_stack.top());
	MatchAtom **r(top.useright ?
		&(top.subroot->as_operator()->m_right) :
		&(top.subroot));
	if(*r != NULLPTR) {
		return (*r)->as_test();
	}
	MatchAtomTest *t(new MatchAtomTest);
	*r = t;
	return t;
}

/**
Update parser_stack.top() according to local_negate
Clear local_negate afterwards.
**/
void MatchTree::parse_local_negate() {
	if(!local_negate) {
		return;
	}
	local_negate = false;
	MatchParseData& top(parser_stack.top());
	MatchAtom **r(top.useright ?
		&(top.subroot->as_operator()->m_right) :
		&(top.subroot));
	if(*r == NULLPTR) {
		*r = new MatchAtom(true);
		return;
	}
	(*r)->m_negate = !((*r)->m_negate);
	return;
}

/**
Modify parser_stack.top() according to op.
Ignores local_negate and local_finished.
**/
void MatchTree::parse_new_operator(MatchAtomOperator::AtomOperator op) {
	MatchAtomOperator *p(new MatchAtomOperator(op));
	MatchParseData& top(parser_stack.top());
	top.useright = true;
	p->m_left = top.subroot;
	top.subroot = p;
}

void MatchTree::parse_and() {
	parse_local_negate();
	parse_new_operator(MatchAtomOperator::AtomAnd);
	local_finished = false;
}

void MatchTree::parse_or() {
	parse_local_negate();
	parse_new_operator(MatchAtomOperator::AtomOr);
	local_finished = false;
}

void MatchTree::parse_test(PackageTest *gtest, bool with_pipe) {
	if(local_finished) {
		parse_new_operator(default_operator);
	}
	local_finished = true;
	MatchAtomTest *at(parse_new_leaf());
	at->set_test(gtest);
	if(with_pipe) {
		at->m_pipe = &piperoot;
	}
	parse_local_negate();
}

void MatchTree::parse_negate() {
	if(local_negate) {
		parse_new_operator(default_operator);
		local_finished = false;
	} else {
		local_negate = true;
	}
}

void MatchTree::parse_open() {
	if(local_finished) {
		parse_new_operator(default_operator);
		local_finished = false;
	}
	MatchParseData& top(parser_stack.top());
	parser_stack.push(MatchParseData(top.useright ?
		&(top.subroot->as_operator()->m_right) :
		&(top.subroot)));
	if(local_negate) {
		local_negate = false;
		parser_stack.top().negatebrace = true;
	}
}

/**
Internal form of parse_close() which can also clear the
first (root) element on parser_stack() and ignores local_negate.
**/
void MatchTree::parse_closeforce() {
	MatchParseData& top(parser_stack.top());
	if(top.negatebrace) {
		if(top.subroot != NULLPTR) {
			top.subroot->m_negate = !(top.subroot->m_negate);
		} else {
			top.subroot = new MatchAtom(true);
		}
	}
	*(top.parent) = top.subroot;
	parser_stack.pop();
}

void MatchTree::parse_close() {
	if(parser_stack.size() <= 1) {
		eix::say_error(_("warning: ignoring --close without --open"));
		return;
	}
	local_finished = true;
	if(local_negate) {
		parse_new_operator(default_operator);
		parse_local_negate();
	}
	parse_closeforce();
}

void MatchTree::end_parse() {
	parse_local_negate();
	while(!parser_stack.empty()) {
		parse_closeforce();
	}
#ifdef DEBUG_MATCHTREE
	if(root == NULLPTR) {
		eix::say("root=NULLPTR");
	} else {
		root->match(NULLPTR);
		eix::say_empty();
	}
	std::exit(EXIT_SUCCESS);
#endif
}

