// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef SRC_PORTAGE_VERSION_H_
#define SRC_PORTAGE_VERSION_H_ 1

#include <algorithm>
#include <set>
#include <string>
#include <vector>

#include "eixTk/constexpr.h"
#include "eixTk/eixint.h"
#include "eixTk/stringlist.h"
#include "eixTk/stringtypes.h"
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
		static CONSTEXPR Flags
			USEFLAGS_NIL    = 0,
			USEFLAGS_NORMAL = 1,
			USEFLAGS_PLUS   = 2,
			USEFLAGS_MINUS  = 4;
		Flags flags;

		static Flags parse(std::string *s) ATTRIBUTE_NONNULL_;

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

		const char *prefix() const ATTRIBUTE_PURE;

		std::string asString() const;

		bool operator=(const IUse& c) const {
			return (name() == c.name());
		}
};

class IUseSet {
	public:
		typedef std::set<IUse> IUseStd;

		bool empty() const {
			return m_iuse.empty();
		}

		void clear() {
			m_iuse.clear();
		}

		const IUseStd& asStd() const {
			return m_iuse;
		}

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

/** Version expands the BasicVersion class by data relevant for versions in tree/overlays.
 */
class Version : public ExtendedVersion, public Keywords {
	public:
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
		static CONSTEXPR EffectiveState
			EFFECTIVE_UNSAVED = 0,
			EFFECTIVE_USED    = 1,
			EFFECTIVE_UNUSED  = 2;

		std::vector<KeywordsFlags> saved_keywords;
		std::vector<bool>          have_saved_keywords;
		std::vector<MaskFlags>     saved_masks;
		std::vector<bool>          have_saved_masks;
		std::vector<std::string>   saved_effective;
		std::vector<std::string>   saved_accepted;
		std::vector<EffectiveState> states_effective;

		typedef std::vector<SetsIndex> SetsIndizes;
		SetsIndizes sets_indizes;

		std::string m_accepted_keywords;

		IUseSet iuse;

		Version() :
			saved_keywords(SAVEKEY_SIZE, KeywordsFlags()),
			have_saved_keywords(SAVEKEY_SIZE, false),
			saved_masks(SAVEMASK_SIZE, MaskFlags()),
			have_saved_masks(SAVEMASK_SIZE, false),
			saved_effective(SAVEEFFECTIVE_SIZE, ""),
			saved_accepted(SAVEEFFECTIVE_SIZE, ""),
			states_effective(SAVEEFFECTIVE_SIZE, EFFECTIVE_UNSAVED),
			effective_state(EFFECTIVE_UNUSED) {
		}

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
				sets_indizes.push_back(m_set);
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

		/** Calls must be initialized with reset_effective_keywords().
		    Call save_effective_keywords only after the last modify command! */
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

		void reasons_string(OutputString *s, const OutputString& skip, const OutputString& sep) const ATTRIBUTE_NONNULL_;

		bool have_reasons() const ATTRIBUTE_NONNULL_ {
			return !reasons.empty();
		}

	protected:
		typedef std::set<StringList> Reasons;
		Reasons reasons;
		std::string full_keywords, effective_keywords;
		EffectiveState effective_state;
};

#endif  // SRC_PORTAGE_VERSION_H_
