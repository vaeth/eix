// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include "formatstring.h"
#include <config.h>
#include <eixTk/ansicolor.h>
#include <eixTk/exceptions.h>
#include <eixTk/formated.h>
#include <eixTk/i18n.h>
#include <eixTk/likely.h>
#include <eixTk/stringutils.h>
#include <eixrc/eixrc.h>
#include <portage/basicversion.h>
#include <portage/extendedversion.h>

#include <iostream>
#include <set>
#include <string>
#include <vector>

#include <cstddef>
#include <cstring>

class PortageSettings;

using namespace std;

void
MarkedList::add(const char *pkg, const char *ver)
{
	pair<string, BasicVersion*> p;
	p.first = string(pkg);
	if(ver)
		p.second = new BasicVersion(ver);
	else
		p.second = NULL;
	insert(p);
}

inline MarkedList::CIPair
MarkedList::equal_range_pkg(const Package &pkg) const
{
	return equal_range(pkg.category + "/" + pkg.name);
}

/** Return pointer to (newly allocated) sorted vector of marked versions,
    or NULL. With nonversion argument, its content will decide whether
    the package was marked with a non-version argument */
set<BasicVersion> *
MarkedList::get_marked_versions(const Package &pkg, bool *nonversion) const
{
	CIPair beg_end(equal_range_pkg(pkg));
	if(nonversion != NULL)
		*nonversion = false;
	if(likely((beg_end.first == end()) || (beg_end.first == beg_end.second)))// no match
		return NULL;
	set<BasicVersion> *ret(NULL);
	for(const_iterator it(beg_end.first); likely(it != beg_end.second); ++it) {
		BasicVersion *p(it->second);
		if(p == NULL) {
			if(nonversion != NULL)
				*nonversion = true;
			continue;
		}
		if(ret == NULL)
			ret = new set<BasicVersion>;
		ret->insert(*p);
	}
	if(likely(ret == NULL))// No version was explicitly marked
		return NULL;
	return ret;
}

/** Return true if pkg is marked. If ver is non-NULL also *ver must match */
bool
MarkedList::is_marked(const Package &pkg, const BasicVersion *ver) const
{
	CIPair beg_end = equal_range_pkg(pkg);
	if((beg_end.first == end()) || (beg_end.first == beg_end.second))// no match
		return false;
	if(ver == NULL) // do not care about versions
		return true;
	for(const_iterator it(beg_end.first); likely(it != beg_end.second); ++it ) {
		BasicVersion *p(it->second);
		if(p != NULL) {
			if(unlikely(*p == *ver))
				return true;
		}
	}
	return false;
}

/** Return String of marked versions (sorted) */
string
MarkedList::getMarkedString(const Package &pkg) const
{
	bool nonversion;
	set<BasicVersion> *marked(get_marked_versions(pkg, &nonversion));
	if(marked == NULL)
		return nonversion ? "*" : "";
	string ret;
	if(nonversion)
		ret = "*";
	for(set<BasicVersion>::const_iterator it(marked->begin());
		likely(it != marked->end()); ++it ) {
		if(!ret.empty())
			ret.append(1, ' ');
		ret.append(it->getFull());
	}
	delete marked;
	return ret;
}

LocalCopy::LocalCopy(const PrintFormat *fmt, Package *pkg) :
	PackageSave((fmt->recommend_mode) == LOCALMODE_DEFAULT ? NULL : pkg)
{
	if((fmt->recommend_mode) == LOCALMODE_DEFAULT)
		return;
	if(fmt->recommend_mode == LOCALMODE_LOCAL) {
		fmt->StabilityLocal(*pkg);
		return;
	}
	//(fmt->recommend_mode == LOCALMODE_NONLOCAL)
	fmt->StabilityNonlocal(*pkg);
}

Node *
PrintFormat::parse_variable(const string &varname) const throw(ExBasic)
{
	VarParserCache::iterator f(varcache.find(varname));
	if(f == varcache.end()) {
		return varcache[varname].init((*eix_rc)[varname].c_str(), !no_color, true);
	}
	VarParserCacheNode &v(f->second);
	if(unlikely(v.in_use)) {
		throw ExBasic(_("Variable %r calls itself for printing"))
			% varname;
	}
	v.in_use = true;
	return v.m_parser.rootnode();
}

string
PrintFormat::overlay_keytext(ExtendedVersion::Overlay overlay, bool plain) const
{
	string start("[");
	string end("]");
	bool color(!no_color);
	if(plain)
		color = false;
	if(color) {
		if(is_virtual(overlay))
			start = color_virtualkey + start;
		else
			start = color_overlaykey + start;
		end += AnsiColor(AnsiColor::acDefault).asString();
	}
	if(overlay) {
		vector<ExtendedVersion::Overlay>::size_type index(overlay - 1);
		if(overlay_used)
			(*overlay_used)[index] = true;
		if(some_overlay_used)
			*some_overlay_used = true;
		if(overlay_translations) {
			overlay = (*overlay_translations)[index];
			if(!overlay) {
				ExtendedVersion::Overlay number(0);
				for(vector<ExtendedVersion::Overlay>::iterator it(overlay_translations->begin());
					likely(it != overlay_translations->end()); ++it) {
					if(number < *it)
						number = *it;
				}
				(*overlay_translations)[index] = ++number;
				overlay = number;
			}
		}
	}
	if(plain)
		return eix::format("%s") % overlay;
	return eix::format("%s%s%s") % start % overlay % end;
}

void
PrintFormat::setupResources(EixRc &rc)
{
	eix_rc = &rc;
	color_overlaykey = rc["COLOR_OVERLAYKEY"];
	color_virtualkey = rc["COLOR_VIRTUALKEY"];

	before_keywords  = rc["FORMAT_BEFORE_KEYWORDS"];
	after_keywords   = rc["FORMAT_AFTER_KEYWORDS"];
	print_effective  = rc.getBool("PRINT_EFFECTIVE_KEYWORDS");
	before_ekeywords = rc["FORMAT_BEFORE_EFFECTIVE_KEYWORDS"];
	after_ekeywords  = rc["FORMAT_AFTER_EFFECTIVE_KEYWORDS"];
	alpha_use        = rc.getBool("SORT_INST_USE_ALPHA");
	before_set_use   = rc["FORMAT_BEFORE_SET_USE"];
	after_set_use    = rc["FORMAT_AFTER_SET_USE"];
	before_unset_use = rc["FORMAT_BEFORE_UNSET_USE"];
	after_unset_use  = rc["FORMAT_AFTER_UNSET_USE"];

	magic_newline    = rc.getBool("NEWLINE");
}

void
PrintFormat::setupColors()
{
	bool use_color(!no_color);
	if(use_color) {
		color_overlaykey = AnsiColor(color_overlaykey).asString();
		color_virtualkey = AnsiColor(color_virtualkey).asString();
	}
	before_keywords    = parse_colors(before_keywords, use_color);
	after_keywords     = parse_colors(after_keywords, use_color);
	before_ekeywords   = parse_colors(before_ekeywords, use_color);
	after_ekeywords    = parse_colors(after_ekeywords, use_color);
	before_set_use     = parse_colors(before_set_use, use_color);
	after_set_use      = parse_colors(after_set_use, use_color);
	before_unset_use   = parse_colors(before_unset_use, use_color);
	after_unset_use    = parse_colors(after_unset_use, use_color);
}

bool
PrintFormat::recPrint(string *result, const void *entity, GetProperty get_property, Node *root) const
{
	bool printed(false);
	for(; likely(root != NULL); root = root->next) {
		switch(root->type) {
			case Node::TEXT: /* text!! */
				{
					const string &t(static_cast<Text*>(root)->text);
					if(!t.empty()) {
						printed = true;
						if(result)
							result->append(t);
						else
							cout << t;
					}
				}
				break;
			case Node::OUTPUT:
				try {
					Property *p(static_cast<Property*>(root));
					string s;
					if(p->user_variable)
						s = user_variables[p->name];
					else {
						try {
							s = get_property(this, entity, p->name);
						}
						catch(const ExBasic &e) {
							cerr << e << endl;
							s.clear();
						}
					}
					if(!s.empty()) {
						printed = true;
						if(result)
							result->append(s);
						else
							cout << s;
					}
				}
				catch(const ExBasic &e) {
					cerr << e << endl;
				}
				break;
			default:
			//case Node::IF:
			//case Node::SET:
				{
					ConditionBlock *ief = static_cast<ConditionBlock*>(root);
					string rhs;
					switch(ief->rhs) {
						case ConditionBlock::RHS_VAR:
							rhs = user_variables[ief->text.text];
							break;
						case ConditionBlock::RHS_PROPERTY:
							try {
								rhs = get_property(this, entity, ief->text.text);
							}
							catch(const ExBasic &e) {
								cerr << e << endl;
								rhs.clear();
							}
							break;
						default:
						//case ConditionBlock::RHS_STRING:
							rhs = ief->text.text;
							break;
					}
					if(root->type == Node::SET) {
						if(ief->negation) {
							if(rhs.empty())
								user_variables[ief->variable.name] = "1";
							else
								user_variables[ief->variable.name].clear();
						}
						else
							user_variables[ief->variable.name] = rhs;
						break;
					}
					// Node::IF:
					bool ok(false);
					if(ief->user_variable) {
						ok = (user_variables[ief->variable.name] == rhs);
					}
					else {
						try {
							ok = (get_property(this, entity, ief->variable.name) == rhs);
						}
						catch(const ExBasic &e) {
							cerr << e << endl;
							ok = rhs.empty();
						}
					}
					ok = ief->negation ? !ok : ok;
					if(ok && ief->if_true) {
						if(recPrint(result, entity, get_property, ief->if_true))
							printed = true;
					}
					else if(!ok && ief->if_false) {
						if(recPrint(result, entity, get_property, ief->if_false))
							printed = true;
					}
				}
				break;
		}
	}
	return printed;
}

bool
PrintFormat::print(void *entity, GetProperty get_property, Node *root, const DBHeader *dbheader, VarDbPkg *vardbpkg, const PortageSettings *ps, const SetStability *s)
{
	// The four hackish variables
	header = dbheader; vardb = vardbpkg; portagesettings = ps; stability = s;
	version_variables = NULL;
	varcache.clear_use();
	user_variables.clear();
	bool r(recPrint(NULL, entity, get_property, root));
	// Reset the four hackish variables
	header = NULL; vardb = NULL; portagesettings = NULL; stability = NULL;
	if(r && magic_newline)
		fputc('\n', stdout);
	return r;
}

string
parse_colors(const string &colorstring, bool colors)
{
	string ret;
	FormatParser parser;
	for(Node *root = parser.start(colorstring.c_str(), colors, true);
		root != NULL; root = root->next)
	{
		if(root->type != Node::TEXT)
			throw ExBasic(_("Internal error: bad node for parse_colors."));
		ret.append((static_cast<Text*>(root))->text);
	}
	return ret;
}

int
FormatParser::getPosition(int *line, int *column)
{
	const char *x(band), *y(band);
	while(x <= band_position && x) {
		y = x;
		x = strchr(x, '\n');
		if(x) {
			++x;
			++*line;
			*column = band_position - y;
		}
	}
	return band_position - band;
}


FormatParser::ParserState
FormatParser::state_START()
{
	switch(*band_position++) {
		case '\0':
			return STOP;
		case '{':
			if(only_colors)
				break;
			return IF;
		case '<':
			if(only_colors)
				break;
			return PROPERTY;
		case '(':
			return COLOR;
		default:
			break;
	}
	--band_position;
	return TEXT;
}

FormatParser::ParserState
FormatParser::state_TEXT()
{
	string textbuffer;
	const char *end_of_text("<{(");
	if(only_colors)
		end_of_text = "(";
	while(*band_position && (strchr(end_of_text, *band_position ) == NULL)) {
		if(*band_position == '\\') {
			textbuffer.append(1, get_escape(*(++band_position)));
		}
		else {
			textbuffer.append(band_position, 1);
		}
		++band_position;
	}
	keller.push(new Text(textbuffer));
	return START;
}

FormatParser::ParserState
FormatParser::state_COLOR()
{
	const char *q(strchr(band_position, ')'));
	if(q == NULL) {
		last_error = _("'(' without closing ')'");
		return ERROR;
	}
	if(enable_colors) {
		try {
			keller.push(new Text(AnsiColor(string(band_position, q - band_position)).asString()));
		}
		catch(const ExBasic &e) {
			last_error = eix::format(_("Error while parsing color: %s")) % e;
			return ERROR;
		}
	}
	band_position = q + 1;
	return START;
}

inline static const char *seek_character(const char *fmt) ATTRIBUTE_PURE;
inline static const char *
seek_character(const char *fmt)
{
	while(*fmt && isspace(*fmt, localeC)) {
		++fmt;
	}
	return fmt;
}

FormatParser::ParserState
FormatParser::state_PROPERTY()
{
	const char *q(strchr(band_position, '>'));
	if(q == NULL) {
		last_error = _("'<' without closing '>'");
		return ERROR;
	}

	/* Look for variable */
	bool user_variable(false);
	if(*band_position == '$') {
		user_variable = true;
		++band_position;
		band_position = seek_character(band_position);
	}
	/*
	   If "magic" variables with ":" and color arguments should be used,
	   we should pass the following string argument through
	   "parse_colors(·, enable_colors)".
	   With the currently available attribute this is not necessary.
	*/
	keller.push(new Property(string(band_position, q - band_position), user_variable));
	band_position = q + 1;
	return START;
}

FormatParser::ParserState
FormatParser::state_IF()
{
	if(strncmp(band_position, "else}", 5) == 0) {
		band_position += 5;
		return ELSE;
	}
	if(strncmp(band_position, "}", 1) == 0) {
		band_position += 1;
		return FI;
	}

	ConditionBlock *n(new ConditionBlock);
	keller.push(n);

	/* Look for negation */
	band_position = seek_character(band_position);
	if(*band_position == '!') {
		n->negation = true;
		band_position = seek_character(band_position + 1);
	}
	else {
		n->negation = false;
	}

	/* Look for variable */
	switch(*band_position) {
		case '*':
			n->type = Node::SET;
			n->user_variable = true;
			band_position = seek_character(band_position + 1);
			break;
		case '$':
			n->user_variable = true;
			band_position = seek_character(band_position + 1);
			break;
		default:
			n->user_variable = false;
	}

	unsigned int i(0);
	const char *name_start(band_position);
	for(char c(*band_position);
		c && (c != '}') && (c != '=') && !(isspace(c));
		c = *(++band_position))
		++i;
	if(!i) {
		last_error = _("No name of property/variable found after '{'.");
		return ERROR;
	}
	if(!*band_position) {
		last_error = _("Found '{' without closing '}'.");
		return ERROR;
	}
	n->variable = Property(string(name_start, i));

	band_position = seek_character(band_position);
	if(*band_position == '}') {
		n->text = Text("");
		n->negation = !n->negation;
		n->rhs = ConditionBlock::RHS_STRING;
		++band_position;
		return START;
	}
	/* This MUST be a '=' */
	if(*band_position != '=') {
		last_error = eix::format(_("Unexpected symbol %r found after '{'.")) % (*band_position);
		return ERROR;
	}

	band_position = seek_character(band_position + 1);
	if(!*band_position) {
		last_error = _("Found '{' without closing '}'.");
		return ERROR;
	}

	static const char single_quote = '\'', double_quote = '"', plain = ' ', property = '>';
	char parse_modus;
	switch(*band_position) {
		case '<':
			n->rhs = ConditionBlock::RHS_PROPERTY;
			parse_modus = property;
			++band_position;
			band_position = seek_character(band_position);
			break;
		case '$':
			n->rhs = ConditionBlock::RHS_VAR;
			parse_modus = plain;
			++band_position;
			band_position = seek_character(band_position);
			break;
		case '\'':
			n->rhs = ConditionBlock::RHS_STRING;
			parse_modus = single_quote;
			++band_position;
			break;
		case '\"':
			n->rhs = ConditionBlock::RHS_STRING;
			parse_modus = double_quote;
			++band_position;
			break;
		default:
			n->rhs = ConditionBlock::RHS_STRING;
			parse_modus = plain;
			break;
	}

	string textbuffer;
	for(char c(*band_position); likely(c != '\0'); c = *(++band_position)) {
		if(parse_modus != plain) {
			if(c == parse_modus)
				break;
		}
		else if((c == '}') || isspace(c))
			break;
		if((c == '\\') && (parse_modus != single_quote)) {
			textbuffer.append(1, get_escape(*(++band_position)));
			if(!*band_position)
				break;
		}
		else {
			textbuffer.append(1, c);
		}
	}
	n->text = Text(textbuffer);

	if(*band_position != '}') {
		if(*band_position) {
			band_position = seek_character(band_position + 1);
		}
		if(*band_position != '}') {
			last_error = _("Found '{' without closing '}'.");
			return ERROR;
		}
	}
	++band_position;
	return START;
}

FormatParser::ParserState
FormatParser::state_ELSE()
{
	Node *p(NULL), *q(NULL);
	if(keller.empty()) {
		return START;
	}
	p = keller.top();
	keller.pop();
	while(p->type != Node::IF || (static_cast<ConditionBlock*>(p))->final == true) {
		p->next = q;
		q = p;
		if(keller.empty()) {
			last_error = _("Found ELSE without IF.");
			return ERROR;
		}
		p = keller.top();
		keller.pop();
	}
	if(q == NULL) {
		q = new Text("");
	}
	(static_cast<ConditionBlock*>(p))->if_true = q;
	keller.push(p);
	return START;
}

FormatParser::ParserState
FormatParser::state_FI()
{
	Node *p = NULL, *q = NULL;
	if(keller.empty()) {
		return START;
	}
	p = keller.top();
	keller.pop();
	while(p->type != Node::IF || (static_cast<ConditionBlock*>(p))->final == true) {
		p->next = q;
		q = p;
		if(keller.empty()) {
			last_error = _("Found FI without IF.");
			return ERROR;
		}
		p = keller.top();
		keller.pop();
	}
	if((static_cast<ConditionBlock*>(p))->if_true == NULL) {
		(static_cast<ConditionBlock*>(p))->if_true = q;
	}
	else {
		(static_cast<ConditionBlock*>(p))->if_false = q;
	}
	(static_cast<ConditionBlock*>(p))->final = true;
	keller.push(p);
	return START;
}

Node *
FormatParser::start(const char *fmt, bool colors, bool parse_only_colors) throw(ExBasic)
{
	/* Initialize machine */
	enable_colors = colors;
	only_colors = parse_only_colors;
	last_error = _("Check your syntax");
	state = START;
	band = fmt;
	band_position = fmt;
	/* Run machine */
	while(state != STOP && state != ERROR) {
		switch(state) {
			case START:    state = state_START(); break;
			case TEXT:     state = state_TEXT(); break;
			case COLOR:    state = state_COLOR(); break;
			case PROPERTY: state = state_PROPERTY(); break;
			case IF:       state = state_IF(); break;
			case ELSE:     state = state_ELSE(); break;
			case FI:       state = state_FI(); break;
			case ERROR:    throw ExBasic(_("Bad state: ERROR"));
			case STOP:     throw ExBasic(_("Bad state: STOP"));
			default:       throw ExBasic(_("Bad state: undefined"));
		}
	}
	/* Check if the machine went into ERROR-state. */
	if(state == ERROR) {
		/* Clean stacks. */
		while(!keller.empty()) {
			delete keller.top();
			keller.pop();
		}
		int line(0), column(0);
		getPosition(&line, &column);
		throw ExBasic(_("Line %r, column %r: %s")) % line % column % last_error;
	}
	/* Pop elements and form a single linked list. */
	Node *p(NULL), *q(NULL);
	while(!keller.empty()) {
		p = keller.top();
		keller.pop();
		p->next = q;
		q = p;
	}
	root_node = p;
	/* Return root-node. */
	return root_node;
}
