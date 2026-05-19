#include <string.h>
#include <stdio.h>
#include "DoubleLinkList.h"
#include "Alternative.h"
#include "DoubleLink.h"
#include "Stak.h"
#include "Element.h"
#include "Buffer.h"
#include "PLGparse.h"
#include "PLGitem.h"
#include "PLGset.h"
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
	// Blank-line separator only — `//` lines reset TAWK's field-resolution
	// context, which strips `use parser` from the next assignment and breaks
	// compilation when the generated body is pasted into a use-parser block.
	output->appendString("\ncurrentRule = getRule(\"",0,0);
	output->appendString(name,0,0);
	output->appendString("\");",0,0);
	output->appendString("\n",0,0);
	if ( balanceRule )
		{
		output->appendString("currentRule.immediate = balancE;",0,0);
		output->appendString("\n",0,0);
		}
	else
	if ( balanceBody )
		{
		output->appendString("currentRule.immediate = balancEbody;",0,0);
		output->appendString("\n",0,0);
		}
	else
	if ( balanceAny )
		{
		output->appendString("currentRule.immediate = balancEbail;",0,0);
		output->appendString("\n",0,0);
		}
	else
	if ( immediateAction )
		{
		output->appendString("currentRule.immediate = ",0,0);
		output->appendString(name,0,0);
		output->appendString(parentParser->parserName,0,0);
		output->appendString("Now;",0,0);
		output->appendString("\n",0,0);
		}
	if ( deferAction )
		{
		output->appendString("currentRule.defer = ",0,0);
		output->appendString(name,0,0);
		output->appendString(parentParser->parserName,0,0);
		output->appendString("Act;",0,0);
		output->appendString("\n",0,0);
		}
	if ( doNotGuard )
		{
		output->appendString("currentRule.doNotGuard = true;",0,0);
		output->appendString("\n",0,0);
		}
	else
	if ( guardSet )
		guardSet->generate(output);
	alternatives->resetIterator();
	while ( test = (Alternative*)alternatives->next() )
		{
		test->generate(output);
		if ( test->guardSet )
			{
			test->guardSet->generate(output);
			output->appendString("currentRule.guardSet = currentSet;",0,0);
			output->appendString("\n",0,0);
			}
		}
	if ( debug )
		{
		output->appendString("currentRule.debug = true;",0,0);
		output->appendString("\n",0,0);
		}
	if ( failMethod )
		{
		output->appendString("currentRule.fail = ",0,0);
		output->appendString(failMethod->toString(),0,0);
		output->appendString(";\n",0,0);
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
	// Auto-revert at end of any diverted buffer so include-resolved
	// content flows seamlessly back to the outer parse stream.
	while ( state->cursor >= state->eof && state->inputStack && state->inputStack->length )
		state->revertInput();
	if ( guardSet && state->cursor < state->eof && !guardSet->contains(*state->cursor) )
		{
		char 	ch = *state->cursor;
		if ( state->debugRulePLG || debug )
			::printf("PLGrule: %s GUARD-REJECTED at offset %lu char='%c' guard=[%s]\n",name,(state->cursor - state->buffer->start),ch,guardSet->toString());
		return 0;
		}
parseAttempt:
	if ( state->debugRulePLG || debug )
		::printf("PLGrule: %s (%d alts) at offset %lu\n",name,altCount,(state->cursor - state->buffer->start));
	for ( link = alternatives->first; link; link = link->next )
		{
		alt = (Alternative*)link->value;
		altNum++;
		saved = state->cursor;
		if ( state->debugRulePLG || debug )
			::printf("  %s try alt %d/%d at offset %lu\n",name,altNum,altCount,(saved - state->buffer->start));
		if ( alt->match(state,result) )
			{
matchSucceeded:
			if ( state->cursor > saved )
				{
				if ( state->debugRulePLG || debug )
					::printf("  %s alt %d SUCCEEDED -> offset %lu\n",name,altNum,(state->cursor - state->buffer->start));
				if ( immediate )
					immediate(state,result);
				if ( defer )
					{
					DoubleLinkList 	*childEntries = 0;
					DoubleLink 		*dlink = 0;
					result->deferRule = this;
					// Prepend (this, result) so the rule's defer fires
					// BEFORE its children's defers — preorder cascade.
					// AlternativeplgAct must run before its Elements'
					// ElementplgAct so currentAlt is set when Elements
					// arrive.
					childEntries = result->deferred;
					result->deferred = new DoubleLinkList();
					result->deferred->add(result);
					if ( childEntries )
						for ( dlink = childEntries->first; dlink; dlink = dlink->next )
							result->deferred->add(dlink->value);
					}
matched:
				return result;
				}
			if ( state->debugRulePLG || debug )
				::printf("  %s alt %d ZERO-ADVANCE — treating as fail\n",name,altNum);
			// Restore cursor — Alternative.match may have advanced
			// through partial matches before its required element failed.
			// Without this, the next alt starts at the wrong position.
			state->cursor = saved;
			}
		else {
			if ( state->debugRulePLG || debug )
				::printf("  %s alt %d FAILED at offset %lu (was %lu)\n",name,altNum,(state->cursor - state->buffer->start),(saved - state->buffer->start));
			state->cursor = saved;
			}
		}
	if ( state->debugRulePLG || debug )
		::printf("  %s ALL %d alts failed\n",name,altCount);
parseReturn:
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
				// Banged elements (`!`, negative lookahead) — matching means
				// failure, so their FIRST chars must NOT seed the guard. Skip
				// contribution and continue walking the alternative.
				if ( elem->banged )
					{
					if ( elem->minimum > 0 || elem->minimum == -1 )
						break;
					continue;
					}
				switch (elem->kind)
					{
					case 1:
						if ( elem->litText )
							guard->set(*elem->litText);
						break;
					case 2:
						guard->set(elem->chrChar);
						break;
					case 3:
						// Negated sets (`^X`) admit "almost any char" — we
						// can't compactly union them into a positive FIRST
						// set. Treat as noGuard so the rule isn't fast-
						// rejected on legitimate matches.
						if ( elem->setRef )
							{
							if ( elem->setRef->negated )
								noGuard = 1;
							else	guard->set(elem->setRef);
							}
						break;
					case 4:
						// Matches any char — guard cannot fast-reject.
						noGuard = 1;
						break;
					case 5:
						// Matches end-of-input — no usable FIRST char to
						// pre-screen on.
						noGuard = 1;
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
					default:
						// kKeyTable, kCondition, kVariable, kUpTo, kBalanced —
						// FIRST set isn't easily computable from the element
						// alone. Conservative: noGuard, accept anything.
						noGuard = 1;
						break;
					}
				if ( elem->minimum > 0 )
					break;
				}
			}
		}
	// If ANY alt's chain has a noGuard reason (kAny, negated set, banged-
	// only chain, doNotGuard descendant, unknown kind), this rule also
	// can't be reliably guarded — return null so callers don't fast-fail
	// on what would otherwise be an empty (=reject-all) guardSet. Empty
	// PLGset and null are NOT the same: empty rejects everything; null
	// means "accept anything, decide at match time."
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
