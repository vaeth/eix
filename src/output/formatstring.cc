// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include "formatstring.h"

#include <portage/conf/portagesettings.h>
#include <eixrc/global.h>

using namespace std;

string
get_escape(const char *p)
{
	switch(*p) {
		case '\\': return string("\\");
		case 'n':  return string("\n");
		case 'r':  return string("\r");
		case 't':  return string("\t");
		case 'b':  return string("\b");
		case 'a':  return string("\a");
		default:
			break;
	}
	return string(p, 1);
}

void MarkedList::add(const char *pkg, const char *ver)
{
	pair<string, BasicVersion*> p;
	p.first = string(pkg);
	if(ver)
		p.second = new BasicVersion(ver);
	else
		p.second = NULL;
	insert(p);
}

inline
MarkedList::CIPair MarkedList::equal_range_pkg(const Package &pkg) const
{
	return equal_range(pkg.category + "/" + pkg.name);
}

/** Return pointer to (newly allocated) sorted vector of marked versions,
    or NULL. With nonversion argument, its content will decide whether
    the package was marked with a non-version argument */
vector<BasicVersion> *MarkedList::get_marked_vector(const Package &pkg, bool *nonversion) const
{
	CIPair beg_end = equal_range_pkg(pkg);
	if(nonversion)
		*nonversion = false;
	if((beg_end.first == end()) || (beg_end.first == beg_end.second))// no match
		return NULL;
	vector<BasicVersion> *ret = NULL;
	for(const_iterator it = beg_end.first ; it != beg_end.second; ++it)
	{
		BasicVersion *p = it->second;
		if(!p)
		{
			if(nonversion)
				*nonversion = true;
			continue;
		}
		if(ret)
			ret->push_back(*p);
		else
			ret = new vector<BasicVersion>(1,*p);
	}
	if(!ret)// No version was explicitly marked
		return NULL;
	sort_uniquify(*ret);
	return ret;
}

/** Return true if pkg is marked. If ver is non-NULL also *ver must match */
bool MarkedList::is_marked(const Package &pkg, const BasicVersion *ver) const
{
	CIPair beg_end = equal_range_pkg(pkg);
	if((beg_end.first == end()) || (beg_end.first == beg_end.second))// no match
		return false;
	if(!ver)	// do not care about versions
		return true;
	for(const_iterator it = beg_end.first ; it != beg_end.second; ++it )
	{
		BasicVersion *p = it->second;
		if(p)
		{
			if(*p == *ver)
				return true;
		}
	}
	return false;
}

/** Return String of marked versions (sorted) */
string MarkedList::getMarkedString(const Package &pkg) const
{
	bool nonversion;
	vector<BasicVersion> *marked = get_marked_vector(pkg, &nonversion);
	if(!marked)
		return nonversion ? "*" : "";
	string ret;
	if(nonversion)
		ret = "*";
	for(vector<BasicVersion>::const_iterator it = marked->begin();
		it != marked->end(); ++it )
	{
		if(!ret.empty())
			ret.append(" ");
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
	VarParserCache::iterator f = varcache.find(varname);
	if(f == varcache.end()) {
		EixRc &rc = get_eixrc(NULL);
		return varcache[varname].init(rc[varname].c_str(), !no_color, true);
	}
	VarParserCacheNode &v = f->second;
	if(v.in_use) {
		throw ExBasic(_("Variable %r calls itself for printing"))
			% varname;
	}
	v.in_use = true;
	return v.m_parser.rootnode();
}

string
PrintFormat::overlay_keytext(Version::Overlay overlay, bool never_color) const
{
	string start = "[";
	string end = "]";
	bool color = !no_color;
	if(never_color)
		color = false;
	if(color) {
		if(is_virtual(overlay))
			start = color_virtualkey + start;
		else
			start = color_overlaykey + start;
		end += AnsiColor(AnsiColor::acDefault).asString();
	}
	if(overlay) {
		vector<Version::Overlay>::size_type index = overlay - 1;
		if(overlay_used)
			(*overlay_used)[index] = true;
		if(some_overlay_used)
			*some_overlay_used = true;
		if(overlay_translations) {
			overlay = (*overlay_translations)[index];
			if(!overlay) {
				Version::Overlay number = 0;
				for(vector<Version::Overlay>::iterator it = overlay_translations->begin();
					it != overlay_translations->end(); ++it) {
					if(number < *it)
						number = *it;
				}
				(*overlay_translations)[index] = ++number;
				overlay = number;
			}
		}
	}
	return eix::format("%s%s%s") % start % overlay % end;
}

bool
PrintFormat::recPrint(string *result, const void *entity, GetProperty get_property, Node *root) const
{
	bool printed = false;
	for(;
		root != NULL;
		root = root->next)
	{
		switch(root->type) {
			case Node::TEXT: /* text!! */
				{
					const string &t = static_cast<Text*>(root)->text;
					if(!t.empty()) {
						printed = true;
						if(result)
							result->append(t);
						else
							cout << t;
					}
				}
				break;
			case Node::VARIABLE:
				try {
					const string s = get_property(this, entity, (static_cast<Property*>(root))->name);
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
			case Node::IF:
				{
					ConditionBlock *ief = static_cast<ConditionBlock*>(root);
					bool ok = false;
					try {
						ok = (get_property(this, entity, ief->variable.name) == ief->text.text);
					}
					catch(const ExBasic &e) {
						cerr << e << endl;
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
			default:
				break;
		}
	}
	return printed;
}

bool
PrintFormat::print(void *entity, GetProperty get_property, Node *root, DBHeader *dbheader, VarDbPkg *vardbpkg, PortageSettings *ps, const SetStability *s)
{
	// The four hackish variables
	header = dbheader; vardb = vardbpkg; portagesettings = ps; stability = s;
	version_variables = NULL;
	varcache.clear_use();
	version_variables = NULL;
	bool r = recPrint(NULL, entity, get_property, root);
	// Reset the four hackish variables
	header = NULL; vardb = NULL; portagesettings = NULL; stability = NULL;
	if(r)
		fputc('\n', stdout);
	return r;
}

string parse_colors(const string &colorstring, bool colors)
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
	const char *end_of_text = "<{(";
	if(only_colors)
		end_of_text = "(";
	while(*band_position && (strchr(end_of_text, *band_position ) == NULL)) {
		if(*band_position == '\\') {
			textbuffer.append(get_escape(++band_position));
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
	const char *q = strchr(band_position, ')');
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

FormatParser::ParserState
FormatParser::state_PROPERTY()
{
	const char *q = strchr(band_position, '>');
	if(q == NULL) {
		last_error = _("'<' without closing '>'");
		return ERROR;
	}
	keller.push(new Property(parse_colors(
		string(band_position, q - band_position), enable_colors)));
	band_position = q + 1;
	return START;
}

const char *
seek_character(const char *fmt)
{
	while(*fmt && isspace(*fmt)) {
		++fmt;
	}
	return fmt;
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

	ConditionBlock *n = new ConditionBlock;
	keller.push(n);

	/* Look for negation */
	band_position = seek_character(band_position);
	if(*band_position == '\0' || *band_position == '}') {
		last_error = _("Ran into end-of-string or '}' while looking for possible negation-mark (!) in condition.");
		return ERROR;
	}
	if(*band_position == '!') {
		n->negation = true;
		++band_position;
	}
	else {
		n->negation = false;
	}

	band_position = seek_character(band_position);
	if(*band_position == '\0' || *band_position == '}') {
		last_error = _("Ran into end-of-string or '}' while looking for property-name in condition.");
		return ERROR;
	}
	unsigned int i = 0;
	while(*band_position && strchr(" \t\n\r}", *band_position) == NULL) {
		++band_position;
		++i;
	}
	if(i == 0 || !*band_position) {
		last_error = _("Ran into end-of-string while reading property-name.");
		return ERROR;
	}
	n->variable = Property(string(band_position - i, i));

	band_position = seek_character(band_position);
	if(*band_position == '}') {
		n->text = Text("");
		n->negation = !n->negation;
		++band_position;
		return START;
	}
	/* This MUST be a '=' */
	if(*band_position != '=') {
		last_error = _("Unknown operator in if-construct.");
		return ERROR;
	}
	++band_position;

	band_position = seek_character(band_position);
	if(!*band_position) {
		last_error = _("Run into end-of-string while looking for right-hand of condition.");
		return ERROR;
	}

	string textbuffer;
	static const char single_quote = '\'', double_quote = '"', plain = ' ';
	char parse_modus = plain;

	if(*band_position == double_quote) {
		parse_modus = double_quote;
		++band_position;
	}
	else if(*band_position == single_quote) {
		parse_modus = single_quote;
		++band_position;
	}

	while(*band_position) {
		if((parse_modus != plain && *band_position == parse_modus) || strchr(" \t\n\r}", *band_position) != NULL) {
			break;
		}
		if(*band_position == '\\' && parse_modus != single_quote) {
			textbuffer.append(get_escape(++band_position));
		}
		else {
			textbuffer.append(band_position, 1);
		}
		++band_position;
	}
	n->text = Text(textbuffer);
	++band_position;
	band_position = seek_character(band_position);
	++band_position;
	return START;
}

FormatParser::ParserState
FormatParser::state_ELSE()
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
		while(keller.size() > 0) {
			delete keller.top();
			keller.pop();
		}
		int line = 0, column = 0;
		getPosition(&line, &column);
		throw ExBasic(_("Line %r, column %r: %s")) % line % column % last_error;
	}
	/* Pop elements and form a single linked list. */
	Node *p = NULL, *q = NULL;
	while(keller.size() != 0) {
		p = keller.top();
		keller.pop();
		p->next = q;
		q = p;
	}
	root_node = p;
	/* Return root-node. */
	return root_node;
}
