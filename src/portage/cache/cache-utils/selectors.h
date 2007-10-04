// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)                                                         
//   Wolfgang Frisch <xororand@users.sourceforge.net>                    
//   Emil Beinroth <emilbeinroth@gmx.net>                                
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>                     

#ifndef __SELECTORS_H__
#define __SELECTORS_H__

#include <config.h>
#include <dirent.h>

int package_selector (SCANDIR_ARG3 dent);
int ebuild_selector (SCANDIR_ARG3 dent);

#endif /* __SELECTORS_H__ */
