// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef EIX__MATCHTREE_H__
#define EIX__MATCHTREE_H__ 1

#include <stack>

#include <cstddef>

class MatchAtomOperator;
class MatchAtomTest;
class MatchTree;
class PackageReader;
class PackageTest;

class MatchAtom {
	friend class MatchTree;
protected:
	bool m_negate;
public:
	MatchAtom(bool negate = false) : m_negate(negate)
	{ }

	/// Virtual deconstructor
	virtual ~MatchAtom()
	{ }

	/** Check (recursively if necessary) whether the atom matches.
	 * @param p Package to match
	 * @return true if match; else false */
	virtual bool match(PackageReader *p);

	virtual MatchAtomOperator *as_operator()
	{ return NULL; }

	virtual MatchAtomTest *as_test()
	{ return NULL; }
};

class MatchAtomOperator : public MatchAtom {
	friend class MatchTree;
private:
	enum AtomOperator { AtomAnd, AtomOr };
	AtomOperator m_operator;
	MatchAtom *m_left, *m_right;
public:
	MatchAtomOperator(AtomOperator op) :
		m_operator(op), m_left(NULL), m_right(NULL)
	{ }

	~MatchAtomOperator();

	bool match(PackageReader *p);

	MatchAtomOperator *as_operator()
	{ return this; }
};

class MatchAtomTest : public MatchAtom {
	friend class MatchTree;
private:
	PackageTest *m_test;
	MatchAtom **m_pipe;
public:
	MatchAtomTest() : m_test(NULL), m_pipe(NULL)
	{ }

	~MatchAtomTest();

	bool match(PackageReader *p);

	void set_test(PackageTest *gtest);

	MatchAtomTest *as_test()
	{ return this; }
};

class MatchParseData {
public:
	MatchAtom **parent; /// parent of the current subroot. Always Non-NULL.
	                    /// Modified only when the current tree is finished.
	MatchAtom *subroot; /// root of the current tree, possibly NULL.
	bool useright;      /// subroot is an operator and grow right leaf.
	bool negatebrace;   /// Must the whole result be negated at the end?
	                    /// This is only set after -! -(

	MatchParseData(MatchAtom **p) : parent(p),
		subroot(NULL), useright(false), negatebrace(false)
	{ }

};

class MatchTree {
private:
	MatchAtom *root, *piperoot;
	MatchAtomOperator::AtomOperator default_operator;

	// The following flags must be carefully honoured and updated
	// in every public parse_* function
	// (the private function sometimes ignore these flags):
	bool local_negate;  /// Is currently some -! active?
	bool local_finished;/// Will -( -! or a test cast a binary operator?
	std::stack<MatchParseData> parser_stack;

	MatchAtomTest *parse_new_leaf();

	/// Update parser_stack.top() according to local_negate
	/// Clear local_negate afterwards.
	void parse_local_negate();

	/// Modifies parser_stack.top() according to op.
	/// Ignores local_negate and local_finished.
	void parse_new_operator(MatchAtomOperator::AtomOperator op);

	/// Internal form of parse_close() which can also clear the
	/// first (root) element on parser_stack() and ignores local_negate.
	void parse_closeforce();
public:
	MatchTree(bool default_is_or);

	~MatchTree();

	bool match(PackageReader *p)
	{ return ((root == NULL) || root->match(p)); }

	void set_pipetest(PackageTest *gtest);

	void parse_test(PackageTest *gtest, bool with_pipe);

	void parse_and();

	void parse_or();

	void parse_negate();

	void parse_open();

	void parse_close();

	void end_parse();
};

#endif /* EIX__MATCHTREE_H__ */
