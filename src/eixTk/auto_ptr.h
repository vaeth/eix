// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef EIX__AUTO_PTR_H__
#define EIX__AUTO_PTR_H__ 1

namespace eix {

	template<typename m_Type>
	class auto_list
	{
		public:
			auto_list(m_Type *p)
				: m_p(p)
			{ }

			~auto_list()
			{
				if(m_p)
					delete[] m_p;
			}

			m_Type* get() const
			{ return m_p; }

		protected:
			m_Type* m_p;
	};

}

#endif /* EIX__AUTO_PTR_H__ */
