// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include <config.h>
#include "matchtree.h"
#include <eixTk/i18n.h>
#include <eixTk/likely.h>
#include <eixTk/unused.h>
#include <search/packagetest.h>

#include <iostream>
#include <stack>

#include <cstddef>

// #define DEBUG_MATCHTREE 1

#ifdef DEBUG_MATCHTREE
#include <cstdlib>
#endif

using namespace std;

bool
MatchAtom::match(PackageReader *p ATTRIBUTE_UNUSED)
{
	UNUSED(p);
#ifdef DEBUG_MATCHTREE
	cout << (m_negate ? " '!' " : " '' ");
	return false;
#endif
	return !m_negate;
}

MatchAtomOperator::~MatchAtomOperator()
{
	if(likely(m_left != NULL)) {
		delete m_left;
	}
	if(likely(m_right != NULL)) {
		delete m_right;
	}
}

bool
MatchAtomOperator::match(PackageReader *p)
{
#ifdef DEBUG_MATCHTREE
	cout << (m_negate ? " !(" : " (");
	if(m_left == NULL) cout << " NULL "; else m_left->match(p);
	cout << ((m_operator == AtomAnd) ? '&' : '|');
	if(m_right == NULL) cout << " NULL "; else m_right->match(p);
	cout << ") ";
	return false;
#endif
	bool is_match((m_left == NULL) || m_left->match(p));
	if(is_match) {
		if((m_operator == AtomAnd) && ((likely(m_right != NULL)) && !m_right->match(p))) {
			is_match = false;
		}
	}
	else {
		if((m_operator == AtomOr) && ((likely(m_right == NULL)) || m_right->match(p))) {
			is_match = true;
		}
	}
	if(m_negate) {
		return !is_match;
	}
	return is_match;
}

MatchAtomTest::~MatchAtomTest()
{
#ifdef DEBUG_MATCHTREE
	return;
#endif
	if(likely(m_test != NULL)) {
		delete m_test;
	}
}

bool
MatchAtomTest::match(PackageReader *p)
{
#ifdef DEBUG_MATCHTREE
	cout << (m_negate ? " [!" : " [");
	if(m_pipe != NULL) cout << "|";
	if(m_test == NULL) cout << "NULL"; else cout << *reinterpret_cast<int*>(m_test);
	cout << "] ";
	return false;
#endif
	bool is_match((likely(m_pipe == NULL)) ||
		(((*m_pipe) != NULL) && (*m_pipe)->match(p)));
	if(is_match && ((likely(m_test != NULL)) && !(m_test->match(p)))) {
		is_match = false;
	}
	if(m_negate) {
		return !is_match;
	}
	return is_match;
}

void
MatchAtomTest::set_test(PackageTest *gtest)
{
#ifdef DEBUG_MATCHTREE
	static int t_count(0);
	if(gtest != NULL) { delete gtest; }
	int *a = new int(++t_count);
	m_test = reinterpret_cast<PackageTest *>(a);
	return;
#endif
	if(unlikely(m_test != NULL)) {
		delete m_test;
	}
	m_test = gtest;
	if(gtest != NULL) {
		gtest->finalize();
	}
}

MatchTree::MatchTree(bool default_is_or)
{
	root = piperoot = NULL;
	default_operator = (default_is_or ? MatchAtomOperator::AtomOr : MatchAtomOperator::AtomAnd);
	local_negate = local_finished = false;
	parser_stack.push(MatchParseData(&root));
}

MatchTree::~MatchTree()
{
	end_parse();
	if(likely(root != NULL))
		delete root;
	if(piperoot != NULL)
		delete piperoot;
}

void
MatchTree::set_pipetest(PackageTest *gtest)
{
	MatchAtomTest *p(new MatchAtomTest);
	p->set_test(gtest);
	if(unlikely(piperoot == NULL)) {
		piperoot = p;
		return;
	}
	MatchAtomOperator *o(new MatchAtomOperator(MatchAtomOperator::AtomOr));
	o->m_left = piperoot;
	o->m_right = p;
	piperoot = o;
}

MatchAtomTest *
MatchTree::parse_new_leaf()
{
	MatchParseData &top(parser_stack.top());
	MatchAtom **r(top.useright ?
		&(top.subroot->as_operator()->m_right) :
		&(top.subroot));
	if(*r != NULL) {
		return (*r)->as_test();
	}
	MatchAtomTest *t(new MatchAtomTest);
	*r = t;
	return t;
}

/// Update parser_stack.top() according to local_negate
/// Clear local_negate afterwards.
void
MatchTree::parse_local_negate()
{
	if(!local_negate) {
		return;
	}
	local_negate = false;
	MatchParseData &top(parser_stack.top());
	MatchAtom **r(top.useright ?
		&(top.subroot->as_operator()->m_right) :
		&(top.subroot));
	if(*r == NULL) {
		*r = new MatchAtom(true);
		return;
	}
	(*r)->m_negate = !((*r)->m_negate);
	return;
}

/// Modifies parser_stack.top() according to op.
/// Ignores local_negate and local_finished.
void
MatchTree::parse_new_operator(MatchAtomOperator::AtomOperator op)
{
	MatchAtomOperator *p(new MatchAtomOperator(op));
	MatchParseData &top(parser_stack.top());
	top.useright = true;
	p->m_left = top.subroot;
	top.subroot = p;
}

void
MatchTree::parse_and()
{
	parse_local_negate();
	parse_new_operator(MatchAtomOperator::AtomAnd);
	local_finished = false;
}

void
MatchTree::parse_or()
{
	parse_local_negate();
	parse_new_operator(MatchAtomOperator::AtomOr);
	local_finished = false;
}


void
MatchTree::parse_test(PackageTest *gtest, bool with_pipe)
{
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

void
MatchTree::parse_negate()
{
	if(local_negate) {
		parse_new_operator(default_operator);
		local_finished = false;
	}
	else {
		local_negate = true;
	}
}

void
MatchTree::parse_open()
{
	if(local_finished) {
		parse_new_operator(default_operator);
		local_finished = false;
	}
	MatchParseData &top(parser_stack.top());
	parser_stack.push(top.useright ?
		&(top.subroot->as_operator()->m_right) :
		&(top.subroot));
	if(local_negate) {
		local_negate = false;
		parser_stack.top().negatebrace = true;
	}
}

/// Internal form of parse_close() which can also clear the
/// first (root) element on parser_stack() and ignores local_negate.
void
MatchTree::parse_closeforce()
{
	MatchParseData &top(parser_stack.top());
	if(top.negatebrace) {
		if(top.subroot != NULL) {
			top.subroot->m_negate = !(top.subroot->m_negate);
		}
		else {
			top.subroot = new MatchAtom(true);
		}
	}
	*(top.parent) = top.subroot;
	parser_stack.pop();
}

void
MatchTree::parse_close()
{
	if(parser_stack.size() <= 1) {
		cerr << _("warning: ignoring --close without --open") << endl;
		return;
	}
	local_finished = true;
	if(local_negate) {
		parse_new_operator(default_operator);
		parse_local_negate();
	}
	parse_closeforce();
}

void
MatchTree::end_parse()
{
	parse_local_negate();
	while(!parser_stack.empty()) {
		parse_closeforce();
	}
#ifdef DEBUG_MATCHTREE
	if(root == NULL) {
		cout << "root=NULL\n";
	}
	else {
		root->match(NULL);
		cout << "\n";
	}
	exit(EXIT_SUCCESS);
#endif
}

