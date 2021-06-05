// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

// check_includes: #include <config.h> include "eixTk/i18n.h"

AddOption(STRING, "I18N_INSTALLEDVERSIONS", P_("I18N_INSTALLEDVERSIONS",
	"Installed versions:"), P_("I18N_INSTALLEDVERSIONS",
	"This variable is only used for delayed substitution.\n"
	"It translates \"Installed versions:\""));

AddOption(STRING, "I18N_VERSION", P_("I18N_VERSION",
	"Version:"), P_("I18N_VERSION",
	"This variable is only used for delayed substitution.\n"
	"It translates \"Version:\""));

AddOption(STRING, "I18N_DATE", P_("I18N_DATE",
	"Date:"), P_("I18N_DATE",
	"This variable is only used for delayed substitution.\n"
	"It translates \"Date:\""));

AddOption(STRING, "I18N_SRC_URI", P_("I18N_SRC_URI",
	"SRC_URI:"), P_("I18N_SRC_URI",
	"This variable is only used for delayed substitution.\n"
	"It translates \"SRC_URI:\""));

AddOption(STRING, "I18N_EAPI", P_("I18N_EAPI",
	"EAPI:"), P_("I18N_EAPI",
	"This variable is only used for delayed substitution.\n"
	"It translates \"EAPI:\""));

AddOption(STRING, "I18N_IUSE", P_("I18N_IUSE",
	"IUSE:"), P_("I18N_IUSE",
	"This variable is only used for delayed substitution.\n"
	"It translates \"IUSE:\""));

AddOption(STRING, "I18N_IUSEALLVERSIONS", P_("I18N_IUSEALLVERSIONS",
	"IUSE \\(all versions\\):"), P_("I18N_IUSEALLVERSIONS",
	"This variable is only used for delayed substitution.\n"
	"It translates \"IUSE \\(all versions\\):\""));

AddOption(STRING, "I18N_REQUIRED_USE", P_("I18N_REQUIRED_USE",
	"REQUIRED_USE:"), P_("I18N_IUSE",
	"This variable is only used for delayed substitution.\n"
	"It translates \"REQUIRED_USE:\""));

AddOption(STRING, "I18N_USE", P_("I18N_USE",
	"USE:"), P_("I18N_USE",
	"This variable is only used for delayed substitution.\n"
	"It translates \"USE:\""));

AddOption(STRING, "I18N_DEPEND", P_("I18N_DEPEND",
	"DEPEND:"), P_("I18N_DEPEND",
	"This variable is only used for delayed substitution.\n"
	"It translates \"DEPEND:\""));

AddOption(STRING, "I18N_RDEPEND", P_("I18N_RDEPEND",
	"RDEPEND:"), P_("I18N_RDEPEND",
	"This variable is only used for delayed substitution.\n"
	"It translates \"RDEPEND:\""));

AddOption(STRING, "I18N_PDEPEND", P_("I18N_PDEPEND",
	"PDEPEND:"), P_("I18N_PDEPEND",
	"This variable is only used for delayed substitution.\n"
	"It translates \"PDEPEND:\""));

AddOption(STRING, "I18N_BDEPEND", P_("I18N_BDEPEND",
	"BDEPEND:"), P_("I18N_BDEPEND",
	"This variable is only used for delayed substitution.\n"
	"It translates \"BDEPEND:\""));

AddOption(STRING, "I18N_IDEPEND", P_("I18N_IDEPEND",
	"IDEPEND:"), P_("I18N_IDEPEND",
	"This variable is only used for delayed substitution.\n"
	"It translates \"IDEPEND:\""));

AddOption(STRING, "I18N_KEYWORDS", P_("I18N_KEYWORDS",
	"KEYWORDS:"), P_("I18N_KEYWORDS",
	"This variable is only used for delayed substitution.\n"
	"It translates \"KEYWORDS:\""));

AddOption(STRING, "I18N_KEYWORDSS", P_("I18N_KEYWORDSS",
	"KEYWORDS*:"), P_("I18N_KEYWORDSS",
	"This variable is only used for delayed substitution.\n"
	"It translates \"KEYWORDS*:\""));

AddOption(STRING, "I18N_MASK", P_("I18N_MASK",
	"Mask:"), P_("I18N_MASK",
	"This variable is only used for delayed substitution.\n"
	"It translates \"Mask:\""));

AddOption(STRING, "I18N_MARKED", P_("I18N_MARKED",
	"Marked:"), P_("I18N_MARKED",
	"This variable is only used for delayed substitution.\n"
	"It translates \"Marked:\""));

AddOption(STRING, "I18N_AVAILABLEVERSIONS", P_("I18N_AVAILABLEVERSIONS",
	"Available versions:"), P_("I18N_AVAILABLEVERSIONS",
	"This variable is only used for delayed substitution.\n"
	"It translates \"Available versions:\""));

AddOption(STRING, "I18N_BESTVERSIONSSLOT", P_("I18N_BESTVERSIONSSLOT",
	"Best versions/slot:"), P_("I18N_BESTVERSIONSSLOT",
	"This variable is only used for delayed substitution.\n"
	"It translates \"Best versions/slot:\""));

AddOption(STRING, "I18N_RECOMMENDATION", P_("I18N_RECOMMENDATION",
	"Recommendation:"), P_("I18N_RECOMMENDATION",
	"This variable is only used for delayed substitution.\n"
	"It translates \"Recommendation:\""));

AddOption(STRING, "I18N_UPGRADE", P_("I18N_UPGRADE",
	"Upgrade"), P_("I18N_UPGRADE",
	"This variable is only used for delayed substitution.\n"
	"It translates \"Upgrade\""));

AddOption(STRING, "I18N_DOWNGRADE", P_("I18N_DOWNGRADE",
	"Downgrade"), P_("I18N_DOWNGRADE",
	"This variable is only used for delayed substitution.\n"
	"It translates \"Downgrade\""));

AddOption(STRING, "I18N_AND", P_("I18N_AND",
	" and "), P_("I18N_AND",
	"This variable is only used for delayed substitution.\n"
	"It translates \" and \""));

AddOption(STRING, "I18N_NONE", P_("I18N_NONE",
	"None"), P_("I18N_NONE",
	"This variable is only used for delayed substitution.\n"
	"It translates \"None\""));

AddOption(STRING, "I18N_PACKAGESETS", P_("I18N_PACKAGESETS",
	"Package sets:"), P_("I18N_PACKAGESETS",
	"This variable is only used for delayed substitution.\n"
	"It translates \"Package sets:\""));

AddOption(STRING, "I18N_HOMEPAGE", P_("I18N_HOMEPAGE",
	"Homepage:"), P_("I18N_HOMEPAGE",
	"This variable is only used for delayed substitution.\n"
	"It translates \"Homepage:\""));

AddOption(STRING, "I18N_FINDOPENBUGS", P_("I18N_FINDOPENBUGS",
	"Find open bugs:"), P_("I18N_FINDOPENBUGS",
	"This variable is only used for delayed substitution.\n"
	"It translates \"Find open bugs:\""));

AddOption(STRING, "I18N_DESCRIPTION", P_("I18N_DESCRIPTION",
	"Description:"), P_("I18N_DESCRIPTION",
	"This variable is only used for delayed substitution.\n"
	"It translates \"Description:\""));

AddOption(STRING, "I18N_LICENSE", P_("I18N_LICENSE",
	"License:"), P_("I18N_LICENSE",
	"This variable is only used for delayed substitution.\n"
	"It translates \"License:\""));

AddOption(STRING, "C_COLUMN_TITLE",
	"5", P_("C_COLUMN_TITLE",
	"This variable is only used for delayed substitution.\n"
	"It defines the first column for the titles in the C locale."));

AddOption(STRING, "I18N_COLUMN_TITLE", P_("I18N_COLUMN_TITLE",
	"%{C_COLUMN_TITLE}"), P_("I18N_COLUMN_TITLE",
	"This variable is only used for delayed substitution.\n"
	"It defines the first column for the titles in the current locale.\n"
	"It should be redefined only for languages with extremely long translations."));

AddOption(STRING, "C_COLUMN_CONTENT",
	"26", P_("C_COLUMN_CONTENT",
	"This variable is only possibly used for delayed substitution in\n"
	"I18N_COLUMN_CONTENT. It defines the first column of the content\n"
	"in the C locale."));

AddOption(STRING, "I18N_COLUMN_CONTENT", P_("I18N_COLUMN_CONTENT",
	"%{C_COLUMN_CONTENT}"), P_("I18N_COLUMN_CONTENT",
	"This variable is only used for delayed substitution.\n"
	"It defines the first column for the content in the current locale."));

AddOption(STRING, "C_COLUMN_INST_TITLE",
	"%{I18N_COLUMN_CONTENT}", P_("C_COLUMN_INST_TITLE",
	"This variable is only possibly used for delayed substitution in\n"
	"I18N_COLUMN_INST_TITLE. It defines the first column for the title of\n"
	"installed versions in the C locale."));

AddOption(STRING, "I18N_COLUMN_INST_TITLE", P_("I18N_COLUMN_INST_TITLE",
	"%{C_COLUMN_INST_TITLE}"), P_("I18N_COLUMN_INST_TITLE",
	"This variable is only used for delayed substitution.\n"
	"It defines the first column for the title of installed versions\n"
	"in the current locale."));

AddOption(STRING, "C_COLUMN_INST_CONTENT",
	"37", P_("C_COLUMN_INST_CONTENT",
	"This variable is only possibly used for delayed substitution in\n"
	"I18N_COLUMN_INST_CONTENT. It defines the first column for the content of\n"
	"installed versions in the C locale."));

AddOption(STRING, "I18N_COLUMN_INST_CONTENT", P_("I18N_COLUMN_INST_CONTENT",
	"%{C_COLUMN_INST_CONTENT}"), P_("I18N_COLUMN_INST_CONTENT",
	"This variable is only used for delayed substitution.\n"
	"It defines the first column for the content of installed versions\n"
	"in the current locale."));

AddOption(STRING, "C_COLUMN_AVAILABLE_TITLE",
	"12", P_("C_COLUMN_AVAILABLE_TITLE",
	"This variable is only possibly used for delayed substitution in\n"
	"I18N_COLUMN_AVAILABLE_TITLE. It defines the first column for the title of\n"
	"available versions in the C locale."));

AddOption(STRING, "I18N_COLUMN_AVAILABLE_TITLE", P_("I18N_COLUMN_AVAILABLE_TITLE",
	"%{C_COLUMN_AVAILABLE_TITLE}"), P_("I18N_COLUMN_AVAILABLE_TITLE",
	"This variable is only used for delayed substitution.\n"
	"It defines the first column for the title of available versions\n"
	"in the current locale.\n"
	"It should be redefined only for languages with extremely long translations."));

AddOption(STRING, "C_COLUMN_AVAILABLE_CONTENT",
	"%{I18N_COLUMN_CONTENT}", P_("C_COLUMN_AVAILABLE_CONTENT",
	"This variable is only possibly used for delayed substitution in\n"
	"I18N_COLUMN_AVAILABLE_CONTENT. It defines the first column for the content\n"
	"of available versions in the C locale."));

AddOption(STRING, "I18N_COLUMN_AVAILABLE_CONTENT", P_("I18N_COLUMN_AVAILABLE_CONTENT",
	"%{C_COLUMN_AVAILABLE_CONTENT}"), P_("I18N_COLUMN_AVAILABLE_CONTENT",
	"This variable is only used for delayed substitution.\n"
	"It defines the first column for the content of available versions\n"
	"in the current locale."));
