// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#ifndef SRC_PORTAGE_VERSION_H_
#define SRC_PORTAGE_VERSION_H_ 1

#include <config.h>  // IWYU pragma: keep

#include <algorithm>
#include <set>
#include <string>
#include <vector>

#include "eixTk/attribute.h"
#include "eixTk/dialect.h"
#include "eixTk/eixarray.h"
#include "eixTk/eixint.h"
#include "eixTk/stringlist.h"
#include "eixTk/stringtypes.h"
#include "eixTk/stringutils.h"
#include "portage/basicversion.h"
#include "portage/extendedversion.h"
#include "portage/keywords.h"
#include "portage/packagesets.h"

class Database;
class DBHeader;
class OutputString;

class IUse : public std::string {
	public:
		typedef eix::UChar Flags;
		static CONSTEXPR const Flags
			USEFLAGS_NIL    = 0,
			USEFLAGS_NORMAL = 1,
			USEFLAGS_PLUS   = 2,
			USEFLAGS_MINUS  = 4;
		Flags flags;

		ATTRIBUTE_NONNULL_ static Flags parse(std::string *s);

		std::string& name() {
			return *static_cast<std::string *>(this);
		}

		const std::string& name() const {
			return *static_cast<const std::string *>(this);
		}

		explicit IUse(const std::string& s) : std::string(s) {
			flags = parse(&name());
		}

		IUse(const std::string& s, Flags f) : std::string(s), flags(f) {
		}

		ATTRIBUTE_PURE const char *prefix() const;

		std::string asString() const;

		bool operator==(const IUse& c) const {
			return (name() == c.name());
		}
		bool operator!=(const IUse& c) const {
			return (name() != c.name());
		}
		bool operator<(const IUse& c) const {
			return (name() < c.name());
		}
		bool operator>(const IUse& c) const {
			return (name() > c.name());
		}
		bool operator<=(const IUse& c) const {
			return (name() <= c.name());
		}
		bool operator>=(const IUse& c) const {
			return (name() >= c.name());
		}
};

class IUseNatural {
	public:
		IUseNatural(const IUse *use) {
			m_iuse = use;
		}

		const IUse& iuse() const {
			return *m_iuse;
		}

		const std::string& name() const {
			return m_iuse->name();
		}

		std::string asString() const {
			return m_iuse->asString();
		}

		bool operator==(const IUseNatural& c) const {
			return (name() == c.name());
		}
		bool operator!=(const IUseNatural& c) const {
			return (name() != c.name());
		}
		bool operator<(const IUseNatural& c) const {
			return (natcmp(name(), c.name()) < 0);
		}
		bool operator>(const IUseNatural& c) const {
			return (natcmp(name(), c.name()) > 0);
		}
		bool operator<=(const IUseNatural& c) const {
			return (natcmp(name(), c.name()) <= 0);
		}
		bool operator>=(const IUseNatural& c) const {
			return (natcmp(name(), c.name()) >= 0);
		}

	private:
		const IUse *m_iuse;
};

class IUseSet {
	public:
		typedef std::set<IUse> IUseStd;
		typedef std::set<IUseNatural> IUseNaturalOrder;

		bool empty() const {
			return m_iuse.empty();
		}

		void clear() {
			m_iuse.clear();
		}

		const IUseStd& asStd() const {
			return m_iuse;
		}

		IUseNaturalOrder asNaturalOrder() const;

		void insert(const IUseStd& iuse);

		void insert(const IUseSet& iuse) {
			insert(iuse.asStd());
		}

		void insert(const std::string& iuse);

		void insert_fast(const std::string& iuse) {
			insert(IUse(iuse));
		}

		std::string asString() const;

		WordVec asVector() const;

	protected:
		IUseStd m_iuse;

		void insert(const IUse& iuse);
};

/**
Version expands the BasicVersion class by data relevant for versions in tree/overlays.
**/
class Version FINAL : public ExtendedVersion, public Keywords {
	public:
		typedef std::set<StringList> Reasons;

		friend class Database;

		typedef enum {
			SAVEKEY_USER, SAVEKEY_ACCEPT, SAVEKEY_ARCH, SAVEKEY_SIZE
		} SavedKeyIndex;

		typedef enum {
			SAVEMASK_USER, SAVEMASK_USERFILE, SAVEMASK_USERPROFILE,
			SAVEMASK_PROFILE, SAVEMASK_FILE, SAVEMASK_SIZE
		} SavedMaskIndex;

		typedef enum {
			SAVEEFFECTIVE_USERPROFILE, SAVEEFFECTIVE_PROFILE, SAVEEFFECTIVE_SIZE
		} SavedEffectiveIndex;

		typedef eix::UChar EffectiveState;
		static CONSTEXPR const EffectiveState
			EFFECTIVE_UNSAVED = 0,
			EFFECTIVE_USED    = 1,
			EFFECTIVE_UNUSED  = 2;

		eix::array<KeywordsFlags, SAVEKEY_SIZE>      saved_keywords;
		eix::array<bool, SAVEKEY_SIZE>               have_saved_keywords;
		eix::array<MaskFlags, SAVEMASK_SIZE>         saved_masks;
		eix::array<bool, SAVEMASK_SIZE>              have_saved_masks;
		eix::array<std::string, SAVEEFFECTIVE_SIZE>  saved_effective;
		eix::array<std::string, SAVEEFFECTIVE_SIZE>  saved_accepted;
		eix::array<EffectiveState, EFFECTIVE_UNUSED> states_effective;

		typedef std::vector<SetsIndex> SetsIndizes;
		SetsIndizes sets_indizes;

		std::string m_accepted_keywords;

		IUseSet iuse;

		static bool use_required_use;

		std::string required_use;

		Version();

		void save_keyflags(SavedKeyIndex i) {
			have_saved_keywords[i] = true;
			saved_keywords[i] = keyflags;
		}

		void save_maskflags(SavedMaskIndex i) {
			have_saved_masks[i] = true;
			saved_masks[i] = maskflags;
		}

		void save_accepted_effective(SavedEffectiveIndex i) {
			saved_accepted[i] = m_accepted_keywords;
			if((states_effective[i] = effective_state) == EFFECTIVE_USED) {
				saved_effective[i] = effective_keywords;
			}
		}

		bool restore_keyflags(SavedKeyIndex i) {
			if(have_saved_keywords[i]) {
				keyflags = saved_keywords[i];
				return true;
			}
			return false;
		}

		bool restore_maskflags(SavedMaskIndex i) {
			if(have_saved_masks[i]) {
				maskflags = saved_masks[i];
				return true;
			}
			return false;
		}

		void set_iuse(const std::string& s) {
			iuse.clear();
			iuse.insert(s);
		}

		void set_required_use(const char *s) {
			if(use_required_use) {
				required_use.assign(s);
			}
		}

		void set_required_use(const std::string& s) {
			if(use_required_use) {
				required_use.assign(s);
			}
		}

		bool restore_accepted_effective(SavedEffectiveIndex i) {
			EffectiveState s(states_effective[i]);
			if(s == EFFECTIVE_UNSAVED) {
				return false;
			}
			m_accepted_keywords = saved_accepted[i];
			if((effective_state = s) == EFFECTIVE_USED) {
				effective_keywords = saved_effective[i];
			} else {
				effective_keywords.clear();
			}
			return true;
		}

		bool is_in_set(SetsIndex m_set) const {
			return (std::find(sets_indizes.begin(), sets_indizes.end(), m_set) != sets_indizes.end());
		}

		void add_to_set(SetsIndex m_set) {
			if(!is_in_set(m_set)) {
				sets_indizes.PUSH_BACK(MOVE(m_set));
			}
		}

		void set_full_keywords(const std::string& keywords) {
			full_keywords = keywords;
		}

		std::string get_full_keywords() const {
			return full_keywords;
		}

		void reset_accepted_effective_keywords() {
			effective_state = EFFECTIVE_UNUSED;
			m_accepted_keywords.clear();
			effective_keywords.clear();
		}

		/**
		Calls must be initialized with reset_effective_keywords().
		Call save_effective_keywords only after the last modify command!
		**/
		void modify_effective_keywords(const std::string& modify_keys);

		void add_accepted_keywords(const std::string& accepted_keywords);

		const std::string get_effective_keywords() const {
			return ((effective_state == EFFECTIVE_USED) ? effective_keywords : full_keywords);
		}

		KeywordsFlags::KeyType get_keyflags(const WordSet& accepted_keywords) const {
			if(effective_state == EFFECTIVE_USED) {
				return KeywordsFlags::get_keyflags(accepted_keywords, effective_keywords);
			}
			return KeywordsFlags::get_keyflags(accepted_keywords, full_keywords);
		}

		void set_keyflags(const WordSet& accepted_keywords) {
			keyflags.set_keyflags(get_keyflags(accepted_keywords));
		}

		void add_reason(const StringList& reason);

		ATTRIBUTE_NONNULL_ void reasons_string(OutputString *s, const OutputString& skip, const OutputString& sep) const;

		bool have_reasons() const {
			return !reasons.empty();
		}

		const Reasons *reasons_ptr() const {
			return &reasons;
		}

	protected:
		Reasons reasons;
		std::string full_keywords, effective_keywords;
		EffectiveState effective_state;
};

#endif  // SRC_PORTAGE_VERSION_H_
