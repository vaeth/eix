/***************************************************************************
 *   eix is a small utility for searching ebuilds in the                   *
 *   Gentoo Linux portage system. It uses indexing to allow quick searches *
 *   in package descriptions with regular expressions.                     *
 *                                                                         *
 *   https://sourceforge.net/projects/eix                                  *
 *                                                                         *
 *   Copyright (c)                                                         *
 *     Wolfgang Frisch <xororand@users.sourceforge.net>                    *
 *     Emil Beinroth <emilbeinroth@gmx.net>                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

/* we use strndup */
#define _GNU_SOURCE

#include "mask.h"
#include <portage/keywords.h>

#include <eixTk/regexp.h>
#include <eixTk/exceptions.h>

#include <iostream>

using namespace std;

typedef struct {
	char *str;
	Mask::Operator op;
} OperatorTable;

/** Data-driven programming is nice :) */
static OperatorTable operators[] = {
	{ "",   Mask::maskOpAll },
	{ "<" , Mask::maskOpLess },
	{ "<=", Mask::maskOpLessEqual },
	{ "=" , Mask::maskOpEqual },
	{ ">=", Mask::maskOpGreaterEqual },
	{ "~" , Mask::maskOpRevisions },
	{ ">" , Mask::maskOpGreater },
	{ NULL, Mask::maskOpAll /* doesn't matter */ }
};

const char *
Mask::getMaskOperator(Mask::Operator type)
{
	int i = 0;
	for( ;operators[i].str != NULL; ++i) {
		if( type == operators[i].op ) {
			return operators[i].str;
			break;
		}
	}
	return NULL; /* Should never be reached because we already thrown up if the
					maskoperator isn't in the table. */
}

/***************************************************************************/
/* TODO: How about making this one expression? (appro) */
/* Regular expressions, used by splitMaskString */
Regex re_mask1("\\([~<=>]*\\)\\([a-z0-9-]\\+\\)/\\(.*\\)", 0);
Regex re_rest2name( "\\(.*\\)-[^a-zA-Z].*", 0 );
Regex re_rest2ver(".*-\\([^a-zA-Z].*\\)", 0);
/***************************************************************************/

/** Constructor. */
Mask::Mask( string strMask, Type type)
{
	_category = _name = NULL;
	_wcpos = 0;
	_type = type;
	splitMaskString( strMask );
}

/** Deconstructor, frees rootcat, subcat, name, ver */
Mask::~Mask()
{
	free(_category);
	free(_name);
}

/** split a "mask string" into its components */
void Mask::splitMaskString( string strMask ) throw(ExBasic)
{
	regmatch_t rm[6];
	const char *strm = strMask.c_str();
	char *comp_operator = NULL,
		 *rest = NULL,
		 *wildcardptr = NULL,
		 *ver = NULL;

	/* split strm into compararator, cat, subcat, rest */
	if( regexec( re_mask1.get(), strm, 4, rm, 0  ) == 0 ) {
		OOM_ASSERT(comp_operator = strndup( &(strm[rm[1].rm_so]), rm[1].rm_eo - rm[1].rm_so));
		OOM_ASSERT(_category      = strndup( &(strm[rm[2].rm_so]), rm[2].rm_eo - rm[2].rm_so));
		OOM_ASSERT(rest          = strndup( &(strm[rm[3].rm_so]), rm[3].rm_eo - rm[3].rm_so));
	} else
		throw ExBasic("Unable to split mask \"%s\" into compararator-category-rest", strm);

	// determine comparison operator
	int i = 0;
	for( ;operators[i].str != NULL; ++i) {
		if( strcmp(comp_operator, operators[i].str) == 0 ) {
			_op = operators[i].op;
			break;
		}
	}
	if(operators[i].str == NULL) {
		throw ExBasic("Unkown operator \"%s\" in \"%s\"", comp_operator, strm);
	}

	// split rest into name / version
	if( regexec( re_rest2ver.get(), rest, 2, rm, 0 ) == 0 ) {
		OOM_ASSERT(ver = strndup( &(rest[rm[1].rm_so]), rm[1].rm_eo - rm[1].rm_so));
		if( regexec( re_rest2name.get(), rest, 2, rm, 0 ) == 0 )
			OOM_ASSERT(_name = strndup( &(rest[rm[1].rm_so]), rm[1].rm_eo - rm[1].rm_so));
		else
			throw ExBasic("Unable to split \"%s\" from mask \"%s\" into name-version.", rest, strm);
	}
	else {
		OOM_ASSERT(_name = strdup( rest ));
		OOM_ASSERT(ver = strdup(""));
	}

	if((wildcardptr = strchr( ver, '*' )) == NULL) {
		/* If there is a version we need to parse it. */
		if(_op != maskOpAll) {
			parseVersion(ver);
		}
	}
	else {
		_wcpos = (wildcardptr - ver);
		parseVersion(ver, _wcpos);
	}

	// clean up
	free(ver);
	free(rest);
	free(comp_operator);
}

/** Tests if the mask applies to a Version.
 * @param name name of package (NULL if shall not be tested)
 * @param category category of package (NULL if shall not be tested)
 * @return true if applies. */
bool Mask::test(Version& ve, BasicVersion *bv)
{
	if(bv == NULL) {
		bv = this;
	}

	switch(_op) {
		case maskOpAll:
			return true;
		case maskOpLess:
			if( ve < *bv)
				return true;
			return false;
		case maskOpLessEqual:
			if( ve <= *bv)
				return true;
			return false;
		case maskOpEqual:
			if( ve == *bv)
				return true;
			return false;
		case maskOpGreaterEqual:
			if( ve >= *bv)
				return true;
			return false;
		case maskOpGreater:
			if( ve > *bv)
				return true;
			return false;
		case maskOpRevisions:
			if( bv->comparePrimary(ve) == 0
					&& ( bv->suffixlevel == ve.suffixlevel )
					&& ( bv->suffixnum == ve.suffixnum ) )
				return true;
			return false;
	}
	return false; /* Never reached */
}

void Mask::expand(Package *pkg) {
	/* Test every version if our mask-strings would match. */
	vector<Version*>::iterator vi = pkg->begin();
	while(vi != pkg->end()) {
		if(strncmp((*vi)->full.c_str(), getVersion(), _wcpos) == 0) {
			/* For every matching version, expand and apply to every version. */
			vector<Version*>::iterator _vi = pkg->begin();
			while(_vi != pkg->end()) {
				apply(**_vi, (BasicVersion *)*vi);
				++_vi;
			}
		}
		++vi;
	}
}

vector<Version*>
Mask::getMatches(Package &pkg) {
	vector<Version*> ret;
	if(_wcpos != 0) {
		/* Test every version. */
		for(vector<Version*>::iterator version = pkg.begin(); version != pkg.end(); ++version) {
			if(strncmp((*version)->full.c_str(), getVersion(), _wcpos) == 0) {
				for(vector<Version*>::iterator v = pkg.begin(); v != pkg.end(); ++v) {
					if(test(**v, (BasicVersion*)*version))
						ret.push_back(*v);
				}
			}
		}
		vector<Version*>::iterator new_end = unique(ret.begin(), ret.end());
		ret.erase(new_end, ret.end());
	}
	else {
		for(vector<Version*>::iterator version = pkg.begin(); version != pkg.end(); ++version) {
			if(test(**version, this))
				ret.push_back(*version);
		}
	}
	return ret;
}

/** Sets the stability members of all version in package according to the mask. */
void 
Mask::checkMask(Package& pkg, const bool check_category, const bool check_name)
{
	if((check_name && pkg.name.c_str() != _name)
		|| (check_category && pkg.category.c_str() != _category))
		return;

	/* If we have a wildcard we need to expand it first.
	 * TODO: cleanup and perhaps rethink the whole thing */
	if(_wcpos != 0) {
		expand(&pkg);
	}
	else {
		vector<Version*>::iterator vi = pkg.begin();
		while(vi != pkg.end()) {
			apply(*(*vi++));
		}
	}
}

/** Sets the stability & masked members of ve according to the mask.
 * @param ve Version instance to be set
 * @param name name of package (NULL if shall not be tested) */
void Mask::apply(Version& ve, BasicVersion *bv)
{
	if(bv == NULL) {
		bv = this;
	}
	/* Don't do sensless checking. */
	if(        (_type == maskUnmask
				&& !ve.isPackageMask())     /* Unmask but is not masked */
			|| (_type == maskMask
				&& ve.isPackageMask())      /* Mask but is already masked */
			|| (_type == maskAllowedByProfile
				&& ve.isProfileMask())      /* Won't change anything cause already masked by profile */
			|| (_type == maskInSystem
				&& ve.isSystem()
				&& ve.isProfileMask())
			|| (_type == maskTypeNone)      /* We have nothing to masked. */
			)
		return;

	switch(_type) {
		case maskUnmask:
			if(test(ve, bv))
				ve &= ~Keywords::PACKAGE_MASK;
			return;
		case maskMask:
			if(test(ve, bv))
				ve |= Keywords::PACKAGE_MASK;
			return;
		case maskInSystem:
			if( test(ve, bv) )
				ve |= Keywords::SYSTEM_PACKAGE;
			else
				ve |= Keywords::PROFILE_MASK;
			return;
		case maskAllowedByProfile:
			if(!test(ve, bv))
				ve |= Keywords::PROFILE_MASK;
			return;
		case maskTypeNone:
			return;
	}
}

vector<Mask*> *MaskList::get(Package *p) {
	return &((*this)[p->category][p->name]);
}
