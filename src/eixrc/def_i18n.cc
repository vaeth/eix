// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

// Make check_includes happy: #include <config.h> #include "eixTk/i18n.h"

AddOption(STRING, "I18N_INSTALLEDVERSIONS",
	_("Installed versions:"), _(
	"This variable is only used for delayed substitution.\n"
	"It translates \"Installed versions:\""));

AddOption(STRING, "I18N_VERSION",
	_("Version:"), _(
	"This variable is only used for delayed substitution.\n"
	"It translates \"Version:\""));

AddOption(STRING, "I18N_DATE",
	_("Date:"), _(
	"This variable is only used for delayed substitution.\n"
	"It translates \"Date:\""));

AddOption(STRING, "I18N_IUSE",
	_("IUSE:"), _(
	"This variable is only used for delayed substitution.\n"
	"It translates \"IUSE:\""));

AddOption(STRING, "I18N_IUSEALLVERSIONS",
	_("IUSE \\(all versions\\):"), _(
	"This variable is only used for delayed substitution.\n"
	"It translates \"IUSE \\(all versions\\):\""));

AddOption(STRING, "I18N_USE",
	_("USE:"), _(
	"This variable is only used for delayed substitution.\n"
	"It translates \"USE:\""));

AddOption(STRING, "I18N_DEPEND",
	_("DEPEND:"), _(
	"This variable is only used for delayed substitution.\n"
	"It translates \"DEPEND:\""));

AddOption(STRING, "I18N_RDEPEND",
	_("RDEPEND:"), _(
	"This variable is only used for delayed substitution.\n"
	"It translates \"RDEPEND:\""));

AddOption(STRING, "I18N_PDEPEND",
	_("PDEPEND:"), _(
	"This variable is only used for delayed substitution.\n"
	"It translates \"PDEPEND:\""));

AddOption(STRING, "I18N_HDEPEND",
	_("HDEPEND:"), _(
	"This variable is only used for delayed substitution.\n"
	"It translates \"HDEPEND:\""));

AddOption(STRING, "I18N_KEYWORDS",
	_("KEYWORDS:"), _(
	"This variable is only used for delayed substitution.\n"
	"It translates \"KEYWORDS:\""));

AddOption(STRING, "I18N_KEYWORDSS",
	_("KEYWORDS*:"), _(
	"This variable is only used for delayed substitution.\n"
	"It translates \"KEYWORDS*:\""));

AddOption(STRING, "I18N_MASK",
	_("Mask:"), _(
	"This variable is only used for delayed substitution.\n"
	"It translates \"Mask:\""));

AddOption(STRING, "I18N_MARKED",
	_("Marked:"), _(
	"This variable is only used for delayed substitution.\n"
	"It translates \"Marked:\""));

AddOption(STRING, "I18N_AVAILABLEVERSIONS",
	_("Available versions:"), _(
	"This variable is only used for delayed substitution.\n"
	"It translates \"Available versions:\""));

AddOption(STRING, "I18N_BESTVERSIONSSLOT",
	_("Best versions/slot:"), _(
	"This variable is only used for delayed substitution.\n"
	"It translates \"Best versions/slot:\""));

AddOption(STRING, "I18N_RECOMMENDATION",
	_("Recommendation:"), _(
	"This variable is only used for delayed substitution.\n"
	"It translates \"Recommendation:\""));

AddOption(STRING, "I18N_UPGRADE",
	_("Upgrade"), _(
	"This variable is only used for delayed substitution.\n"
	"It translates \"Upgrade\""));

AddOption(STRING, "I18N_DOWNGRADE",
	_("Downgrade"), _(
	"This variable is only used for delayed substitution.\n"
	"It translates \"Downgrade\""));

AddOption(STRING, "I18N_AND",
	_(" and "), _(
	"This variable is only used for delayed substitution.\n"
	"It translates \" and \""));

AddOption(STRING, "I18N_NONE",
	_("None"), _(
	"This variable is only used for delayed substitution.\n"
	"It translates \"None\""));

AddOption(STRING, "I18N_PACKAGESETS",
	_("Package sets:"), _(
	"This variable is only used for delayed substitution.\n"
	"It translates \"Package sets:\""));

AddOption(STRING, "I18N_HOMEPAGE",
	_("Homepage:"), _(
	"This variable is only used for delayed substitution.\n"
	"It translates \"Homepage:\""));

AddOption(STRING, "I18N_FINDOPENBUGS",
	_("Find open bugs:"), _(
	"This variable is only used for delayed substitution.\n"
	"It translates \"Find open bugs:\""));

AddOption(STRING, "I18N_DESCRIPTION",
	_("Description:"), _(
	"This variable is only used for delayed substitution.\n"
	"It translates \"Description:\""));

AddOption(STRING, "I18N_LICENSE",
	_("License:"), _(
	"This variable is only used for delayed substitution.\n"
	"It translates \"License:\""));

AddOption(STRING, "C_COLUMN_TITLE",
	"5", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the first column for the titles in the C locale."));

AddOption(STRING, "I18N_COLUMN_TITLE",
	_("%{C_COLUMN_TITLE}"), _(
	"This variable is only used for delayed substitution.\n"
	"It defines the first column for the titles in the current locale.\n"
	"It should be redefined only for languages with extremely long translations."));

AddOption(STRING, "C_COLUMN_CONTENT",
	"26", _(
	"This variable is only possibly used for delayed substitution in\n"
	"I18N_COLUMN_CONTENT. It defines the first column of the content\n"
	"in the C locale."));

AddOption(STRING, "I18N_COLUMN_CONTENT",
	_("%{C_COLUMN_CONTENT}"), _(
	"This variable is only used for delayed substitution.\n"
	"It defines the first column for the content in the current locale."));

AddOption(STRING, "C_COLUMN_INST_TITLE",
	"%{I18N_COLUMN_CONTENT}", _(
	"This variable is only possibly used for delayed substitution in\n"
	"I18N_COLUMN_INST_TITLE. It defines the first column for the title of\n"
	"installed versions in the C locale."));

AddOption(STRING, "I18N_COLUMN_INST_TITLE",
	_("%{C_COLUMN_INST_TITLE}"), _(
	"This variable is only used for delayed substitution.\n"
	"It defines the first column for the title of installed versions\n"
	"in the current locale."));

AddOption(STRING, "C_COLUMN_INST_CONTENT",
	"37", _(
	"This variable is only possibly used for delayed substitution in\n"
	"I18N_COLUMN_INST_CONTENT. It defines the first column for the content of\n"
	"installed versions in the C locale."));

AddOption(STRING, "I18N_COLUMN_INST_CONTENT",
	_("%{C_COLUMN_INST_CONTENT}"), _(
	"This variable is only used for delayed substitution.\n"
	"It defines the first column for the content of installed versions\n"
	"in the current locale."));

AddOption(STRING, "C_COLUMN_AVAILABLE_TITLE",
	"15", _(
	"This variable is only possibly used for delayed substitution in\n"
	"I18N_COLUMN_AVAILABLE_TITLE. It defines the first column for the title of\n"
	"available versions in the C locale."));

AddOption(STRING, "I18N_COLUMN_AVAILABLE_TITLE",
	_("%{C_COLUMN_AVAILABLE_TITLE}"), _(
	"This variable is only used for delayed substitution.\n"
	"It defines the first column for the title of available versions\n"
	"in the current locale.\n"
	"It should be redefined only for languages with extremely long translations."));

AddOption(STRING, "C_COLUMN_AVAILABLE_CONTENT",
	"%{I18N_COLUMN_CONTENT}", _(
	"This variable is only possibly used for delayed substitution in\n"
	"I18N_COLUMN_AVAILABLE_CONTENT. It defines the first column for the content\n"
	"of available versions in the C locale."));

AddOption(STRING, "I18N_COLUMN_AVAILABLE_CONTENT",
	_("%{C_COLUMN_AVAILABLE_CONTENT}"), _(
	"This variable is only used for delayed substitution.\n"
	"It defines the first column for the content of available versions\n"
	"in the current locale."));
