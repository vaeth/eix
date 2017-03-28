// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>

#ifndef SRC_EIXTK_EIXARRAY_H_
#define SRC_EIXTK_EIXARRAY_H_ 1

#include <config.h>

#include <sys/types.h>

#ifdef HAVE_ARRAY_CLASS
#include <array>
#endif

#include "eixTk/dialect.h"

namespace eix {

#ifdef HAVE_ARRAY_CLASS

using std::array;

#else  // !defined(HAVE_ARRAY_CLASS)

template<class T, size_t N> class array {
	private:
		T a[N];

	public:
		typedef T value_type;
		typedef size_t size_type;
		typedef const T& const_reference;
		typedef T& reference;

		void fill(const T& value) {
			for (size_type i(0); i < N; ++i) {
				a[i] = value;
			}
		}

		reference operator[](size_type i) {
			return a[i];
		}

		CONSTEXPR const_reference operator[](size_type i) const {
			return a[i];
		}

		reference back() {
			return a[N - 1];
		}

		CONSTEXPR const_reference back() const {
			return a[N - 1];
		}

		reference front() {
			return a[0];
		}

		CONSTEXPR const_reference front() const {
			return a[0];
		}

		value_type *data() {
			return a;
		}

		CONSTEXPR const value_type *data() const {
			return a;
		}

		CONSTEXPR size_type size() const {
			return N;
		}

		CONSTEXPR size_type max_size() const {
			return N;
		}
};
#endif  // !defined(HAVE_ARRAY_CLASS)

}  // namespace eix

#endif  // SRC_EIXTK_EIXARRAY_H_
