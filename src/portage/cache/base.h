// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)                                                         
//   Wolfgang Frisch <xororand@users.sourceforge.net>                    
//   Emil Beinroth <emilbeinroth@gmx.net>                                
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>                     

#ifndef __BASICCACHE_H__
#define __BASICCACHE_H__

#include <eixTk/exceptions.h>
#include <string>
#include <map>
#include <cstdlib>

#define UNUSED(p) ((void)(p))

class Category;
class Package;
class Version;
class PackageTree;
class PortageSettings;

#if 0
// Add package to vector
Package *addPackage(Category &v, const std::string &cat, const std::string &pkg);

// Find Package and return pointer to it.
Package *findPackage(Category &v, const char *pkg);

// Remove and delete Package. */
bool deletePackage(Category &v, const std::string &pkg);
#endif

// Parent class of every cache that eix can use. */
class BasicCache {

	public:
		typedef void (*ErrorCallback)(const char *fmt, ...);

		BasicCache()
		{ portagesettings = NULL; }

		/// Virtual deconstructor
		virtual ~BasicCache()
		{ }

		virtual bool use_prefixport() const
		{ return false; }

		virtual bool use_prefixexec() const
		{ return false; }

		/// Set scheme for this cache
		void setScheme(const char *prefix, const char *prefixport, const char *prefixexec, std::string scheme);

		/// Set overlay-key
		void setKey(short key)
		{ m_overlay_key = key; }

		// Get scheme for this cache
		std::string getPath() const
		{ return m_scheme; }

		// Get scheme with path for this cache
		std::string getPrefixedPath() const;

		// Get scheme with path(s) for this cache
		std::string getPathHumanReadable() const;

		/// Set callback function to be used in case of errors
		void setErrorCallback(ErrorCallback error_callback)
		{ m_error_callback = error_callback; }

		// @return name of Cache (formatted for good printing)
		virtual const char *getType() const = 0;

		/// Can the method even read multiple categories at once?
		virtual bool can_read_multiple_categories() const
		{ return false; }

		/// If available, the function to read multiple categories.
		/** Note that "categories" might possibly grow if categories should be added.
		    @param packagetree must point to packagetree (if category is not null)
		    @param categories must point to list of categories (if category is not null)
		    @param category if non-null, the function is identical to readCategory
		    @return false if an error occurred so fatal that further calls
		    with this scheme (even with other categories) are useless. */
		virtual bool readCategories(PackageTree *packagetree, std::vector<std::string> *categories, Category *category = NULL) throw(ExBasic)
		{ UNUSED(packagetree); UNUSED(categories); UNUSED(category); return 1; }

		/// Read Cache for a category
		/** @return false if an error occurred so fatal that further calls
		    with this scheme (even with other categories) are useless. */
		virtual bool readCategory(Category &vec) throw(ExBasic)
		{ return readCategories(NULL, NULL, &vec); }

	protected:
		std::string m_scheme, m_prefix, m_prefix_exec;
		bool have_prefix, have_prefix_exec;
		short  m_overlay_key;
		ErrorCallback m_error_callback;
		void env_add_package(std::map<std::string,std::string> &env, const Package &package, const Version &version, const std::string &ebuild_dir, const char *ebuild_full) const;

	public:
		PortageSettings *portagesettings;
};

#endif /* __BASICCACHE_H__ */
