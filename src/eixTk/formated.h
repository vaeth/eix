// vim:set et sw=4 sts=8 fdm=syntax:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Emil Beinroth <emilbeinroth@gmx.net>

#ifndef __GUARD__FORMATED_H__
#define __GUARD__FORMATED_H__

#include <string>
#include <sstream>
#include <iostream>

namespace eix
{
  //! printf-like, typesafe text formating that replaces tokens in the given
  //! string with textual representations of values.
  //
  //   - %r enclose strings in single quotes, all other types are formated 
  class format
    {
  public:
      //! Set @c format as the template string.
      explicit format(const std::string &format)
          : sstream_(), format_(format)
        { goto_next_spec(); }

      //! Copy @b current state into new format.
      explicit format(const format &e)
          : sstream_(e.sstream_.str()), format_(e.format_)
        { }

      //! Insert the value for the next placeholder.
      template<typename T>
          format &operator % (const T& s);

      //! Return the formatted string.
      std::string str() const
        { return sstream_.str(); }

      //! Convenience wrapper for @c str().
      operator std::string () 
        { return str(); }

      //! Write formated string to ostream @c os.
      friend std::ostream& operator<< (std::ostream& os, const format& formater)
        { return os << formater.str(); }

  protected:
      std::ostringstream sstream_;
      std::string format_;

      char spec;

      void goto_next_spec();

      std::ostream& write_repr(std::ostream& s, const char *t)
        { return s << "'" << t << "'"; }

      std::ostream& write_repr(std::ostream& s, const std::string &t)
        { return s << "'" << t << "'"; }

      template<typename T>
      std::ostream &write_repr(std::ostream& s, const T& t)
        { return s << t; }
    };

  template<typename T>
  inline format &format::operator % (const T& s)
    {
      if (spec == 'r')
          write_repr(sstream_, s);
      else
          sstream_ << s;
      goto_next_spec();
      return *this;
    }

  inline void format::goto_next_spec()
    {
      spec = 0;
      std::string::size_type next = format_.find('%');
      if (next == std::string::npos || format_.size() < next+2) {
          sstream_ << format_;
          format_.clear();
      }
      else if (format_.at(next+1) == '%') {
          // %% gives a single %
          sstream_ << format_.substr(0, next+1);
          format_.erase(0, next+2);
          goto_next_spec();
      }
      else {
          // remember the specifier so we can use it in the next call to the
          // %-operator.
          spec = format_.at(next+1);
          sstream_ << format_.substr(0, next);
          format_.erase(0, next+2);
      }
    }
}
#endif /* __GUARD__FORMATED_H__ */
