#include <string.h>
#include <stdio.h>
#include "DoubleLinkList.h"
#include "PLGrule.h"
#include "Alternative.h"
#include "DoubleLink.h"
#include "PLGset.h"
#include "Stak.h"
#include "BaseHash.h"
#include "Element.h"
#include "Buffer.h"
#include "PLGitem.h"
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
	Main
*******************************************************************************/
int main(int argc, char **argv)
{
PLG 	*state = 0;
	if ( argc > 1 )
		{
		state = new PLG();
		state->process(argv[1]);
		}
	else {
		state = new PLG("12345 hello");
		state->run();
		}
}

/*******************************************************************************
	PLGparse constructors
*******************************************************************************/
PLGparse::PLGparse()
{
	buffer = 0;
	cursor = 0;
	eof = 0;
	parserName = 0;
	currentRule = 0;
	currentAlt = 0;
	currentSet = 0;
	defaultSkip = 0;
	skipSet = 0;
	skipStack = 0;
	inputStack = 0;
	depth = 0;
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
	if ( !PLGitem::itemEmpty )
		{
		PLGitem::itemEmpty = new PLGitem();
		PLGitem::itemEmpty->itemLength = 0;
		}
}

PLGparse::PLGparse(char *input)
{
	parserName = 0;
	currentRule = 0;
	currentAlt = 0;
	currentSet = 0;
	defaultSkip = 0;
	skipSet = 0;
	skipStack = 0;
	inputStack = 0;
	depth = 0;
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
	buffer = ::bufferFactory1();
	buffer->appendString(input);
	cursor = buffer->start;
	eof = buffer->end;
	rules = new BaseHash();
	setTable = new BaseHash();
	keyWordTable = new BaseHash();
	conditionTable = new BaseHash();
	variableTable = new BaseHash();
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
	buffer = ::bufferFactory1();
	buffer->appendString(s);
	cursor = buffer->start;
	eof = buffer->end;
}

/*****************************************************************************
	Loop thru rules and generate code to implement them
*****************************************************************************/
void PLGparse::generateRules(Buffer *output)
{
PLGrule 	*rule = 0;
PLGset 		*set = 0;
	output->appendString("\nvoid setRules()\n{\n	setSkip();");
	output->appendString("\n");
	setTable->hashList->resetIterator();
	while ( set = (PLGset*)setTable->hashList->next() )
		set->generate(output);
	rules->hashList->entry = 0;
	while ( rule = (PLGrule*)rules->hashList->next() )
		rule->generate(output);
	output->appendString("}");
	output->appendString("\n");
	// end of setup
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

PLGitem *PLGparse::parse(PLGrule *rule)
{
	if ( !rule )
		return 0;
	return rule->match(this);
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
		buffer = ::bufferFactory1();
	buffer->reset();
	buffer->appendString(s);
	cursor = buffer->start;
	eof = buffer->end;
}

void PLGparse::setInput(PLGitem *item)
{
	if ( !buffer )
		buffer = ::bufferFactory1();
	buffer->reset();
	buffer->appendString(item->toString());
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
/*	Warning: the following methods were referenced but not declared
	process(char*)
*/
