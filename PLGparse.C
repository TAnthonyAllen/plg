#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "DoubleLinkList.h"
#include "PLGrule.h"
#include "Alternative.h"
#include "DoubleLink.h"
#include "Stak.h"
#include "BaseHash.h"
#include "Element.h"
#include "Buffer.h"
#include "PLGitem.h"
#include "PLGset.h"
#include "PLG.h"
#include "PLGparse.h"

/*******************************************************************************
	foundIn — free function (was PLGset::foundIn(PLGitem) before PLGset
	moved into the support library, where PLGitem is unreachable).
	Returns true iff any character of `item`'s text is in `set`.
*******************************************************************************/
extern "C" int foundIn(PLGset *set, PLGitem *item)
{
	if ( item && set )
		{
		int 	i = 0;
		char 	*atText = item->itemStart;
		if ( atText && !set->isEmpty() )
			for ( i = item->itemLength; i > 0; i--, atText++ )
				if ( set->contains(*atText) )
					return 1;
		}
	return 0;
}

/*******************************************************************************
	PLGparse constructors
*******************************************************************************/
PLGparse::PLGparse()
{
	buffer = 0;
	cursor = 0;
	eof = 0;
	plgStart = 0;
	parserName = 0;
	currentRule = 0;
	currentAlt = 0;
	currentSet = 0;
	defaultSkip = 0;
	skipSet = 0;
	skipStack = 0;
	inputStack = 0;
	depth = 0;
	helperCount = 0;
	pendingLabel = 0;
	pendingNoSkip = 0;
	sourceDir = 0;
	bodyRule = 0;
	alternateSet = 0;
	characterSet = 0;
	commandSet = 0;
	commentSet = 0;
	elementSet = 0;
	excludeSet = 0;
	nameSet = 0;
	numberSet = 0;
	singleQuote = 0;
	debugRulePLG = 0;
	debugGuardPLG = 0;
	doNotGuard = 0;
	setsInitialized = 0;
	skipping = 0;
	rules = new BaseHash();
	setTable = new BaseHash();
	keyWordTable = new BaseHash();
	conditionTable = new BaseHash();
	variableTable = new BaseHash();
	spliceAccumulator = new Buffer("splice",8192);
	actionNames = new Stak();
	if ( !PLGitem::itemEmpty )
		{
		PLGitem::itemEmpty = new PLGitem();
		PLGitem::itemEmpty->itemLength = 0;
		}
}

PLGparse::PLGparse(char *input)
{
	plgStart = 0;
	parserName = 0;
	currentRule = 0;
	currentAlt = 0;
	currentSet = 0;
	defaultSkip = 0;
	skipSet = 0;
	skipStack = 0;
	inputStack = 0;
	depth = 0;
	helperCount = 0;
	pendingLabel = 0;
	pendingNoSkip = 0;
	sourceDir = 0;
	bodyRule = 0;
	alternateSet = 0;
	characterSet = 0;
	commandSet = 0;
	commentSet = 0;
	elementSet = 0;
	excludeSet = 0;
	nameSet = 0;
	numberSet = 0;
	singleQuote = 0;
	debugRulePLG = 0;
	debugGuardPLG = 0;
	doNotGuard = 0;
	setsInitialized = 0;
	skipping = 0;
	buffer = new Buffer();
	buffer->appendString(input,0,0);
	cursor = buffer->start;
	eof = buffer->end;
	rules = new BaseHash();
	setTable = new BaseHash();
	keyWordTable = new BaseHash();
	conditionTable = new BaseHash();
	variableTable = new BaseHash();
	spliceAccumulator = new Buffer("splice",8192);
	actionNames = new Stak();
	if ( !PLGitem::itemEmpty )
		{
		PLGitem::itemEmpty = new PLGitem();
		PLGitem::itemEmpty->itemLength = 0;
		}
}

/*****************************************************************************
	addTest builds an Element from the given parameters and appends it to
	currentAlt. Used by setRules() to replace the verbose
	"new Element(); elem->kind = ...; alt->addElement(elem);" pattern with
	one call per element.
*****************************************************************************/
void PLGparse::addTest(int kind, char *data, char *label, int min, int max, char *skipSet)
{
Element 	*elem = new Element();
	elem->kind = kind;
	elem->minimum = min;
	elem->maximum = max;
	// min=-1 is the convention for the `!` (banged) modifier — flip the
	// banged flag here so the element actually inverts at match time.
	if ( min == -1 )
		elem->banged = 1;
	if ( label )
		elem->label = label;
	if ( skipSet )
		elem->skipSet = getSet(skipSet);
	switch (kind)
		{
		case 1:
			elem->litText = data;
			break;
		case 2:
			elem->chrChar = *data;
			break;
		case 3:
			elem->setRef = getSet(data);
			break;
		case 6:
			elem->ruleRef = getRule(data);
			break;
		case 7:
			::fprintf(stderr,"addTest: kKeyTable not yet supported (getTable() not implemented)\n");
			break;
		case 4:
		case 5:
			break;
			// no data needed
		default:
			::fprintf(stderr,"addTest: unsupported kind %d\n",kind);
			break;
		}
	if ( !currentAlt )
		{
		::fprintf(stderr,"addTest: no currentAlt set\n");
		return;
		}
	currentAlt->elements->add((void*)elem);
}

// advance to the next line
void PLGparse::attachActions(char *content)
{
char 		*src = 0;
char 		*nameStart = 0;
int 		nameLen = 0;
char 		*tagStart = 0;
int 		tagLen = 0;
char 		*key = 0;
int 		keyLen = 0;
char 		*bodyStart = 0;
char 		*scan = 0;
char 		*ln = 0;
char 		after = 0;
int 		bodyLen = 0;
char 		*body = 0;
int 		isDefer = 0;
int 		atTerminator = 0;
PLGrule 	*rule = 0;
	if ( !content )
		return;
	src = content;
	while ( *src )
		{
		// Skip blank lines / whitespace between entries.
		while ( *src == ' ' || *src == '\t' || *src == '\n' || *src == '\r' || *src == '\f' )
			src++;
		if ( !*src )
			break;
		// Header line: ruleName [':' tag] ['defer'].
		// Read the rule name: <identifier>.
		nameStart = src;
		if ( (*src >= 'a' && *src <= 'z') || (*src >= 'A' && *src <= 'Z') )
			{
			src++;
			while ( (*src >= 'a' && *src <= 'z') || (*src >= 'A' && *src <= 'Z') || (*src >= '0' && *src <= '9') )
				src++;
			}
		nameLen = src - nameStart;
		if ( nameLen == 0 )
			{
			// Not a header — skip the rest of this line and retry.
			while ( *src && *src != '\n' )
				src++;
			continue;
			}
		// Optional ':' option tag (name or number), appended to the key:
		// ruleName → "Block", ruleName:2 → "Block2", ruleName:cast → "Blockcast".
		tagStart = src;
		tagLen = 0;
		if ( *src == ':' )
			{
			src++;
			tagStart = src;
			while ( (*src >= 'a' && *src <= 'z') || (*src >= 'A' && *src <= 'Z') || (*src >= '0' && *src <= '9') )
				src++;
			tagLen = src - tagStart;
			}
		keyLen = nameLen + tagLen;
		key = (char*)::malloc(keyLen + 1);
		::strncpy(key,nameStart,nameLen);
		if ( tagLen )
			::strncpy(key + nameLen,tagStart,tagLen);
		*(key + keyLen) = 0;
		// Optional 'defer' keyword → deferred action; absence → immediate.
		isDefer = 0;
		while ( *src == ' ' || *src == '\t' )
			src++;
		if ( ::strncmp(src,"defer",5) == 0 )
			{
			after = *(src + 5);
			if ( !((after >= 'a' && after <= 'z') || (after >= 'A' && after <= 'Z') || (after >= '0' && after <= '9')) )
				{
				isDefer = 1;
				src += 5;
				}
			}
		// Consume the rest of the header line.
		while ( *src && *src != '\n' )
			src++;
		if ( *src == '\n' )
			src++;
		// Body runs to a standalone 'enD' line. 'enD' (not 'end') is the
		// terminator keyword precisely so it never collides with the common
		// body variable `end` (e.g. PoundCommand's `PLGitem end = state;`).
		// Matches action.g's `ActionEnd : 'enD' nameSet!&`.
		bodyStart = src;
		scan = src;
		atTerminator = 0;
		while ( *scan )
			{
			ln = scan;
			// first char of the current line
			while ( *ln == ' ' || *ln == '\t' )
				ln++;
			if ( *ln == 'e' && *(ln + 1) == 'n' && *(ln + 2) == 'D' )
				{
				after = *(ln + 3);
				if ( !((after >= 'a' && after <= 'z') || (after >= 'A' && after <= 'Z') || (after >= '0' && after <= '9')) )
					{
					atTerminator = 1;
					break;
					// scan is at the 'end' line start
					}
				}
			while ( *scan && *scan != '\n' )
				scan++;
			if ( *scan == '\n' )
				scan++;
			}
		// Body excludes the terminator line; trim its trailing newline.
		bodyLen = scan - bodyStart;
		if ( bodyLen > 0 && *(bodyStart + bodyLen - 1) == '\n' )
			bodyLen--;
		body = (char*)::malloc(bodyLen + 1);
		::strncpy(body,bodyStart,bodyLen);
		*(body + bodyLen) = 0;
		// Advance src past the 'end' line.
		src = scan;
		if ( atTerminator )
			{
			while ( *src && *src != '\n' )
				src++;
			if ( *src == '\n' )
				src++;
			}
		// Bind the body to its rule, keyed by ruleName(+tag).
		rule = getRule(key);
		if ( isDefer )
			rule->deferAction = new PLGitem(body);
		else	rule->immediateAction = new PLGitem(body);
		}
}

/*******************************************************************************
	divertInput loads the string passed in to a new buffer, if there is an
    existing current buffer, it gets put on the inputStack, and the current
    cursor is set to the contents of the new buffer
*******************************************************************************/
void PLGparse::divertInput(char *s)
{
	if ( !inputStack )
		inputStack = new Stak();
	// Save outer parse cursor on the buffer so revertInput can restore it.
	buffer->current = cursor;
	inputStack->push(buffer);
	buffer = new Buffer();
	buffer->appendString(s,0,0);
	cursor = buffer->start;
	eof = buffer->end;
}

/*****************************************************************************
	Like divertInput(s,rule) but looks the rule up by name. The rule lookup
    (and not-found diagnostic) is handled by parse(String name).
*****************************************************************************/
PLGitem *PLGparse::divertInput(char *s, char *ruleName)
{
PLGitem 	*item = 0;
	divertInput(s);
	item = parse(ruleName);
	revertInput();
	return item;
}

/*****************************************************************************
	Diverts input to the string passed in and fires up the rule. After the
    rule runs, reverts the input back and returns the result of the parse.
*****************************************************************************/
PLGitem *PLGparse::divertInput(char *s, PLGrule *rule)
{
PLGitem 	*item = 0;
	divertInput(s);
	item = parse(rule);
	revertInput();
	return item;
}

/*****************************************************************************
	Loop thru rules and generate code to implement them
*****************************************************************************/
void PLGparse::generateRules(Buffer *output, char *baseName)
{
PLGrule 		*rule = 0;
PLGset 			*set = 0;
PLGrule 		*src = 0;
Alternative 	*alt = 0;
DoubleLink 		*altLink = 0;
int 			optN = 0;
Buffer 			*keyBuf = new Buffer("optkey",64);
	// Emit the generated parser's include manifest. The grammar's own
	// header (Tawk.g: `include includes`) names a tok include file that
	// pulls in frame/globals plus the `external Tawk` block (tok.ext) —
	// the class fields, KeyTable/PLGset decls, and method signatures tok
	// needs to compile the generated .twk. We reproduce that line here.
	// TODO (general): capture pre-%% Header includes from the grammar
	// (re-enable HeaderplgNow → includeBuffer) instead of hard-coding.
	output->appendString("include includes",0,0);
	output->appendString("\n",0,0);
	output->appendString("",0,0);
	output->appendString("\n",0,0);
	// Class wrapper. Per the new model the generated parser is one class:
	// the .rtn splice (parser-state fields + helper-method bodies) AND the
	// expanded action-body methods both live INSIDE the class, so action
	// bodies reach parser state directly as members (no parser pointer).
	// The matching forward declarations live in tok.ext's `external Tawk`
	// block (pulled in via `include includes`); plg emits no externals
	// here. BaseName is the grammar filename stem (process() derives it).
	output->appendString("class ",0,0);
	output->appendString(baseName,0,0);
	output->appendString(" extends PLGparse",0,0);
	output->appendString("\n",0,0);
	output->appendString("{",0,0);
	output->appendString("\n",0,0);
	// .rtn splice flush — verbatim parser-state field declarations and
	// helper-method bodies. Now INSIDE the class: at top level the field
	// declarations (e.g. `SymbolType currentClass,`) tripped tok's
	// ERROR Inheritance; inside a class they are legal members.
	if ( spliceAccumulator && spliceAccumulator->length() > 0 )
		{
		output->appendString(spliceAccumulator->toString(),0,0);
		output->appendString("\n",0,0);
		}
	// Per-alternative action distribution. attachActions stored each .act
	// option's body on a numbered rule shell (Foo, Foo2, Foo3 …). An action
	// belongs to ONE alternative (option) of its rule, so copy each shell's
	// body + method name onto the matching alternative of the base rule.
	// (Done here, not in attachActions, because a rule's alternatives are not
	// parsed until after the .act include.) Alternative 0 ← the base rule
	// itself; alternative N ← the numbered shell <name><N+1>.
	rules->hashList->entry = 0;
	while ( rule = (PLGrule*)rules->hashList->next() )
		{
		if ( !rule->alternatives )
			continue;
		optN = 0;
		for ( altLink = rule->alternatives->first; altLink; altLink = altLink->next )
			{
			alt = (Alternative*)altLink->value;
			if ( optN == 0 )
				{
				src = rule;
				alt->actionName = rule->name;
				}
			else {
				keyBuf->reset();
				keyBuf->appendString(rule->name,0,0);
				keyBuf->appendInt((optN + 1),0,0);
				alt->actionName = keyBuf->toString();
				src = (PLGrule*)rules->get(alt->actionName);
				}
			if ( src )
				{
				alt->immediateAction = src->immediateAction;
				alt->deferAction = src->deferAction;
				}
			optN++;
			}
		}
	// Expanded action-body methods, one per alternative that carries an
	// action. Emitted inside the class, before setRules() wires them. A
	// numbered shell (0 alternatives) emits nothing — its body was distributed
	// above onto the base rule's alternative.
	rules->hashList->entry = 0;
	while ( rule = (PLGrule*)rules->hashList->next() )
		rule->writeActions(baseName,output);
	// setRules() — wires rules and sets at runtime (unchanged).
	output->appendString("\nvoid setRules()\n{\n	setSkip();",0,0);
	output->appendString("\n",0,0);
	setTable->hashList->resetIterator();
	while ( set = (PLGset*)setTable->hashList->next() )
		set->generateNamed(output);
	rules->hashList->entry = 0;
	while ( rule = (PLGrule*)rules->hashList->next() )
		rule->generate(baseName,output);
	output->appendString("}",0,0);
	output->appendString("\n",0,0);
	// end of setRules
	output->appendString("}",0,0);
	output->appendString("\n",0,0);
	// end of class
}

/*****************************************************************************
	Find a rule and return it
*****************************************************************************/
PLGrule *PLGparse::getRule(char *name)
{
PLGrule 	*rule = (PLGrule*)rules->get(name);
	if ( !rule )
		{
		rule = new PLGrule(name);
		rules->add(name,(void*)rule);
		}
	return rule;
}

/*****************************************************************************
	Look up a set in the setTable and return it
*****************************************************************************/
PLGset *PLGparse::getSet(char *specs)
{
PLGset 	*set = (PLGset*)setTable->get(specs);
	if ( !set )
		{
		set = new PLGset(specs);
		setTable->add(specs,(void*)set);
		}
	return set;
}

PLGset *PLGparse::getSet(char *name, char *specs)
{
PLGset 	*set = (PLGset*)setTable->get(name);
	if ( !set )
		{
		set = new PLGset(specs);
		set->name = name;
		setTable->add(name,(void*)set);
		}
	return set;
}

PLGset *PLGparse::getSet(PLGset *named, char *specs)
{
	if ( !named )
		named = new PLGset(specs);
	else	named->set(specs);
	setTable->add(named->name,(void*)named);
	return named;
}

/*****************************************************************************
	initialize — set the skip set, then populate FIRST sets (guards) on
	every rule for fast-rejection in PLGrule::match.
*****************************************************************************/
void PLGparse::initialize()
{
PLGrule 	*rule = 0;
	setSkip();
	rules->hashList->entry = 0;
	while ( rule = (PLGrule*)rules->hashList->next() )
		rule->setGuard();
}

/*****************************************************************************
	The parse rules run the parse by calling match
*****************************************************************************/
PLGitem *PLGparse::parse(PLGrule *rule)
{
	if ( rule )
		debugRulePLG = 1;
		return rule->match(this);
	return 0;
}

PLGitem *PLGparse::parse(char *name)
{
PLGrule 	*rule = (PLGrule*)rules->get(name);
	if ( !rule )
		{
		::fprintf(stderr,"parse: rule not found: %s\n",name);
		return 0;
		}
	return rule->match(this);
}

/*****************************************************************************
    parseActDeclarations — extract a leading `actions Name1, Name2, ...;`
    declarations line from .act file content. Each declared name is pushed
    onto this.actionNames as a freshly-allocated String. Returns a pointer
    into the input string at the position just past the `;` (and trailing
    newline if present), so the caller can append the remaining content
    verbatim to spliceAccumulator.

    If no leading "actions" keyword is found (after skipping whitespace),
    the input is returned unchanged — the entire file body is splice
    content, no declared names. This is the expected shape for old-style
    .act files migrating into the new pipeline.

    plg does not own .act content beyond this one line; the rest is for
    tok to consume after Brief 4 splices it into generated output.
*****************************************************************************/
char *PLGparse::parseActDeclarations(char *input)
{
char 	*src = 0;
char 	*nameStart = 0;
int 	nameLen = 0;
char 	*name = 0;
	if ( !input )
		return input;
	src = input;
	// Skip leading whitespace
	while ( *src == ' ' || *src == '\t' || *src == '\n' || *src == '\r' || *src == '\f' )
		src++;
	// Check for "actions" keyword followed by whitespace
	if ( ::strncmp(src,"actions",7) != 0 )
		return input;
	if ( !(*(src + 7) == ' ' || *(src + 7) == '\t') )
		return input;
	src += 7;
	// Scan comma-separated names until `;`
	while ( *src && *src != ';' )
		{
		// Skip whitespace and commas
		while ( *src == ' ' || *src == '\t' || *src == ',' || *src == '\n' || *src == '\r' )
			src++;
		if ( !*src || *src == ';' )
			break;
		// Capture name — non-whitespace, non-comma, non-`;`
		nameStart = src;
		while ( *src && *src != ' ' && *src != '\t' && *src != ',' && *src != ';' && *src != '\n' && *src != '\r' )
			src++;
		nameLen = src - nameStart;
		if ( nameLen > 0 )
			{
			name = (char*)::malloc(nameLen + 1);
			::strncpy(name,nameStart,nameLen);
			*(name + nameLen) = 0;
			actionNames->push((void*)name);
			}
		}
	if ( *src == ';' )
		src++;
	// Consume the newline after `;` if present, so spliced content
	// doesn't start with a stray blank line.
	if ( *src == '\n' )
		src++;
	return src;
}

void PLGparse::reportError(Alternative *alt, char *message)
{
	::fprintf(stderr,"FAIL: %s\n",message);
}

void PLGparse::restore(char *s)
{
	cursor = s;
}

/*******************************************************************************
	revertInput undoes what divertInput done did, reseting the parser input
    buffer back to what it was before being diverted
*******************************************************************************/
void PLGparse::revertInput()
{
	if ( !inputStack || !inputStack->length )
		{
		::fprintf(stderr,"revertInput: input stack is empty\n");
		return;
		}
	buffer = (Buffer*)inputStack->pop();
	cursor = buffer->current;
	eof = buffer->end;
}

/*****************************************************************************
    Set banged on the most-recently-added element on currentAlt. Pairs with
    addTest to express the `!` (banged / negative-lookahead) modifier.
*****************************************************************************/
void PLGparse::setBanged()
{
DoubleLink 	*last = 0;
Element 	*e = 0;
	if ( !currentAlt )
		return;
	last = currentAlt->elements->last;
	if ( !last )
		return;
	e = (Element*)last->value;
	if ( e )
		e->banged = 1;
}

/*****************************************************************************
    Set isIgnored on the most-recently-added element on currentAlt. The
    element still must match but its result is discarded (no label binding,
    no children entry).
*****************************************************************************/
void PLGparse::setIgnored()
{
DoubleLink 	*last = 0;
Element 	*e = 0;
	if ( !currentAlt )
		return;
	last = currentAlt->elements->last;
	if ( !last )
		return;
	e = (Element*)last->value;
	if ( e )
		e->isIgnored = 1;
}

/*******************************************************************************
	Load input into the parse buffer
*******************************************************************************/
void PLGparse::setInput(char *s)
{
	if ( !buffer )
		buffer = new Buffer();
	buffer->reset();
	buffer->appendString(s,0,0);
	cursor = buffer->start;
	eof = buffer->end;
}

void PLGparse::setInput(PLGitem *item)
{
	if ( !buffer )
		buffer = new Buffer();
	buffer->reset();
	buffer->appendString(item->toString(),0,0);
	cursor = buffer->start;
	eof = buffer->end;
}

/*****************************************************************************
    Set noSkip on the most-recently-added element on currentAlt. Used in
    setRules() to express the `&` (no-skip) modifier on a preceding element
    where the source plg syntax `excludeSet!&` couldn't otherwise be
    threaded through addTest. Pair with the banged auto-set in addTest
    when both `!` and `&` should apply.
*****************************************************************************/
void PLGparse::setNoSkip()
{
DoubleLink 	*last = 0;
Element 	*e = 0;
	if ( !currentAlt )
		return;
	last = currentAlt->elements->last;
	if ( !last )
		return;
	e = (Element*)last->value;
	if ( e )
		e->noSkip = 1;
}

/*******************************************************************************
	Create skipSet
*******************************************************************************/
void PLGparse::setSkip()
{
	if ( !defaultSkip )
		defaultSkip = new PLGset(" \t\n\r\f");
	skipSet = defaultSkip;
}

/*******************************************************************************
	Skip space and advance the current cursor
*******************************************************************************/
void PLGparse::skip()
{
	if ( skipSet )
		cursor = skipSet->skip(cursor);
}

/*******************************************************************************
	PLGparse methods that alter state
*******************************************************************************/
char *PLGparse::snapshot()
{
	return cursor;
}

/*****************************************************************************
    stripComments - pre-parse pass that replaces block comments (slash-star
    ... star-slash) and line comments (slash-slash to newline) with spaces,
    preserving newlines and char positions so error messages still report
    correct line numbers. Quoted strings are passed through unmodified -
    comment markers inside 'literals' or "literals" don't trigger stripping.
    Returns a freshly-allocated String; caller owns the returned buffer.
*****************************************************************************/
char *PLGparse::stripComments(char *input)
{
char 	*src = 0;
char 	*dst = 0;
char 	*out = 0;
int 	len = 0;
char 	quote = 0;
	if ( !input )
		return input;
	len = ::strlen(input);
	out = (char*)::malloc(len + 1);
	src = input;
	dst = out;
	while ( *src )
		{
		if ( *src == '[' )
			{
			// Character-set literal — pass through to closing ] verbatim,
			// honoring `\X` escape so a literal `]` inside doesn't terminate.
			*dst = *src;
			dst++;
			src++;
			while ( *src && *src != ']' )
				{
				if ( *src == '\\' && *(src + 1) )
					{
					*dst = *src;
					dst++;
					src++;
					*dst = *src;
					dst++;
					src++;
					}
				else {
					*dst = *src;
					dst++;
					src++;
					}
				}
			if ( *src )
				{
				*dst = *src;
				dst++;
				src++;
				}
			continue;
			}
		if ( *src == '\'' || *src == '"' )
			{
			quote = *src;
			*dst = *src;
			dst++;
			src++;
			while ( *src && *src != quote )
				{
				if ( *src == '\\' && *(src + 1) )
					{
					*dst = *src;
					dst++;
					src++;
					*dst = *src;
					dst++;
					src++;
					}
				else {
					*dst = *src;
					dst++;
					src++;
					}
				}
			if ( *src )
				{
				*dst = *src;
				dst++;
				src++;
				}
			continue;
			}
		if ( *src == '/' && *(src + 1) == '/' )
			{
			while ( *src && *src != '\n' )
				{
				*dst = ' ';
				dst++;
				src++;
				}
			continue;
			}
		if ( *src == '/' && *(src + 1) == '*' )
			{
			*dst = ' ';
			dst++;
			src++;
			*dst = ' ';
			dst++;
			src++;
			while ( *src && !(*src == '*' && *(src + 1) == '/') )
				{
				if ( *src == '\n' )
					{
					*dst = '\n';
					dst++;
					}
				else {
					*dst = ' ';
					dst++;
					}
				src++;
				}
			if ( *src )
				{
				*dst = ' ';
				dst++;
				src++;
				if ( *src )
					{
					*dst = ' ';
					dst++;
					src++;
					}
				}
			continue;
			}
		*dst = *src;
		dst++;
		src++;
		}
	*dst = 0;
	return out;
}

/*****************************************************************************
	Debugging routine to list out the rules and the number of times
	each one was run
*****************************************************************************/
void PLGparse::summary(int threshold)
{
PLGrule 	*rule = 0;
int 		total = 0;
	::printf("Rules\n");
	while ( rule = (PLGrule*)rules->hashList->next() )
		{
		::printf("\t\t%s\n",rule->name);
		total++;
		}
	::printf("\tRules processed: %d\n",total);
}
/*	Warning: the following methods were referenced but not declared
	generateNamed(Buffer*)
*/
