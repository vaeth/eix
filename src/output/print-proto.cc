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

void PrintProto::start() {
	eix_output = new EixOutput();
	category_index.clear();
}

void PrintProto::package(Package *pkg) {
	if(eix_output == NULLPTR) {
		start();
	}
	EixOutput_Category *category;
	if(eix_output->category_size() != 0) {
		// Re-use existing category if possible
		int& index = category_index[pkg->category];
		category = eix_output->mutable_category(index);
		if(index != 0 || category->category() != pkg->category) {
			// category is actually new
			index = eix_output->category_size();
			category = eix_output->add_category();
		}
	} else {
		category = eix_output->add_category();
	}
	EixOutput_Package *package = category->add_package();
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
		EixOutput_Version *version = package->add_version();
		version->set_id(ver->getFull());
		version->set_eapi(ver->eapi.get());

		EixOutput_Repository *repository = version->mutable_repository();
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

		MaskFlags currmask(ver->maskflags);
		KeywordsFlags currkey(ver->keyflags);
		MaskFlags wasmask;
		KeywordsFlags waskey;
		stability->calc_version_flags(false, &wasmask, &waskey, *ver, pkg);
		if(wasmask.isHardMasked()) {
			if(currmask.isProfileMask()) {
				version->add_mask_type(EixOutput::PROFILE);
			} else if(currmask.isPackageMask()) {
				version->add_mask_type(EixOutput::PROFILE);
			} else if(wasmask.isProfileMask()) {
				version->add_mask_type(EixOutput::PROFILE);
				version->add_unmask_type(EixOutput::PACKAGE_UNMASK);
			} else {
				version->add_mask_type(EixOutput::HARD);
				version->add_unmask_type(EixOutput::PACKAGE_UNMASK);
			}
		} else if(currmask.isHardMasked()) {
			version->add_mask_type(EixOutput::PACKAGE_MASK);
		}

		if(currkey.isStable()) {
			if(waskey.isStable()) {
				//
			} else if(waskey.isUnstable()) {
				version->add_mask_type(EixOutput::KEYWORD);
				version->add_unmask_type(EixOutput::PACKAGE_KEYWORDS);
			} else if(waskey.isMinusKeyword()) {
				version->add_mask_type(EixOutput::MINUS_KEYWORD);
				version->add_unmask_type(EixOutput::PACKAGE_KEYWORDS);
			} else if(waskey.isAlienStable()) {
				version->add_mask_type(EixOutput::ALIEN_STABLE);
				version->add_unmask_type(EixOutput::PACKAGE_KEYWORDS);
			} else if(waskey.isAlienUnstable()) {
				version->add_mask_type(EixOutput::ALIEN_UNSTABLE);
				version->add_unmask_type(EixOutput::PACKAGE_KEYWORDS);
			} else if(waskey.isMinusUnstable()) {
				version->add_mask_type(EixOutput::MINUS_UNSTABLE);
				version->add_unmask_type(EixOutput::PACKAGE_KEYWORDS);
			} else if(waskey.isMinusAsterisk()) {
				version->add_mask_type(EixOutput::MINUS_ASTERISK);
				version->add_unmask_type(EixOutput::PACKAGE_KEYWORDS);
			} else {
				version->add_mask_type(EixOutput::MISSING_KEYWORDS);
				version->add_unmask_type(EixOutput::PACKAGE_KEYWORDS);
			}
		} else if(currkey.isUnstable()) {
			version->add_mask_type(EixOutput::KEYWORD);
		} else if(currkey.isMinusKeyword()) {
			version->add_mask_type(EixOutput::MINUS_KEYWORD);
		} else if(currkey.isAlienStable()) {
			version->add_mask_type(EixOutput::ALIEN_STABLE);
		} else if(currkey.isAlienUnstable()) {
			version->add_mask_type(EixOutput::ALIEN_UNSTABLE);
		} else if(currkey.isMinusUnstable()) {
			version->add_mask_type(EixOutput::MINUS_UNSTABLE);
		} else if(currkey.isMinusAsterisk()) {
			version->add_mask_type(EixOutput::MINUS_ASTERISK);
		} else {
			version->add_mask_type(EixOutput::MISSING_KEYWORDS);
		}

		if(unlikely(ver->have_reasons())) {
			const Version::Reasons *reasons_ptr(ver->reasons_ptr());
			for(Version::Reasons::const_iterator it(reasons_ptr->begin());
				unlikely(it != reasons_ptr->end()); ++it) {
				const WordVec *vec(it->asWordVecPtr());
				if((vec == NULLPTR) || (vec->empty())) {
					continue;
				}
				EixOutput::Lines *lines = version->add_mask_reason();
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
		EixOutput_Installed *installed = NULLPTR;

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
			if(unlikely((restrict & ExtendedVersion::RESTRICT_BINCHECKS) != 0)) {
				version->add_restrict(EixOutput::BINCHECKS);
			}
			if(unlikely((restrict & ExtendedVersion::RESTRICT_STRIP) != 0)) {
				version->add_restrict(EixOutput::STRIP);
			}
			if(unlikely((restrict & ExtendedVersion::RESTRICT_TEST) != 0)) {
				version->add_restrict(EixOutput::TEST);
			}
			if(unlikely((restrict & ExtendedVersion::RESTRICT_USERPRIV) != 0)) {
				version->add_restrict(EixOutput::USERPRIV);
			}
			if(unlikely((restrict & ExtendedVersion::RESTRICT_INSTALLSOURCES) != 0)) {
				version->add_restrict(EixOutput::INSTALLSOURCES);
			}
			if(unlikely((restrict & ExtendedVersion::RESTRICT_FETCH) != 0)) {
				version->add_restrict(EixOutput::FETCH);
			}
			if(unlikely((restrict & ExtendedVersion::RESTRICT_MIRROR) != 0)) {
				version->add_restrict(EixOutput::MIRROR);
			}
			if(unlikely((restrict & ExtendedVersion::RESTRICT_PRIMARYURI) != 0)) {
				version->add_restrict(EixOutput::PRIMARYURI);
			}
			if(unlikely((restrict & ExtendedVersion::RESTRICT_BINDIST) != 0)) {
				version->add_restrict(EixOutput::BINDIST);
			}
			if(unlikely((restrict & ExtendedVersion::RESTRICT_PARALLEL) != 0)) {
				version->add_restrict(EixOutput::PARALLEL);
			}
		}
		ExtendedVersion::Restrict properties(ver->propertiesFlags);
		if(unlikely(properties != ExtendedVersion::PROPERTIES_NONE)) {
			if(unlikely((properties & ExtendedVersion::PROPERTIES_INTERACTIVE) != 0)) {
				version->add_property(EixOutput::INTERACTIVE);
			}
			if(unlikely((properties & ExtendedVersion::PROPERTIES_LIVE) != 0)) {
				version->add_property(EixOutput::LIVE);
			}
			if(unlikely((properties & ExtendedVersion::PROPERTIES_VIRTUAL) != 0)) {
				version->add_property(EixOutput::VIRTUAL);
			}
			if(unlikely((properties & ExtendedVersion::PROPERTIES_SET) != 0)) {
				version->add_property(EixOutput::SET);
			}
		}

		string full_kw = ver->get_full_keywords();
		if(!full_kw.empty()) {
			version->set_keywords(full_kw);
		}
		string eff_kw = ver->get_effective_keywords();
		if(unlikely(full_kw != eff_kw)) {
			version->set_effective_keywords(eff_kw);
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

void PrintProto::finish() {
	if(eix_output == NULLPTR) {
		return;
	}
	eix_output->SerializeToOstream(&std::cout);
	delete eix_output;
	eix_output = NULLPTR;
}

#else

void PrintProto::start() {}
void PrintProto::finish() {}
void PrintProto::package(Package *) {
	eix::say_error(_("protobuf format is not compiled in"));
	std::exit(EXIT_FAILURE);
}

#endif
