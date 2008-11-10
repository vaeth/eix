// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>

#if !defined(EIX__AUTO_PTR_H__)
#define EIX__AUTO_PTR_H__

namespace eix {

	template<typename _type>
	class auto_list
	{
		public:
			auto_list(_type *p)
				: m_p(p)
			{ }

			~auto_list()
			{
				if(m_p)
					delete[] m_p;
			}

			_type* get() const
			{ return m_p; }

		protected:
			_type* m_p;
	};

}

#endif /* EIX__AUTO_PTR_H__ */
