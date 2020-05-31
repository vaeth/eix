// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>

#include "output/print-proto.h"
#include <config.h>  // IWYU pragma: keep

#ifdef WITH_PROTOBUF
#include <iostream>
#include <set>
#include <string>
#else
#include <cstdlib>
#endif

#ifdef WITH_PROTOBUF
#include "eixTk/dialect.h"
#else
#include "eixTk/formated.h"
#include "eixTk/i18n.h"
#endif

#ifdef WITH_PROTOBUF
#include "eixTk/likely.h"
#include "eixTk/null.h"
#include "eixTk/stringtypes.h"
#include "eixTk/unordered_set.h"
#include "output/eix.pb.h"
#include "output/formatstring.h"
#include "portage/basicversion.h"
#include "portage/depend.h"
#include "portage/extendedversion.h"
#include "portage/instversion.h"
#include "portage/keywords.h"
#include "portage/overlay.h"
#include "portage/package.h"
#include "portage/set_stability.h"
#include "portage/vardbpkg.h"
#include "portage/version.h"

using std::set;
using std::string;

static void set_mask_flags(eix_proto::MaskFlags *mask_flags, MaskFlags mask);
static void set_key_flags(eix_proto::KeyFlags *key_flags, KeywordsFlags key);
static void add_restrictions(eix_proto::Restrictions *restrictions, ExtendedVersion::Restrict restrict);
static void add_properties(eix_proto::Properties *properties, ExtendedVersion::Restrict props);

void PrintProto::start() {
	collection = new eix_proto::Collection();
	category_index.clear();
}

void PrintProto::package(Package *pkg) {
	if(collection == NULLPTR) {
		start();
	}
	eix_proto::Category *category;
	if(collection->category_size() != 0) {
		// Re-use existing category if possible
		int& index = category_index[pkg->category];
		category = collection->mutable_category(index);
		if(index != 0 || category->category() != pkg->category) {
			// category is actually new
			index = collection->category_size();
			category = collection->add_category();
		}
	} else {
		category = collection->add_category();
	}
	eix_proto::Package *package = category->add_package();
	package->set_name(pkg->name);
	package->set_description(pkg->desc);
	package->set_homepage(pkg->homepage);
	package->set_licenses(pkg->licenses);

	UNORDERED_SET<const Version*> have_inst;
	if((likely(var_db_pkg != NULLPTR)) && var_db_pkg->isInstalled(*pkg)) {
		set<BasicVersion> know_inst;  // NOLINT(build/include_what_you_use)
		// First we check which versions are installed with correct overlays.
		if(likely(hdr != NULLPTR)) {
			// Package is a 'list' of Versions with added members ^^
			for(Package::const_iterator ver(pkg->begin());
				likely(ver != pkg->end()); ++ver) {
				if(var_db_pkg->isInstalledVersion(*pkg, *ver, *hdr) > 0) {
					know_inst.INSERT(**ver);
					have_inst.INSERT(*ver);
				}
			}
		}
		// From the remaining ones we choose the last.
		// The following should actually be const_reverse_iterator,
		// but some compilers would then need a cast of rend(),
		// see https://bugs.gentoo.org/show_bug.cgi?id=354071
		for(Package::reverse_iterator ver(pkg->rbegin());
			likely(ver != pkg->rend()); ++ver) {
			if(know_inst.count(**ver) == 0) {
				know_inst.INSERT(**ver);
				have_inst.INSERT(*ver);
			}
		}
	}

	for(Package::const_iterator ver(pkg->begin()); likely(ver != pkg->end()); ++ver) {
		eix_proto::Version *version = package->add_version();
		version->set_id(ver->getFull());
		version->set_eapi(ver->eapi.get());

		eix_proto::Repository *repository = version->mutable_repository();
		ExtendedVersion::Overlay overlay_key(ver->overlay_key);
		if(unlikely(overlay_key != 0)) {
			if(print_format->is_virtual(overlay_key)) {
				repository->set_virtual_(true);
			}
			const OverlayIdent& overlay(hdr->getOverlay(overlay_key));
			if(!(overlay.path.empty())) {
				repository->set_overlay(overlay.path);
			}
			if(!overlay.label.empty()) {
				repository->set_repository(overlay.label);
			}
		}
		if(!ver->get_shortfullslot().empty()) {
			version->set_slot(ver->get_longfullslot());
		}
		if(!ver->src_uri.empty()) {
			version->set_src_uri(ver->src_uri);
		}

		MaskFlags local_mask_flags;
		KeywordsFlags local_key_flags;
		stability->calc_version_flags(true, &local_mask_flags, &local_key_flags, *ver, pkg);
		if (!local_mask_flags.empty()) {
			set_mask_flags(version->mutable_local_mask_flags(), local_mask_flags);
		}
		if (!local_key_flags.empty()) {
			set_key_flags(version->mutable_local_key_flags(), local_key_flags);
		}
		MaskFlags system_mask_flags;
		KeywordsFlags system_key_flags;
		stability->calc_version_flags(false, &system_mask_flags, &system_key_flags, *ver, pkg);
		if (!system_mask_flags.empty()) {
			set_mask_flags(version->mutable_system_mask_flags(), system_mask_flags);
		}
		if (!system_key_flags.empty()) {
			set_key_flags(version->mutable_system_key_flags(), system_key_flags);
		}

		if(unlikely(ver->have_reasons())) {
			const Version::Reasons *reasons_ptr(ver->reasons_ptr());
			for(Version::Reasons::const_iterator it(reasons_ptr->begin());
				unlikely(it != reasons_ptr->end()); ++it) {
				const WordVec *vec(it->asWordVecPtr());
				if((vec == NULLPTR) || (vec->empty())) {
					continue;
				}
				eix_proto::Lines *lines = version->add_mask_reason();
				for(WordVec::const_iterator wit(vec->begin());
					likely(wit != vec->end()); ++wit) {
					lines->add_line(*wit);
				}
			}
		}

		if(!(ver->iuse.empty())) {
			const IUseSet::IUseStd& s(ver->iuse.asStd());
			for(IUseSet::IUseStd::const_iterator it(s.begin()); likely(it != s.end()); ++it) {
				IUse::Flags flags = it->flags;
				if((flags & IUse::USEFLAGS_NORMAL) != 0) {
					version->add_iuse(it->name());
				}
				if((flags & IUse::USEFLAGS_PLUS) != 0) {
					version->add_iuse_plus(it->name());
				}
				if((flags & IUse::USEFLAGS_MINUS) != 0) {
					version->add_iuse_minus (it->name());
				}
			}
		}
		if(Version::use_required_use) {
			string &required_use(ver->required_use);
			if(!(required_use.empty())) {
				version->set_required_use(required_use);
			}
		}

		eix_proto::Installed *installed = NULLPTR;
		InstVersion *installedVersion(NULLPTR);
		if(have_inst.count(*ver) != 0) {
			if(var_db_pkg->isInstalled(*pkg, *ver, &installedVersion)) {
				installed =  version->mutable_installed();
				var_db_pkg->readInstDate(*pkg, installedVersion);
				var_db_pkg->readEapi(*pkg, installedVersion);
			}
		}
		if(installed != NULLPTR) {
			installed->set_date(installedVersion->instDate);
			installed->set_eapi(installedVersion->eapi.get());

			var_db_pkg->readUse(*pkg, installedVersion);
			WordVec inst_iuse(installedVersion->inst_iuse);
			WordSet usedUse(installedVersion->usedUse);
			for(WordVec::const_iterator iu(inst_iuse.begin()); likely(iu != inst_iuse.end()); iu++) {
				if(usedUse.count(*iu) == 0) {
					installed->add_use_enabled(*iu);
				} else {
					installed->add_use_disabled(*iu);
				}
			}
		}

		ExtendedVersion::Restrict restrict(ver->restrictFlags);
		if(unlikely(restrict != ExtendedVersion::RESTRICT_NONE)) {
			add_restrictions(version->mutable_restrictions(), restrict);
		}
		ExtendedVersion::Restrict props(ver->propertiesFlags);
		if(unlikely(props != ExtendedVersion::PROPERTIES_NONE)) {
			add_properties(version->mutable_properties(), props);
		}

		string full_kw = ver->get_full_keywords();
		if(!full_kw.empty()) {
			version->set_keywords(full_kw);
		}
		string eff_kw = ver->get_effective_keywords();
		if(unlikely(full_kw != eff_kw)) {
			version->mutable_keywords_effective()->set_value(eff_kw);
		}

		if(Depend::use_depend) {
			const string& depend = ver->depend.get_depend();
			if(!depend.empty()) {
				version->set_depend(depend);
			}
			const string& rdepend = ver->depend.get_rdepend();
			if(!rdepend.empty()) {
				version->set_rdepend(rdepend);
			}
			const string& pdepend = ver->depend.get_pdepend();
			if(!pdepend.empty()) {
				version->set_pdepend(pdepend);
			}
			const string& bdepend = ver->depend.get_bdepend();
			if(!bdepend.empty()) {
				version->set_bdepend(bdepend);
			}
		}
	}
}  // NOLINT(readability/fn_size)

static void set_mask_flags(eix_proto::MaskFlags *mask_flags, MaskFlags mask) {
	if(mask.isPackageMask()) {
		mask_flags->add_mask_flag(eix_proto::MaskFlags::MASK_PACKAGE);
	}
	if(mask.isSystem()) {
		mask_flags->add_mask_flag(eix_proto::MaskFlags::MASK_SYSTEM);
	}
	if(mask.isProfileMask()) {
		mask_flags->add_mask_flag(eix_proto::MaskFlags::MASK_PROFILE);
	}
	if(mask.isProfile()) {
		mask_flags->add_mask_flag(eix_proto::MaskFlags::IN_PROFILE);
	}
	if(mask.isWorld()) {
		mask_flags->add_mask_flag(eix_proto::MaskFlags::WORLD);
	}
	if(mask.isWorldSets()) {
		mask_flags->add_mask_flag(eix_proto::MaskFlags::WORLD_SETS);
	}
	if(mask.isMarked()) {
		mask_flags->add_mask_flag(eix_proto::MaskFlags::MARKED);
	}
}

static void set_key_flags(eix_proto::KeyFlags *key_flags, KeywordsFlags key) {
	if(key.isStable()) {
		key_flags->add_key_flag(eix_proto::KeyFlags::STABLE);
	}
	if(key.isArchStable()) {
		key_flags->add_key_flag(eix_proto::KeyFlags::ARCHSTABLE);
	}
	if(key.isUnstable()) {
		key_flags->add_key_flag(eix_proto::KeyFlags::ARCHUNSTABLE);
	}
	if(key.isAlienStable()) {
		key_flags->add_key_flag(eix_proto::KeyFlags::ALIENSTABLE);
	}
	if(key.isAlienUnstable()) {
		key_flags->add_key_flag(eix_proto::KeyFlags::ALIENUNSTABLE);
	}
	if(key.isMinusKeyword()) {
		key_flags->add_key_flag(eix_proto::KeyFlags::MINUSKEYWORD);
	}
	if(key.isMinusUnstable()) {
		key_flags->add_key_flag(eix_proto::KeyFlags::MINUSUNSTABLE);
	}
	if(key.isMinusAsterisk()) {
		key_flags->add_key_flag(eix_proto::KeyFlags::MINUSASTERISK);
	}
}

static void add_restrictions(eix_proto::Restrictions *restrictions, ExtendedVersion::Restrict restrict) {
	if(unlikely((restrict & ExtendedVersion::RESTRICT_BINCHECKS) != 0)) {
		restrictions->add_restrict(eix_proto::Restrictions::BINCHECKS);
	}
	if(unlikely((restrict & ExtendedVersion::RESTRICT_STRIP) != 0)) {
		restrictions->add_restrict(eix_proto::Restrictions::STRIP);
	}
	if(unlikely((restrict & ExtendedVersion::RESTRICT_TEST) != 0)) {
		restrictions->add_restrict(eix_proto::Restrictions::TEST);
	}
	if(unlikely((restrict & ExtendedVersion::RESTRICT_USERPRIV) != 0)) {
		restrictions->add_restrict(eix_proto::Restrictions::USERPRIV);
	}
	if(unlikely((restrict & ExtendedVersion::RESTRICT_INSTALLSOURCES) != 0)) {
		restrictions->add_restrict(eix_proto::Restrictions::INSTALLSOURCES);
	}
	if(unlikely((restrict & ExtendedVersion::RESTRICT_FETCH) != 0)) {
		restrictions->add_restrict(eix_proto::Restrictions::FETCH);
	}
	if(unlikely((restrict & ExtendedVersion::RESTRICT_MIRROR) != 0)) {
		restrictions->add_restrict(eix_proto::Restrictions::MIRROR);
	}
	if(unlikely((restrict & ExtendedVersion::RESTRICT_PRIMARYURI) != 0)) {
		restrictions->add_restrict(eix_proto::Restrictions::PRIMARYURI);
	}
	if(unlikely((restrict & ExtendedVersion::RESTRICT_BINDIST) != 0)) {
		restrictions->add_restrict(eix_proto::Restrictions::BINDIST);
	}
	if(unlikely((restrict & ExtendedVersion::RESTRICT_PARALLEL) != 0)) {
		restrictions->add_restrict(eix_proto::Restrictions::PARALLEL);
	}
}

static void add_properties(eix_proto::Properties *properties, ExtendedVersion::Restrict props) {
	if(unlikely((props & ExtendedVersion::PROPERTIES_INTERACTIVE) != 0)) {
		properties->add_property(eix_proto::Properties::INTERACTIVE);
	}
	if(unlikely((props & ExtendedVersion::PROPERTIES_LIVE) != 0)) {
		properties->add_property(eix_proto::Properties::LIVE);
	}
	if(unlikely((props & ExtendedVersion::PROPERTIES_VIRTUAL) != 0)) {
		properties->add_property(eix_proto::Properties::VIRTUAL);
	}
	if(unlikely((props & ExtendedVersion::PROPERTIES_SET) != 0)) {
		properties->add_property(eix_proto::Properties::SET);
	}
}

void PrintProto::finish() {
	if(collection == NULLPTR) {
		return;
	}
	collection->SerializeToOstream(&std::cout);
	delete collection;
	collection = NULLPTR;
}

#else

void PrintProto::start() {}
void PrintProto::finish() {}
void PrintProto::package(Package *) {
	eix::say_error(_("protobuf format is not compiled in"));
	std::exit(EXIT_FAILURE);
}

#endif
