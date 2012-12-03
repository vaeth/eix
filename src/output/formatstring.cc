// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include <config.h>

#include <sys/types.h>

#include <cstdlib>
#include <cstring>

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "eixTk/ansicolor.h"
#include "eixTk/diagnostics.h"
#include "eixTk/eixint.h"
#include "eixTk/formated.h"
#include "eixTk/i18n.h"
#include "eixTk/likely.h"
#include "eixTk/null.h"
#include "eixTk/regexp.h"
#include "eixTk/stringutils.h"
#include "eixrc/eixrc.h"
#include "output/formatstring.h"
#include "portage/basicversion.h"
#include "portage/extendedversion.h"

class PortageSettings;

using std::map;
using std::pair;
using std::set;
using std::string;
using std::vector;

using std::cerr;
using std::cout;
using std::endl;

static void parse_color(string *color, bool use_color) ATTRIBUTE_NONNULL_;
static void colorstring(string *color) ATTRIBUTE_NONNULL_;
static bool parse_colors(string *ret, const string &colorstring, bool colors, string *errtext) ATTRIBUTE_NONNULL_;

void
MarkedList::add(const char *pkg, const char *ver)
{
	pair<string, BasicVersion*> p;
	p.first = string(pkg);
	BasicVersion *&basic_version(p.second);
	if(ver != NULLPTR) {
		basic_version = new BasicVersion;
		if(unlikely(basic_version->parseVersion(ver, NULLPTR) == BasicVersion::parsedError)) {
			delete basic_version;
			basic_version = NULLPTR;
		}
	} else {
		basic_version = NULLPTR;
	}
	insert(p);
}

inline MarkedList::CIPair
MarkedList::equal_range_pkg(const Package &pkg) const
{
	return equal_range(pkg.category + "/" + pkg.name);
}

/** Return pointer to (newly allocated) sorted vector of marked versions,
    or NULLPTR. With nonversion argument, its content will decide whether
    the package was marked with a non-version argument */
set<BasicVersion> *
MarkedList::get_marked_versions(const Package &pkg, bool *nonversion) const
{
	CIPair beg_end(equal_range_pkg(pkg));
	if(nonversion != NULLPTR)
		*nonversion = false;
	if(likely((beg_end.first == end()) || (beg_end.first == beg_end.second))) {
		// no match
		return NULLPTR;
	}
	set<BasicVersion> *ret(NULLPTR);
	for(const_iterator it(beg_end.first); likely(it != beg_end.second); ++it) {
		BasicVersion *p(it->second);
		if(p == NULLPTR) {
			if(nonversion != NULLPTR)
				*nonversion = true;
			continue;
		}
		if(ret == NULLPTR)
			ret = new set<BasicVersion>;
		ret->insert(*p);
	}
	if(likely(ret == NULLPTR))  // No version was explicitly marked
		return NULLPTR;
	return ret;
}

/** Return true if pkg is marked. If ver is non-NULLPTR also *ver must match */
bool
MarkedList::is_marked(const Package &pkg, const BasicVersion *ver) const
{
	CIPair beg_end(equal_range_pkg(pkg));
	if((beg_end.first == end()) || (beg_end.first == beg_end.second)) {
		// no match
		return false;
	}
	if(ver == NULLPTR)  // do not care about versions
		return true;
	for(const_iterator it(beg_end.first); likely(it != beg_end.second); ++it ) {
		BasicVersion *p(it->second);
		if(p != NULLPTR) {
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
	if(marked == NULLPTR)
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
	PackageSave((fmt->recommend_mode) == LOCALMODE_DEFAULT ? NULLPTR : pkg)
{
	if((fmt->recommend_mode) == LOCALMODE_DEFAULT)
		return;
	if(fmt->recommend_mode == LOCALMODE_LOCAL) {
		fmt->StabilityLocal(pkg);
		return;
	}
	// (fmt->recommend_mode == LOCALMODE_NONLOCAL)
	fmt->StabilityNonlocal(pkg);
}

bool
VarParserCacheNode::init(Node *&rootnode, const char *fmt, bool colors, bool use, string *errtext)
{
	in_use = use;
	if(likely(m_parser.start(fmt, colors, false, errtext))) {
		rootnode = m_parser.rootnode();
		return true;
	}
	return false;
}

void
VarParserCache::clear_use()
{
	for(map<string, VarParserCacheNode>::iterator it = begin();
		it != end(); ++it)
		it->second.in_use = false;
}

PrintFormat::PrintFormat(GetProperty get_callback)
{
	m_get_property = get_callback;
	virtuals = NULLPTR;
	overlay_translations = NULLPTR;
	overlay_used = NULLPTR;
	some_overlay_used = NULLPTR;
	marked_list = NULLPTR;
	eix_rc = NULLPTR;
	vardb = NULLPTR;
	portagesettings = NULLPTR;
	stability = NULLPTR;
}

bool
PrintFormat::parse_variable(Node **rootnode, const string &varname, string *errtext) const
{
	VarParserCache::iterator f(varcache.find(varname));
	if(f == varcache.end()) {
		return varcache[varname].init(*rootnode, (*eix_rc)[varname].c_str(), !no_color, true, errtext);
	}
	VarParserCacheNode &v(f->second);
	if(unlikely(v.in_use)) {
		if(errtext != NULLPTR) {
			*errtext = eix::format(_("Variable %r calls itself for printing")) % varname;
		}
		return false;
	}
	v.in_use = true;
	*rootnode = v.m_parser.rootnode();
	return true;
}

Node *
PrintFormat::parse_variable(const string &varname) const
{
	string errtext;
	Node *rootnode;
	if(unlikely(!parse_variable(&rootnode, varname, &errtext))) {
		cerr << errtext << endl;
		exit(EXIT_FAILURE);
	}
	return rootnode;
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
		end.append(color_keyend);
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
PrintFormat::setupResources(EixRc *rc)
{
	eix_rc = rc;
	color_overlaykey     = (*rc)["COLOR_OVERLAYKEY"];
	color_virtualkey     = (*rc)["COLOR_VIRTUALKEY"];
	color_keyend         = (*rc)["COLOR_KEYEND"];
	color_overlayname    = (*rc)["COLOR_OVERLAYNAME"];
	color_overlaynameend = (*rc)["COLOR_OVERLAYNAMEEND"];
	color_numbertext     = (*rc)["COLOR_NUMBERTEXT"];
	color_numbertextend  = (*rc)["COLOR_NUMBERTEXTEND"];
	color_end.clear();

	alpha_use         = rc->getBool("SORT_INST_USE_ALPHA");
	before_use_start  = (*rc)["FORMAT_BEFORE_USE_EXPAND_START"];
	before_use_end    = (*rc)["FORMAT_BEFORE_USE_EXPAND_END"];
	after_use         = (*rc)["FORMAT_AFTER_USE_EXPAND"];
	before_iuse_start = (*rc)["FORMAT_BEFORE_IUSE_EXPAND_START"];
	before_iuse_end   = (*rc)["FORMAT_BEFORE_IUSE_EXPAND_END"];
	after_iuse        = (*rc)["FORMAT_AFTER_IUSE_EXPAND"];
	before_coll_start = (*rc)["FORMAT_BEFORE_COLL_EXPAND_START"];
	before_coll_end   = (*rc)["FORMAT_BEFORE_COLL_EXPAND_END"];
	after_coll        = (*rc)["FORMAT_AFTER_COLL_EXPAND"];
	before_set_use    = (*rc)["FORMAT_BEFORE_SET_USE"];
	after_set_use     = (*rc)["FORMAT_AFTER_SET_USE"];
	before_unset_use  = (*rc)["FORMAT_BEFORE_UNSET_USE"];
	after_unset_use   = (*rc)["FORMAT_AFTER_UNSET_USE"];
	maskreasons_skip  = (*rc)["FORMAT_MASKREASONS_LINESKIP"];
	maskreasons_sep   = (*rc)["FORMAT_MASKREASONS_SEP"];
	maskreasonss_skip = (*rc)["FORMAT_MASKREASONSS_LINESKIP"];
	maskreasonss_sep  = (*rc)["FORMAT_MASKREASONSS_SEP"];

	const char *term((*rc)["TERM"].c_str());
	char schemenum('0');
	for(char i('1'); likely(i != '4'); ++i) {
		if(RegexList((*rc)[string("TERM_ALT") + i]).match(term)) {
			schemenum = i;
			break;
		}
	}
	vector<string> schemes;
	split_string(&schemes, (*rc)[string("COLORSCHEME") + schemenum]);
	if(schemes.size() > 0) {
		eix::TinyUnsigned entry(0);
		if(schemes.size() > 1) {
			eix::SignedBool dark(rc->getBoolText("DARK", "auto"));
			if(dark == 0) {
				entry = 1;
			} else if(dark < 0) {
				if(!RegexList((*rc)["TERM_DARK"]).match(term)) {
					if(!RegexList((*rc)["COLORFGBG_DARK"]).match((*rc)["COLORFGBG"].c_str())) {
						entry = 1;
					}
				}
			}
		}
		AnsiColor::colorscheme = my_atoi(schemes[entry].c_str());
	}
}

static void
parse_color(string *color, bool use_color)
{
	string errtext;
	if(unlikely(!parse_colors(color, *color, use_color, &errtext))) {
		cerr << errtext << endl;
	}
}

static void
colorstring(string *color)
{
	string errtext;
	AnsiColor ac;
	if(likely(ac.initcolor(*color, &errtext))) {
		*color = ac.asString();
	} else {
		cerr << errtext << endl;
	}
}

void
PrintFormat::setupColors()
{
	bool use_color(!no_color);
	if(use_color) {
		colorstring(&color_overlaykey);
		colorstring(&color_virtualkey);
		colorstring(&color_keyend);
		colorstring(&color_overlayname);
		colorstring(&color_overlaynameend);
		colorstring(&color_numbertext);
		colorstring(&color_numbertextend);
		colorstring(&color_end);
	}
	parse_color(&before_use_start, use_color);
	parse_color(&before_use_end, use_color);
	parse_color(&after_use, use_color);
	parse_color(&before_iuse_start, use_color);
	parse_color(&before_iuse_end, use_color);
	parse_color(&after_iuse, use_color);
	parse_color(&before_coll_start, use_color);
	parse_color(&before_coll_end, use_color);
	parse_color(&after_coll, use_color);
	parse_color(&before_set_use, use_color);
	parse_color(&after_set_use, use_color);
	parse_color(&before_unset_use, use_color);
	parse_color(&after_unset_use, use_color);
	parse_color(&maskreasons_skip, use_color);
	parse_color(&maskreasons_sep, use_color);
	parse_color(&maskreasonss_skip, use_color);
	parse_color(&maskreasonss_sep, use_color);
}

bool
PrintFormat::recPrint(string *result, void *entity, GetProperty get_property, Node *root) const
{
	bool printed(false);
	for(; likely(root != NULLPTR); root = root->next) {
		switch(root->type) {
			case Node::TEXT: /* text!! */
				{
					const string &t(static_cast<Text*>(root)->text);
					if(!t.empty()) {
						printed = true;
						if(result != NULLPTR)
							result->append(t);
						else
							cout << t;
					}
				}
				break;
			case Node::OUTPUT:
				{
					Property *p(static_cast<Property*>(root));
					string s;
					if(p->user_variable) {
						s = user_variables[p->name];
					} else {
						s = get_property(this, entity, p->name);
					}
					if(!s.empty()) {
						printed = true;
						if(result)
							result->append(s);
						else
							cout << s;
					}
				}
				break;
			default:
			// case Node::IF:
			// case Node::SET:
				{
					ConditionBlock *ief(static_cast<ConditionBlock*>(root));
					string rhs;
					switch(ief->rhs) {
						case ConditionBlock::RHS_VAR:
							rhs = user_variables[ief->text.text];
							break;
						case ConditionBlock::RHS_PROPERTY:
							rhs = get_property(this, entity, ief->text.text);
							break;
						default:
						// case ConditionBlock::RHS_STRING:
							rhs = ief->text.text;
							break;
					}
					if(root->type == Node::SET) {
						if(ief->negation) {
							if(rhs.empty())
								user_variables[ief->variable.name] = "1";
							else
								user_variables[ief->variable.name].clear();
						} else {
							user_variables[ief->variable.name] = rhs;
						}
						break;
					}
					// Node::IF:
					bool ok(false);
					if(ief->user_variable) {
						ok = (user_variables[ief->variable.name] == rhs);
					} else {
						ok = (get_property(this, entity, ief->variable.name) == rhs);
					}
					ok = ief->negation ? !ok : ok;
					if(ok && ief->if_true) {
						if(recPrint(result, entity, get_property, ief->if_true))
							printed = true;
					} else if(!ok && ief->if_false) {
						if(recPrint(result, entity, get_property, ief->if_false))
							printed = true;
					}
				}
				break;
		}
	}
	return printed;
}

static bool
parse_colors(string *ret, const string &colorstring, bool colors, string *errtext)
{
	FormatParser parser;
	if(unlikely(!parser.start(colorstring.c_str(), colors, true, errtext))) {
		return false;
	}
	ret->clear();
	Node *root(parser.rootnode());
	if(root == NULLPTR) {
		return true;
	}
	while(likely(root->type == Node::TEXT)) {
		ret->append((static_cast<Text*>(root))->text);
		if((root = root->next) == NULLPTR) {
			return true;
		}
	}
	if(errtext != NULLPTR) {
		*errtext = _("Internal error: bad node for parse_colors.");
	}
	return false;
}

void
FormatParser::getPosition(size_t *line, size_t *column)
{
	const char *x(band), *y(band);
	while((x != NULLPTR) && (x <= band_position)) {
		y = x;
		x = strchr(x, '\n');
		if(x != NULLPTR) {
			++x;
			++*line;
GCC_DIAG_OFF(sign-conversion)
			*column = band_position - y;
GCC_DIAG_ON(sign-conversion)
		}
	}
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
	while(*band_position && (strchr(end_of_text, *band_position ) == NULLPTR)) {
		if(*band_position == '\\') {
			textbuffer.append(1, get_escape(*(++band_position)));
		} else {
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
	if(q == NULLPTR) {
		last_error = _("'(' without closing ')'");
		return ERROR;
	}
	if(enable_colors) {
		AnsiColor ac;
		string errtext;
GCC_DIAG_OFF(sign-conversion)
		if(unlikely(!ac.initcolor(string(band_position, q - band_position), &errtext))) {
GCC_DIAG_ON(sign-conversion)
			last_error = eix::format(_("Error while parsing color: %s")) % errtext;
			return ERROR;
		}
		keller.push(new Text(ac.asString()));
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
	if(q == NULLPTR) {
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
GCC_DIAG_OFF(sign-conversion)
	keller.push(new Property(string(band_position, q - band_position), user_variable));
GCC_DIAG_ON(sign-conversion)
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
	} else {
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

	size_t i(0);
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
		} else if((c == '}') || isspace(c)) {
			break;
		}
		if((c == '\\') && (parse_modus != single_quote)) {
			textbuffer.append(1, get_escape(*(++band_position)));
			if(!*band_position)
				break;
		} else {
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
	Node *p(NULLPTR), *q(NULLPTR);
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
	if(q == NULLPTR) {
		q = new Text("");
	}
	(static_cast<ConditionBlock*>(p))->if_true = q;
	keller.push(p);
	return START;
}

FormatParser::ParserState
FormatParser::state_FI()
{
	Node *p = NULLPTR, *q = NULLPTR;
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
	if((static_cast<ConditionBlock*>(p))->if_true == NULLPTR) {
		(static_cast<ConditionBlock*>(p))->if_true = q;
	} else {
		(static_cast<ConditionBlock*>(p))->if_false = q;
	}
	(static_cast<ConditionBlock*>(p))->final = true;
	keller.push(p);
	return START;
}

bool
FormatParser::start(const char *fmt, bool colors, bool parse_only_colors, string *errtext)
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
			case START:    state = state_START();
			               break;
			case TEXT:     state = state_TEXT();
			               break;
			case COLOR:    state = state_COLOR();
			               break;
			case PROPERTY: state = state_PROPERTY();
			               break;
			case IF:       state = state_IF();
			               break;
			case ELSE:     state = state_ELSE();
			               break;
			case FI:       state = state_FI();
			//             break;
			case ERROR:    // break;
			case STOP:     break;
			default:       last_error = _("Bad state: undefined");
			               state = ERROR;
		}
	}
	/* Check if the machine went into ERROR-state. */
	if(state == ERROR) {
		/* Clean stacks. */
		while(!keller.empty()) {
			delete keller.top();
			keller.pop();
		}
		if(errtext != NULLPTR) {
			size_t line(0), column(0);
			getPosition(&line, &column);
			*errtext = eix::format(_("Line %s, column %s: %s")) % line % column % last_error;
		}
		return false;
	}
	/* Pop elements and form a single linked list. */
	Node *p(NULLPTR), *q(NULLPTR);
	while(!keller.empty()) {
		p = keller.top();
		keller.pop();
		p->next = q;
		q = p;
	}
	root_node = p;
	return true;
}

/* return true if something was actually printed */
bool
PrintFormat::print(void *entity, GetProperty get_property, Node *root, const DBHeader *dbheader, VarDbPkg *vardbpkg, const PortageSettings *ps, const SetStability *s)
{
	// The four hackish variables
	header = dbheader;
	vardb = vardbpkg;
	portagesettings = ps;
	stability = s;
	version_variables = NULLPTR;
	varcache.clear_use();
	user_variables.clear();
	bool r(recPrint(NULLPTR, entity, get_property, root));
	// Reset the four hackish variables
	header = NULLPTR;
	vardb = NULLPTR;
	portagesettings = NULLPTR;
	stability = NULLPTR;
	return r;
}

bool
PrintFormat::parseFormat(Node **rootnode, const char *fmt, std::string *errtext)
{
	if(likely(m_parser.start(fmt, !no_color, false, errtext))) {
		*rootnode = m_parser.rootnode();
		return true;
	}
	return false;
}
