// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>

#ifndef SRC_SEARCH_MATCHTREE_H_
#define SRC_SEARCH_MATCHTREE_H_ 1

#include <stack>

#include "eixTk/dialect.h"
#include "eixTk/null.h"

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
		MatchAtom() : m_negate(false) {
		}

		explicit MatchAtom(bool negate) : m_negate(negate) {
		}

		/**
		Virtual deconstructor
		**/
		virtual ~MatchAtom() {
		}

		/**
		Check (recursively if necessary) whether the atom matches.
		@param p Package to match
		@return true if match; else false
		**/
		virtual bool match(PackageReader *p) ATTRIBUTE_PURE;

		virtual MatchAtomOperator *as_operator() {
			return NULLPTR;
		}

		virtual MatchAtomTest *as_test() {
			return NULLPTR;
		}
};

class MatchAtomOperator : public MatchAtom {
		friend class MatchTree;
	private:
		enum AtomOperator { AtomAnd, AtomOr };
		AtomOperator m_operator;
		MatchAtom *m_left, *m_right;

	public:
		explicit MatchAtomOperator(AtomOperator op)
			: m_operator(op), m_left(NULLPTR), m_right(NULLPTR) {
		}

		~MatchAtomOperator();

		bool match(PackageReader *p) OVERRIDE;

		MatchAtomOperator *as_operator() OVERRIDE {
			return this;
		}
};

class MatchAtomTest : public MatchAtom {
		friend class MatchTree;
	private:
		PackageTest *m_test;
		MatchAtom **m_pipe;

	public:
		MatchAtomTest() : m_test(NULLPTR), m_pipe(NULLPTR) {
		}

		~MatchAtomTest();

		bool match(PackageReader *p) OVERRIDE;

		void set_test(PackageTest *gtest);

		MatchAtomTest *as_test() OVERRIDE {
			return this;
		}
};

class MatchParseData {
	public:
		/**
		parent of the current subroot. Always Non-NULLPTR.
		Modified only when the current tree is finished.
		**/
		MatchAtom **parent;

		/**
		root of the current tree, possibly NULLPTR
		**/
		MatchAtom *subroot;

		/**
		subroot is an operator and grow right leaf
		**/
		bool useright;

		/**
		Must the whole result be negated at the end?
		This is only set after -! -(
		**/
		bool negatebrace;

		explicit MatchParseData(MatchAtom **p) ATTRIBUTE_NONNULL((2))
			: parent(p), subroot(NULLPTR), useright(false), negatebrace(false) {
		}
};

class MatchTree {
	private:
		MatchAtom *root, *piperoot;
		MatchAtomOperator::AtomOperator default_operator;

		/**
		The following flags must be carefully honoured and updated
		in every public parse_* function
		(the private function sometimes ignore these flags):
		**/

		/**
		Is currently some -! active?
		**/
		bool local_negate;

		/**
		Will -( -! or a test cast a binary operator?
		**/
		bool local_finished;
		std::stack<MatchParseData> parser_stack;

		MatchAtomTest *parse_new_leaf();

		/**
		Update parser_stack.top() according to local_negate.
		Clear local_negate afterwards.
		**/
		void parse_local_negate();

		/**
		Modify parser_stack.top() according to op.
		Ignores local_negate and local_finished.
		**/
		void parse_new_operator(MatchAtomOperator::AtomOperator op);

		/**
		Internal form of parse_close() which can also clear the
		first (root) element on parser_stack() and ignores local_negate.
		**/
		void parse_closeforce();

	public:
		explicit MatchTree(bool default_is_or);

		~MatchTree();

		bool match(PackageReader *p);

		void set_pipetest(PackageTest *gtest);

		void parse_test(PackageTest *gtest, bool with_pipe);

		void parse_and();

		void parse_or();

		void parse_negate();

		void parse_open();

		void parse_close();

		void end_parse();
};

#endif  // SRC_SEARCH_MATCHTREE_H_
