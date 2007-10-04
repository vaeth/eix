// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)                                                         
//   Wolfgang Frisch <xororand@users.sourceforge.net>                    
//   Emil Beinroth <emilbeinroth@gmx.net>                                
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>                     

#ifndef __FORMATSTRING_PRINT_H__
#define __FORMATSTRING_PRINT_H__

#include <string>
#include <output/formatstring.h>
#include <portage/package.h>
#include <portage/version.h>
#include <eixTk/exceptions.h>

std::string get_extended_version(const PrintFormat *fmt, const ExtendedVersion *version, bool pure_text, const std::string &intermediate = "");
std::string get_inst_use(const Package &p, InstVersion &i, const PrintFormat &fmt, const char **a);
std::string getFullInstalled(const Package &p, const PrintFormat &fmt);
std::string getInstalledString(const Package &p, const PrintFormat &fmt, bool pure_text, char formattype, const std::vector<std::string> &prepend);
void print_version(const PrintFormat *fmt, const Version *version, const Package *package, bool with_slot, bool exclude_overlay, bool full);
void print_versions_versions(const PrintFormat *fmt, const Package *p, bool with_slot, bool full);
void print_versions_slots(const PrintFormat *fmt, const Package *p, bool full);
void print_versions(const PrintFormat *fmt, const Package *p, bool with_slot, bool full);

bool print_package_property(const PrintFormat *fmt, const void *entity, const std::string &name) throw(ExBasic);
std::string get_package_property(const PrintFormat *fmt, const void *entity, const std::string &name) throw(ExBasic);

bool print_diff_package_property(const PrintFormat *fmt, const void *void_entity, const std::string &name) throw(ExBasic);
std::string get_diff_package_property(const PrintFormat *fmt, const void *void_entity, const std::string &name) throw(ExBasic);

#endif /* __FORMATSTRING-PRINT_H__ */
