###########################################################################
#   eix is a small utility for searching ebuilds in the                   #
#   Gentoo Linux portage system. It uses indexing to allow quick searches #
#   in package descriptions with regular expressions.                     #
#                                                                         #
#   https://sourceforge.net/projects/eix                                  #
#                                                                         #
#   Copyright (c)                                                         #
#     Wolfgang Frisch <xororand@users.sourceforge.net>                    #
#     Emil Beinroth <emilbeinroth@gmx.net>                                #
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
#   This program is distributed in the hope that it will be useful,       #
#   but WITHOUT ANY WARRANTY; without even the implied warranty of        #
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         #
#   GNU General Public License for more details.                          #
#                                                                         #
#   You should have received a copy of the GNU General Public License     #
#   along with this program; if not, write to the                         #
#   Free Software Foundation, Inc.,                                       #
#   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             #
###########################################################################


AUTOMAKE_OPTIONS = foreign 1.4 dist-bzip2

SUBDIRS = src manpage contrib

CLEANFILES = doxygen.cfg cppcomplete.tags tags

# Stuff from our distribution
EXTRA_DIST = doxygen.cfg.in 

# Remove doxygen/ on clean
clean-local: doxygen-clean

# Build source docs with doxygen
.PHONY:
doxygen: doxygen-clean
	@regex_cmd@ -e "s,\@PACKAGE_NAME\@,$(PACKAGE)," \
		-e "s,\@PACKAGE_VERSION\@,$(VERSION)," doxygen.cfg.in > doxygen.cfg
	@doxygen doxygen.cfg

# Remove build source-docs
doxygen-clean:
	@rm -fr doxygen
