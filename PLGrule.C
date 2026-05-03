#include <string.h>
#include <stdio.h>
#include "DoubleLinkList.h"
#include "Alternative.h"
#include "DoubleLink.h"
#include "PLGset.h"
#include "Element.h"
#include "Buffer.h"
#include "PLGparse.h"
#include "PLGitem.h"
#include "PLGrule.h"

/*******************************************************************************
	Rule constructor
*******************************************************************************/
PLGrule::PLGrule(char *s)
{
	immediate = 0;
	defer = 0;
	guardSet = 0;
	parentParser = 0;
	deferAction = 0;
	failMethod = 0;
	immediateAction = 0;
	balanceRule = 0;
	balanceAny = 0;
	balanceBody = 0;
	balanceBail = 0;
	doNotGuard = 0;
	debug = 0;
	guardComputed = 0;
	alternatives = new DoubleLinkList();
	name = s;
}

/*******************************************************************************
	addAlternative: simple wrapper adds an alternative to alterntives list
*******************************************************************************/
void PLGrule::addAlternative(Alternative *alt)
{
	alternatives->add(alt);
}

/*****************************************************************************
	This method writes out external declarations for actions associated
	with this rule.
*****************************************************************************/
void PLGrule::declareActions(Buffer *output)
{
	// to be rewritten
}

/*****************************************************************************
	Generate the code to implement this rule
*****************************************************************************/
void PLGrule::generate(Buffer *output)
{
Alternative 	*test = 0;
	output->appendString("//\ncurrentRule = getRule(\"");
	output->appendString(name);
	output->appendString("\");");
	output->appendString("\n");
	if ( balanceRule )
		{
		output->appendString("currentRule.immediate = balancE;");
		output->appendString("\n");
		}
	else
	if ( balanceBody )
		{
		output->appendString("currentRule.immediate = balancEbody;");
		output->appendString("\n");
		}
	else
	if ( balanceAny )
		{
		output->appendString("currentRule.immediate = balancEbail;");
		output->appendString("\n");
		}
	else
	if ( immediateAction )
		{
		output->appendString("currentRule.immediate = ");
		output->appendString(name);
		output->appendString(parentParser->parserName);
		output->appendString("Now;");
		output->appendString("\n");
		}
	if ( deferAction )
		{
		output->appendString("currentRule.defer = ");
		output->appendString(name);
		output->appendString(parentParser->parserName);
		output->appendString("Act;");
		output->appendString("\n");
		}
	if ( doNotGuard )
		{
		output->appendString("currentRule.doNotGuard = true;");
		output->appendString("\n");
		}
	else
	if ( guardSet )
		guardSet->generate(output);
	alternatives->resetIterator();
	::fprintf(stderr,"alternatives count: %d\n",alternatives->length);
	while ( test = (Alternative*)alternatives->next() )
		{
		::fprintf(stderr,"generating alternative\n");
		test->generate(output);
		if ( test->guardSet )
			{
			test->guardSet->generate(output);
			output->appendString("currentRule.guardSet = currentSet;");
			output->appendString("\n");
			}
		}
	if ( debug )
		{
		output->appendString("currentRule.debug = true;");
		output->appendString("\n");
		}
	if ( failMethod )
		{
		output->appendString("currentRule.fail = ");
		output->appendString(failMethod->toString());
		output->appendString(";\n");
		}
}

PLGitem *PLGrule::match(PLGparse *state)
{
DoubleLink 		*link = 0;
Alternative 	*alt = 0;
PLGitem 		*result = 0;
int 			altNum = 0;
int 			altCount = alternatives->length;
char 			*saved = 0;
	if ( guardSet && state->cursor < state->eof && !guardSet->contains(*state->cursor) )
		{
		char 	ch = *state->cursor;
		::printf("PLGrule: %s GUARD-REJECTED at offset %lu char='%c' guard=[%s]\n",name,(state->cursor - state->buffer->start),ch,guardSet->toString());
		return 0;
		}
	::printf("PLGrule: %s (%d alts) at offset %lu\n",name,altCount,(state->cursor - state->buffer->start));
	for ( link = alternatives->first; link; link = link->next )
		{
		alt = (Alternative*)link->value;
		altNum++;
		saved = state->cursor;
		::printf("  %s try alt %d/%d at offset %lu\n",name,altNum,altCount,(saved - state->buffer->start));
		if ( alt->match(state,result) )
			{
			if ( state->cursor > saved )
				{
				::printf("  %s alt %d SUCCEEDED -> offset %lu\n",name,altNum,(state->cursor - state->buffer->start));
				if ( immediate )
					immediate(state,result);
				if ( defer )
					{
					result->deferRule = this;
					if ( !result->deferred )
						result->deferred = new DoubleLinkList();
					result->deferred->add(result);
					}
				return result;
				}
			::printf("  %s alt %d ZERO-ADVANCE — treating as fail\n",name,altNum);
			}
		else {
			::printf("  %s alt %d FAILED at offset %lu (was %lu)\n",name,altNum,(state->cursor - state->buffer->start),(saved - state->buffer->start));
			}
		}
	::printf("  %s ALL %d alts failed\n",name,altCount);
	return 0;
}

/*****************************************************************************
	Set the guard sets for this rule.
	Computes the FIRST set across all alternatives:
	  - kLit  → first char of litText
	  - kSet  → setRef
	  - kRuleRef → recurse into the referenced rule's setGuard
	Cycle protection: assign guardSet (initially empty) before recursing
	so a back-edge sees a partial set instead of infinite-looping.
	Rules flagged doNotGuard get null and won't fast-fail.
*****************************************************************************/
PLGset *PLGrule::setGuard()
{
PLGset 			*guard = 0;
Alternative 	*alt = 0;
Element 		*elem = 0;
DoubleLink 		*link = 0;
DoubleLink 		*elemLink = 0;
PLGset 			*sub = 0;
int 			noGuard = 0;
	if ( doNotGuard )
		return 0;
	if ( guardComputed )
		return guardSet;
	guardComputed = 1;
	guard = new PLGset();
	guardSet = guard;
	for ( link = alternatives->first; link; link = link->next )
		{
		alt = (Alternative*)link->value;
		// For each alt, walk elements until we hit a required (min > 0) one.
		// Optional (min=0) elements contribute their FIRST set AND the next
		// element is also reachable, so keep walking.
		for ( elemLink = alt->elements->first; elemLink; elemLink = elemLink->next )
			{
			elem = (Element*)elemLink->value;
			if ( elem )
				{
				switch (elem->kind)
					{
					case 1:
						if ( elem->litText )
							guard->set(*elem->litText);
						break;
					case 3:
						if ( elem->setRef )
							guard->set(elem->setRef);
						break;
					case 6:
						if ( elem->ruleRef )
							{
							sub = elem->ruleRef->setGuard();
							if ( sub )
								guard->set(sub);
							else	noGuard = 1;
							// descendant is doNotGuard
							}
						break;
					}
				if ( elem->minimum > 0 )
					break;
				}
			}
		}
	// If any alt's chain leads through a doNotGuard rule, this rule
	// also can't be guarded — return null so callers don't fast-fail
	// on what would otherwise be an empty (=reject-all) guardSet.
	if ( noGuard )
		{
		guardSet = 0;
		return 0;
		}
	return guard;
}

/*****************************************************************************
	This method writes out the actions associated with this rule.
*****************************************************************************/
void PLGrule::writeActions(char *parserName, Buffer *output)
{
	// TODO: implement
}
