// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#ifndef SRC_PORTAGE_CONF_PORTAGESETTINGS_H_
#define SRC_PORTAGE_CONF_PORTAGESETTINGS_H_ 1

#include <config.h>  // IWYU pragma: keep

#include <string>
#include <vector>

#include "eixTk/attribute.h"
#include "eixTk/null.h"
#include "eixTk/stringtypes.h"
#include "portage/keywords.h"
#include "portage/mask.h"
#include "portage/mask_list.h"
#include "portage/overlay.h"
#include "portage/package.h"

class CascadingProfile;
class EixRc;
class ParseError;
class Version;

// Files for categories the user defined and categories from the official tree
#define MAKE_GLOBALS_FILE       "/etc/make.globals"
#define MAKE_CONF_FILE          "/etc/make.conf"
#define MAKE_CONF_FILE_NEW      "/etc/portage/make.conf"
#define USER_CATEGORIES_FILE    "/etc/portage/categories"
#define USER_KEYWORDS_FILE1     "/etc/portage/package.keywords"
#define USER_KEYWORDS_FILE2     "/etc/portage/package.accept_keywords"
#define USER_MASK_FILE          "/etc/portage/package.mask"
#define USER_UNMASK_FILE        "/etc/portage/package.unmask"
#define USER_USE_FILE           "/etc/portage/package.use"
#define USER_ENV_FILE           "/etc/portage/package.env"
#define USER_LICENSE_FILE       "/etc/portage/package.license"
#define USER_RESTRICT_FILE      "/etc/portage/package.accept_restrict"
#define USER_CFLAGS_FILE        "/etc/portage/package.cflags"
#define USER_REPOS_CONF         "/etc/portage/repos.conf"
#define USER_PROFILE_DIR        "/etc/portage/profile"
#define PORTDIR_CATEGORIES_FILE "profiles/categories"
#define PORTDIR_MASK_FILE       "profiles/package.mask"
#define PORTDIR_UNMASK_FILE     "profiles/package.unmask"
#define PORTDIR_MAKE_DEFAULTS   "profiles/make.defaults"

class PortageSettings;

class PortageUserConfig {
		friend class PortageSettings;
	private:
		PortageSettings      *m_settings;
		MaskList<Mask>        m_localmasks;
		MaskList<KeywordMask> m_accept_keywords;
		MaskList<KeywordMask> m_use, m_env, m_license, m_restrict, m_cflags;
		bool read_use, read_env, read_license, read_restrict, read_cflags;

		/**
		Your cascading profile, including local settings
		**/
		CascadingProfile     *profile;

		/**
		@return true if something was added
		**/
		bool readMasks();
		bool readKeywords();

		ATTRIBUTE_NONNULL_ bool CheckList(Package *p, const MaskList<KeywordMask> *list, Keywords::Redundant flag_double, Keywords::Redundant flag_in) const;
		ATTRIBUTE_NONNULL_ bool CheckFile(Package *p, const char *file, MaskList<KeywordMask> *list, bool *readfile, Keywords::Redundant flag_double, Keywords::Redundant flag_in) const;
		ATTRIBUTE_NONNULL_ void ReadVersionFile(const char *file, MaskList<KeywordMask> *list) const;

		ATTRIBUTE_NONNULL_ void pushback_set_accepted_keywords(WordVec *result, const Version *v) const;

	public:
		ATTRIBUTE_NONNULL((2)) PortageUserConfig(PortageSettings *psettings, CascadingProfile *local_profile);

		~PortageUserConfig();

		ATTRIBUTE_NONNULL_ void setProfileMasks(Package *p) const;

		/**
		@return true if something from /etc/portage/package.* applied and check involves masks
		**/
		ATTRIBUTE_NONNULL_ bool setMasks(Package *p, Keywords::Redundant check, bool file_mask_is_profile) const;
		ATTRIBUTE_NONNULL_ bool setMasks(Package *p, Keywords::Redundant check) const {
			return setMasks(p, check, false);
		}
		ATTRIBUTE_NONNULL_ bool setMasks(Package *p) const {
			return setMasks(p, Keywords::RED_NOTHING);
		}

		/**
		@return true if something from /etc/portage/package.* applied and check involves keywords
		**/
		ATTRIBUTE_NONNULL_ bool setKeyflags(Package *p, Keywords::Redundant check) const;
		ATTRIBUTE_NONNULL_ bool setKeyflags(Package *p) const {
			return setKeyflags(p, Keywords::RED_NOTHING);
		}

		/**
		@return true if something from /etc/portage/package.use applied
		**/
		ATTRIBUTE_NONNULL_ bool CheckUse(Package *p, Keywords::Redundant check) {
			if(check & Keywords::RED_ALL_USE) {
				return CheckFile(p, USER_USE_FILE, &m_use, &read_use, check & Keywords::RED_DOUBLE_USE, check & Keywords::RED_IN_USE);
			}
			return false;
		}
		/**
		@return true if something from /etc/portage/package.env applied
		**/
		ATTRIBUTE_NONNULL_ bool CheckEnv(Package *p, Keywords::Redundant check) {
			if(check & Keywords::RED_ALL_ENV) {
				return CheckFile(p, USER_ENV_FILE, &m_env, &read_env, check & Keywords::RED_DOUBLE_ENV, check & Keywords::RED_IN_ENV);
			}
			return false;
		}
		/**
		@return true if something from /etc/portage/package.license applied
		**/
		ATTRIBUTE_NONNULL_ bool CheckLicense(Package *p, Keywords::Redundant check) {
			if(check & Keywords::RED_ALL_LICENSE) {
				return CheckFile(p, USER_LICENSE_FILE, &m_license, &read_license, check & Keywords::RED_DOUBLE_LICENSE, check & Keywords::RED_IN_LICENSE);
			}
			return false;
		}
		/**
		@return true if something from /etc/portage/package.accept_restrict applied
		**/
		ATTRIBUTE_NONNULL_ bool CheckAcceptRestrict(Package *p, Keywords::Redundant check) {
			if(check & Keywords::RED_ALL_RESTRICT) {
				return CheckFile(p, USER_RESTRICT_FILE, &m_restrict, &read_restrict, check & Keywords::RED_DOUBLE_RESTRICT, check & Keywords::RED_IN_RESTRICT);
			}
			return false;
		}
		/**
		@return true if something from /etc/portage/package.cflags applied
		**/
		ATTRIBUTE_NONNULL_ bool CheckCflags(Package *p, Keywords::Redundant check) {
			if(check & Keywords::RED_ALL_CFLAGS) {
				return CheckFile(p, USER_CFLAGS_FILE, &m_cflags, &read_cflags, check & Keywords::RED_DOUBLE_CFLAGS, check & Keywords::RED_IN_CFLAGS);
			}
			return false;
		}
};

/**
Holds Portage's settings, e.g. masks, categories, overlay paths.
Reads needed files if content is requested .. so don't worry about initialization :)
**/
class PortageSettings : public WordIterateMap {
	private:
		friend class CascadingProfile;
		friend class PortageUserConfig;

		EixRc *settings_rc;
		const ParseError *parse_error;
		/**
		Vector of all allowed categories
		**/
		WordVec                  m_categories;

		WordVec                  m_accepted_keywords;
		WordSet                  m_accepted_keywords_set, m_arch_set,
		                         m_plain_accepted_keywords_set,
		                        *m_local_arch_set, *m_auto_arch_set;
		std::string              m_raised_arch;

		MaskList<SetMask>        m_package_sets;

		typedef std::vector<SetsList> InheritSets;
		InheritSets parent_sets, children_sets;

		/**
		One may argue whether reading the settings for the upgrade policy
		is only a cache, but it makes things simpler if we say so
		**/
		mutable bool know_upgrade_policy, upgrade_policy;
		mutable MaskList<Mask> upgrade_policy_exceptions;

		mutable bool know_expands;
		mutable WordIterateMap expand_vars;

		bool know_world_sets;
		WordVec world_sets;
		bool world_setslist_up_to_date;
		SetsList world_setslist;

		/**
		Your cascading profile, excluding local settings
		**/
		CascadingProfile  *profile;

		ATTRIBUTE_NONNULL_ void override_by_map(const char *const *vars, const WordIterateMap& varmap);
		ATTRIBUTE_NONNULL_ void override_by_map(const WordIterateMap& varmap);
		ATTRIBUTE_NONNULL_ void override_by_env(const char *const *vars);
		ATTRIBUTE_NONNULL_ void override_by_value(const char *var, const std::string& value);
		ATTRIBUTE_NONNULL((2)) void override_by_value(const char *var, const char *value);
		ATTRIBUTE_NONNULL_ void read_config(const std::string& name, const std::string& prefix, WordIterateMap *vars);
		void read_config(const std::string& name, const std::string& prefix) {
			read_config(name, prefix, this);
		}
		ATTRIBUTE_NONNULL_ void read_make_conf(const std::string& eprefixsource, WordIterateMap *vars);
		void read_make_conf_early(const std::string& eprefixsource);
		void read_make_globals(const std::string& eprefixsource);
		void read_repos_conf(const std::string& eprefixsource);

		ATTRIBUTE_NONNULL_ void addOverlayProfiles(CascadingProfile *p) const;

		ATTRIBUTE_NONNULL_ void calc_recursive_sets(Package *p) const;

		ATTRIBUTE_NONNULL_ void read_world_sets(const char *file);
		ATTRIBUTE_NONNULL_ void calc_world_sets(Package *p);

		void update_world_setslist();

		bool grab_setmasks(const char *file, SetsIndex i, WordVec *contains_set, bool recursive);
		bool grab_setmasks(const char *file, SetsIndex i, WordVec *contains_set) {
			return grab_setmasks(file, i, contains_set, false);
		}

		const MaskList<SetMask> *getPackageSets() const {
			return &m_package_sets;
		}

		WordVec::size_type verify_categories(const std::string &name, WordVec *vec, WordVec::size_type index) const;

	public:
		bool m_recurse_sets;
		std::string m_eprefix;
		std::string m_eprefixconf;
		std::string m_eprefixprofile;
		std::string m_eprefixportdir;
		std::string m_eprefixoverlays;
		std::string m_eprefixaccessoverlays;
		std::string m_world;

		PortageUserConfig *user_config;

		RepoList repos;
		WordVec set_names;

#ifndef HAVE_SETENV
		bool export_portdir_overlay;
#endif
		using WordIterateMap::find;
		using WordIterateMap::erase;
		using WordIterateMap::begin;
		using WordIterateMap::end;
		using WordIterateMap::operator[];

		ATTRIBUTE_PURE const std::string& operator[](const std::string& var) const;

		ATTRIBUTE_PURE const char *cstr(const std::string& var) const;

		/**
		Read make.globals and make.conf
		**/
		ATTRIBUTE_NONNULL_ void init(EixRc *eixrc, const ParseError *e, bool getlocal, bool init_world, bool print_profile_paths);

		ATTRIBUTE_NONNULL_ PortageSettings(EixRc *eixrc, const ParseError *e, bool getlocal, bool init_world, bool print_profile_paths) {
			init(eixrc, e, getlocal, init_world, print_profile_paths);
		}

		ATTRIBUTE_NONNULL_ PortageSettings(EixRc *eixrc, const ParseError *e, bool getlocal, bool init_world) {
			init(eixrc, e, getlocal, init_world, false);
		}

		/**
		Free memory
		**/
		~PortageSettings();

		std::string resolve_overlay_name(const std::string& path, bool resolve);

		void add_repo(const std::string& path, bool resolve, const char *label, OverlayIdent::Priority priority, bool is_main);
		void add_repo(const std::string& path, bool resolve) {
			add_repo(path, resolve, NULLPTR, 0, false);
		}

		void add_repo_vector(const WordVec& v, bool resolve);

		void store_world_sets(const WordVec *s_world_sets, bool override);
		void store_world_sets(const WordVec *s_world_sets) {
			store_world_sets(s_world_sets, false);
		}

		ATTRIBUTE_NONNULL_ void get_setnames(WordSet *names, const Package *p, bool also_nonlocal) const;
		ATTRIBUTE_NONNULL_ void get_setnames(WordSet *names, const Package *p) const {
			get_setnames(names, p, false);
		}

		std::string get_setnames(const Package *p, bool also_nonlocal) const;
		std::string get_setnames(const Package *p) const {
			return get_setnames(p, false);
		}

		void read_local_sets(const WordVec& dir_list);

		const WordVec *get_world_sets() const {
			return &world_sets;
		}

		/**
		pushback categories from profiles to vec. Categories may be duplicate.
		Result is not cashed, i.e. this should be called only once.
		**/
		ATTRIBUTE_NONNULL_ void pushback_categories(WordVec *vec);

		ATTRIBUTE_NONNULL_ void setMasks(Package *p, bool filemask_is_profile) const;
		ATTRIBUTE_NONNULL_ void setMasks(Package *p) const {
			setMasks(p, false);
		}

		/**
		Set stability according to arch or local ACCEPTED_KEYWORDS
		**/
		ATTRIBUTE_NONNULL_ void setKeyflags(Package *pkg, bool use_accepted_keywords) const;

		ATTRIBUTE_NONNULL_ void add_name(SetsList *l, const std::string& s, bool recurse) const;

		ATTRIBUTE_NONNULL_ bool calc_allow_upgrade_slots(const Package *p) const;

		ATTRIBUTE_NONNULL_ void calc_local_sets(Package *p) const;

		ATTRIBUTE_NONNULL_ void finalize(Package *p) {
			calc_world_sets(p);
			p->finalize_masks();
		}

		ATTRIBUTE_NONNULL_ void get_effective_keywords_profile(Package *p) const;

		ATTRIBUTE_NONNULL_ void get_effective_keywords_userprofile(Package *p) const;

		ATTRIBUTE_NONNULL_ bool use_expand(std::string *var, std::string *expvar, const std::string& value) const;

		static void init_static();
};

#endif  // SRC_PORTAGE_CONF_PORTAGESETTINGS_H_
