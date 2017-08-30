// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#include "output/formatstring.h"
#include <config.h>

#include <sys/types.h>

#include <cstdlib>
#include <cstring>

#include <string>
#include <vector>

#include "eixTk/ansicolor.h"
#include "eixTk/attribute.h"
#include "eixTk/diagnostics.h"
#include "eixTk/dialect.h"
#include "eixTk/eixint.h"
#include "eixTk/formated.h"
#include "eixTk/i18n.h"
#include "eixTk/likely.h"
#include "eixTk/null.h"
#include "eixTk/outputstring.h"
#include "eixTk/regexp.h"
#include "eixTk/stringtypes.h"
#include "eixTk/stringutils.h"
#include "eixrc/eixrc.h"
#include "portage/extendedversion.h"

class PortageSettings;
class Darkmode;

using std::string;
using std::vector;

string::size_type PrintFormat::currcolumn = 0;

typedef vector<Darkmode> DarkModes;

ATTRIBUTE_NONNULL_ static void parse_color(OutputString *color, bool use_color);
ATTRIBUTE_NONNULL_ static void colorstring(string *color, bool use_color);
ATTRIBUTE_NONNULL_ static bool parse_colors(OutputString *ret, const string& colorstring, bool colors, string *errtext);
ATTRIBUTE_NONNULL_ static void parse_termdark(DarkModes *mode, WordVec *regexp, const string& termdark);
ATTRIBUTE_PURE inline static const char *seek_character(const char *fmt);

LocalCopy::LocalCopy(const PrintFormat *fmt, Package *pkg) :
	PackageSave((fmt->recommend_mode) == LOCALMODE_DEFAULT ? NULLPTR : pkg) {
	if((fmt->recommend_mode) == LOCALMODE_DEFAULT)
		return;
	if(fmt->recommend_mode == LOCALMODE_LOCAL) {
		fmt->StabilityLocal(pkg);
		return;
	}
	// (fmt->recommend_mode == LOCALMODE_NONLOCAL)
	fmt->StabilityNonlocal(pkg);
}

bool VarParserCacheNode::init(const char *fmt, bool colors, bool use, string *errtext) {
	in_use = use;
	FormatParser parser;
	if(likely(parser.start(fmt, colors, false, errtext))) {
		root_node = parser.rootnode();
		return true;
	}
	root_node = NULLPTR;
	return false;
}

void VarParserCache::clear_use() {
	for(VarParserCacheMap::iterator it(begin()); likely(it != end()); ++it) {
		it->second.in_use = false;
	}
}

void PrintFormat::init(GetProperty get_callback) {
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
	root_node = NULLPTR;
}

bool PrintFormat::parse_variable(Node **rootnode, const string& varname, string *errtext) const {
	VarParserCache::iterator f(varcache.find(varname));
	if(f == varcache.end()) {
		VarParserCacheNode& cache(varcache[varname]);
		bool ok(cache.init((*eix_rc)[varname].c_str(), !no_color, true, errtext));
		*rootnode = cache.rootnode();
		return ok;
	}
	VarParserCacheNode& v(f->second);
	if(unlikely(v.in_use)) {
		if(errtext != NULLPTR) {
			*errtext = eix::format(_("variable %s calls itself for printing")) % varname;
		}
		return false;
	}
	v.in_use = true;
	*rootnode = v.rootnode();
	return true;
}

Node *PrintFormat::parse_variable(const string& varname) const {
	string errtext;
	Node *rootnode;
	if(unlikely(!parse_variable(&rootnode, varname, &errtext))) {
		eix::say_error() % errtext;
		std::exit(EXIT_FAILURE);
	}
	return rootnode;
}

void PrintFormat::overlay_keytext(OutputString *s, ExtendedVersion::Overlay overlay, bool plain) const {
	ExtendedVersion::Overlay number(overlay);
	if(number != 0) {
		OverlayTranslations::size_type index(overlay - 1);
		if(overlay_used != NULLPTR) {
			(*overlay_used)[index] = true;
		}
		if(some_overlay_used != NULLPTR) {
			*some_overlay_used = true;
		}
		if(overlay_translations != NULLPTR) {
			if((number = (*overlay_translations)[index]) == 0) {
				for(OverlayTranslations::const_iterator it(overlay_translations->begin());
					likely(it != overlay_translations->end()); ++it) {
					if(number < *it) {
						number = *it;
					}
				}
				(*overlay_translations)[index] = ++number;
			}
		}
	}
	string onum(eix::format() % number);
	if(plain) {
		s->assign_fast(onum);
		return;
	}
	s->assign(((is_virtual(overlay)) ? color_virtualkey : color_overlaykey), 0);
	s->append_fast('[');
	s->append_fast(onum);
	s->append_fast(']');
	s->append(color_keyend, 0);
}

class Darkmode {
	public:
		bool dark, check;

		void init(bool is_dark, bool is_check) {
			dark = is_dark;
			check = is_check;
		}

		bool init(const string& s) {
			if(s == "true") {
				init(true, false);
			} else if(s == "true*") {
				init(true, true);
			} else if(s == "false") {
				init(false, false);
			} else if(s == "false*") {
				init(false, true);
			} else {
				return false;
			}
			return true;
		}

		Darkmode() {
		}

		Darkmode(bool is_dark, bool is_check) : dark(is_dark), check(is_check) {
		}
};

static void parse_termdark(DarkModes *modes, WordVec *regexp, const string& termdark) {
	WordVec terms_dark;
	split_string(&terms_dark, termdark, true);
	for(WordVec::const_iterator it(terms_dark.begin());
		likely(it != terms_dark.end()); ++it) {
		const string *text(&(*it));
		bool is_default(true);
		if(likely(++it != terms_dark.end())) {
			is_default = false;
			regexp->push_back(*text);
			text = &(*it);
		}
		Darkmode darkmode;
		if(!darkmode.init(*text)) {
			eix::say_error(_("DARK_TERM has illegal format: %s")) % termdark;
			std::exit(EXIT_FAILURE);
		}
		modes->push_back(darkmode);
		if(is_default) {
			return;
		}
	}
	modes->EMPLACE_BACK(Darkmode, (true, true));
}

void PrintFormat::setupResources(EixRc *rc) {
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
	char schemenum('3');
	for(; likely(schemenum != '0'); --schemenum) {
		if(RegexList((*rc)[string("TERM_ALT") + schemenum]).match(term)) {
			break;
		}
	}
	WordVec schemes;
	split_string(&schemes, (*rc)[string("COLORSCHEME") + schemenum]);
	if(schemes.size() > 0) {
		eix::TinyUnsigned entry(0);
		if(schemes.size() > 1) {
			eix::SignedBool dark(rc->getBoolText("DARK", "auto"));
			if(dark == 0) {
				entry = 1;
			} else if(dark < 0) {
				DarkModes modes;
				WordVec regexp;
				parse_termdark(&modes, &regexp, (*rc)["TERM_DARK"]);
				WordVec::size_type i(0);
				for(; likely(i < regexp.size()); ++i) {
					if(Regex(regexp[i].c_str()).match(term)) {
						break;
					}
				}
				const Darkmode& mode(modes[i]);
				bool is_dark(mode.dark);
				if(mode.check) {
					const string& colorfgbg((*rc)["COLORFGBG"]);
					if(!colorfgbg.empty()) {
						is_dark = RegexList((*rc)["COLORFGBG_DARK"]).match(colorfgbg.c_str());
					}
				}
				if(!is_dark) {
					entry = 1;
				}
			}
		}
		AnsiColor::colorscheme = my_atou(schemes[entry].c_str());
	}
}

static void parse_color(OutputString *color, bool use_color) {
	string errtext;
	if(unlikely(!parse_colors(color, color->as_string(), use_color, &errtext))) {
		eix::say_error() % errtext;
	}
}

static void colorstring(string *color, bool use_color) {
	if(!use_color) {
		color->clear();
		return;
	}
	string errtext;
	AnsiColor ac;
	if(likely(ac.initcolor(*color, &errtext))) {
		*color = ac.asString();
	} else {
		eix::say_error() % errtext;
	}
}

void PrintFormat::setupColors() {
	bool use_color(!no_color);
	colorstring(&color_overlaykey, use_color);
	colorstring(&color_virtualkey, use_color);
	colorstring(&color_keyend, use_color);
	colorstring(&color_overlayname, use_color);
	colorstring(&color_overlaynameend, use_color);
	colorstring(&color_numbertext, use_color);
	colorstring(&color_numbertextend, use_color);
	colorstring(&color_end, use_color);
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

bool PrintFormat::printString(OutputString *result, const OutputString& output) const {
	if(output.empty()) {
		return false;
	}
	if(result != NULLPTR) {
		result->append(output);
	} else {
		output.print(&currcolumn);
	}
	return true;
}

bool PrintFormat::recPrint(OutputString *result, void *entity, GetProperty get_property, Node *root) const {
	bool printed(false);
	for(; likely(root != NULLPTR); root = root->next) {
		switch(root->type) {
			case Node::TEXT:  /* text!! */
				if(printString(result, static_cast<Text*>(root)->text)) {
					printed = true;
				}
				break;
			case Node::OUTPUT: {
					Property *p(static_cast<Property*>(root));
					if(p->user_variable) {
						if(printString(result, user_variables[p->name])) {
							printed = true;
						}
					} else {
						OutputString s;
						get_property(&s, this, entity, p->name);
						if(printString(result, s)) {
							printed = true;
						}
					}
				}
				break;
			// case Node::IF:
			// case Node::SET:
			default: {
					ConditionBlock *ief(static_cast<ConditionBlock*>(root));
					OutputString  rhsvalue;
					OutputString *rhs;
					switch(ief->rhs) {
						case ConditionBlock::RHS_VAR:
							rhs = &(user_variables[ief->text.text.as_string()]);
							break;
						case ConditionBlock::RHS_PROPERTY:
							rhs = &rhsvalue;
							get_property(rhs, this, entity, ief->text.text.as_string());
							break;
						default:
						// case ConditionBlock::RHS_STRING:
							rhs = &ief->text.text;
							break;
					}
					if(root->type == Node::SET) {
						OutputString& r(user_variables[ief->variable.name]);
						if(ief->negation) {
							if(rhs->empty()) {
								r.set_one();
							} else {
								r.clear();
							}
						} else {
							r = *rhs;
						}
						break;
					}
					// Node::IF:
					bool ok;
					if(ief->user_variable) {
						ok = rhs->is_equal(user_variables[ief->variable.name]);
					} else {
						OutputString r;
						get_property(&r, this, entity, ief->variable.name);
						ok = rhs->is_equal(r);
					}
					if(ief->negation) {
						ok = !ok;
					}
					if(ok && ief->if_true) {
						if(recPrint(result, entity, get_property, ief->if_true)) {
							printed = true;
						}
					} else if((!ok) && ief->if_false) {
						if(recPrint(result, entity, get_property, ief->if_false)) {
							printed = true;
						}
					}
				}
				break;
		}
	}
	return printed;
}

static bool parse_colors(OutputString *ret, const string& colorstring, bool colors, string *errtext) {
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
			delete parser.rootnode();
			return true;
		}
	}
	delete parser.rootnode();
	*errtext = _("internal error: bad node for parse_colors");
	return false;
}

void FormatParser::getPosition(size_t *line, size_t *column) {
	const char *x(band), *y(band);
	while((x != NULLPTR) && (x <= band_position)) {
		y = x;
		x = std::strchr(x, '\n');
		if(x != NULLPTR) {
			++x;
			++*line;
GCC_DIAG_OFF(sign-conversion)
			*column = band_position - y;
GCC_DIAG_ON(sign-conversion)
		}
	}
}


FormatParser::ParserState FormatParser::state_START() {
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

FormatParser::ParserState FormatParser::state_TEXT() {
	OutputString textbuffer;
	const char *end_of_text(only_colors ? "(" : "<{(");
	while(*band_position && (std::strchr(end_of_text, *band_position) == NULLPTR)) {
		if(*band_position == '\\') {
			textbuffer.append_escape(&band_position);
		} else {
			textbuffer.append_smart(*band_position);
		}
		++band_position;
	}
	keller.push(new Text(textbuffer));
	return START;
}

FormatParser::ParserState FormatParser::state_COLOR() {
	const char *q(std::strchr(band_position, ')'));
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
			last_error = eix::format(_("error while parsing color: %s")) % errtext;
			return ERROR;
		}
		keller.push(new Text(ac.asString(), 0));
	}
	band_position = q + 1;
	return START;
}

inline static const char *seek_character(const char *fmt) {
	while((*fmt != '\0') && my_isspace(*fmt)) {
		++fmt;
	}
	return fmt;
}

FormatParser::ParserState FormatParser::state_PROPERTY() {
	const char *q(std::strchr(band_position, '>'));
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

FormatParser::ParserState FormatParser::state_IF() {
	if(std::strncmp(band_position, "else}", 5) == 0) {
		band_position += 5;
		return ELSE;
	}
	if(std::strncmp(band_position, "}", 1) == 0) {
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
		c && (c != '}') && (c != '=') && !(my_isspace(c));
		c = *(++band_position))
		++i;
	if(!i) {
		last_error = _("no name of property/variable after '{'");
		return ERROR;
	}
	if(!*band_position) {
		last_error = _("'{' without closing '}'");
		return ERROR;
	}
	n->variable = Property(string(name_start, i));

	band_position = seek_character(band_position);
	if(*band_position == '}') {
		n->text = Text();
		n->negation = !n->negation;
		n->rhs = ConditionBlock::RHS_STRING;
		++band_position;
		return START;
	}
	/* This MUST be a '=' */
	if(*band_position != '=') {
		last_error = eix::format(_("unexpected symbol '%s' after '{'")) % (*band_position);
		return ERROR;
	}

	band_position = seek_character(band_position + 1);
	if(!*band_position) {
		last_error = _("'{' without closing '}'");
		return ERROR;
	}

	static CONSTEXPR const char single_quote = '\'', double_quote = '"', plain = ' ', property = '>';
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

	OutputString textbuffer;
	for(char c(*band_position); likely(c != '\0'); c = *(++band_position)) {
		if(parse_modus != plain) {
			if(c == parse_modus)
				break;
		} else if((c == '}') || my_isspace(c)) {
			break;
		}
		if((c == '\\') && (parse_modus != single_quote)) {
			textbuffer.append_escape(&band_position);
			if(!*band_position) {
				break;
			}
		} else if((c == '(') && (parse_modus != single_quote) && (parse_modus != double_quote)) {
			const char *q(++band_position);
			while(isalnum(*q) || (std::strchr("|,;:", *q) != NULLPTR)) {
				++q;
			}
			bool is_color(false);
			if(likely(*q == ')')) {
				AnsiColor ac;
GCC_DIAG_OFF(sign-conversion)
				if(likely(ac.initcolor(string(band_position, q - band_position), NULLPTR))) {
GCC_DIAG_ON(sign-conversion)
					is_color = true;
					if(enable_colors) {
						textbuffer.append(ac.asString(), 0);
					}
				}
			}
			if(likely(is_color)) {
				band_position = q;
			} else {
				--band_position;
				textbuffer.append_fast('(');
			}
		} else {
			textbuffer.append_smart(c);
		}
	}
	n->text = Text(textbuffer);

	if(*band_position != '}') {
		if(*band_position) {
			band_position = seek_character(band_position + 1);
		}
		if(*band_position != '}') {
			last_error = _("'{' without closing '}'");
			return ERROR;
		}
	}
	++band_position;
	return START;
}

FormatParser::ParserState FormatParser::state_ELSE() {
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
			last_error = _("ELSE without IF");
			return ERROR;
		}
		p = keller.top();
		keller.pop();
	}
	if(q == NULLPTR) {
		q = new Text();
	}
	(static_cast<ConditionBlock*>(p))->if_true = q;
	keller.push(p);
	return START;
}

FormatParser::ParserState FormatParser::state_FI() {
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
			last_error = _("FI without IF");
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

bool FormatParser::start(const char *fmt, bool colors, bool parse_only_colors, string *errtext) {
	/* Free possibly old content */
	delete root_node;
	/* root_note = NULLPTR;  Make sure this is called in error case */
	/* Initialize machine */
	enable_colors = colors;
	only_colors = parse_only_colors;
	last_error = _("check your syntax");
	state = START;
	band = fmt;
	band_position = fmt;
	/* Run machine */
	while((state != STOP) && (state != ERROR)) {
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
			default:       last_error = _("bad state: undefined");
			               state = ERROR;
		}
	}
	/* Check if the machine went into ERROR-state. */
	if(state == ERROR) {
		root_node = NULLPTR;
		/* Clean stacks. */
		while(!keller.empty()) {
			delete keller.top();
			keller.pop();
		}
		if(errtext != NULLPTR) {
			size_t line(0), column(0);
			getPosition(&line, &column);
			*errtext = eix::format(_("line %s, column %s: %s")) % line % column % last_error;
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
bool PrintFormat::print(void *entity, GetProperty get_property, Node *root, const DBHeader *dbheader, VarDbPkg *vardbpkg, const PortageSettings *ps, const SetStability *s, bool check_only) {
	// The four hackish variables
	header = dbheader;
	vardb = vardbpkg;
	portagesettings = ps;
	stability = s;
	version_variables = NULLPTR;

	varcache.clear_use();
	user_variables.clear();
	bool r;
	if(unlikely(check_only)) {
		OutputString dummy;
		r = recPrint(&dummy, entity, get_property, root);
	} else {
		r = recPrint(NULLPTR, entity, get_property, root);
	}
	// Reset the four hackish variables
	header = NULLPTR;
	vardb = NULLPTR;
	portagesettings = NULLPTR;
	stability = NULLPTR;
	return r;
}

bool PrintFormat::parseFormat(Node **rootnode, const char *fmt, string *errtext) {
	FormatParser parser;
	if(likely(parser.start(fmt, !no_color, false, errtext))) {
		*rootnode = parser.rootnode();
		return true;
	}
	*rootnode = NULLPTR;
	return false;
}
