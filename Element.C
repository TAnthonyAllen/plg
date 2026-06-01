#include <string.h>
#include <stdio.h>
#include "DoubleLinkList.h"
#include "PLGrule.h"
#include "DoubleLink.h"
#include "Buffer.h"
#include "KeyTableItem.h"
#include "KeyTable.h"
#include "PLGparse.h"
#include "PLGitem.h"
#include "PLGset.h"
#include "Element.h"

PLGitem *Element::applyRepetition(PLGparse *state, PLGitem *firstResult)
{
	if ( !firstResult )
		{
		if ( minimum == 0 || minimum == -1 )
			return PLGitem::itemEmpty;
		return 0;
		}
PLGitem *head = firstResult;
PLGitem *tail = head;
DoubleLink *dlink = 0;
int count = 1;
	if ( state->debugRulePLG )
		::printf("Element applyRepetition\n");
	while ( count < maximum )
		{
		char 	*saved = state->snapshot();
		// Skip between repeats only for sub-rule (kRuleRef) repetitions,
		// where each iteration is its own rule call separated by
		// whitespace (e.g. Name+ across newlines). Char-level repetitions
		// (kSet, kLit, etc.) must NOT skip between matches — that would
		// let `[a-zA-Z]+` swallow whitespace inside what should be one name.
		if ( !noSkip && kind == 6 )
			state->skip();
		// 6 = kRuleRef (kRuleRef macro is a test, not a value)
		// Re-run the element's kind-specific matcher
		if ( state->debugRulePLG )
			::printf("\tapplyRepetition matchByKind kind=%u cursor=%s\n",kind,state->cursor);
		PLGitem *nextItem = matchByKind(state);
		if ( !nextItem )
			{
			state->restore(saved);
			break;
			}
		tail->itemNext = nextItem;
		tail = nextItem;
		// Cascade deferred entries from each repeat-match into head so
		// Alternative.match sees the full set, not just the first match's.
		if ( nextItem->deferred )
			{
			if ( !head->deferred )
				head->deferred = new DoubleLinkList();
			for ( dlink = nextItem->deferred->first; dlink; dlink = dlink->next )
				head->deferred->add(dlink->value);
			}
		count++;
		}
	head->itemLength = state->cursor - head->itemStart;
	if ( count < minimum )
		return 0;
	return head;
}

/*******************************************************************************
	Generate an element as an addTest() call. addTest packages
	new-Element + kind + min/max/label/skipSet into one line; modifiers that
	addTest doesn't carry (noSkip, banged, isIgnored) emit as setX() helpers
	immediately after. Kinds without addTest support (kKeyTable, kCondition)
	fall back to explicit Element construction against currentAlt.
*******************************************************************************/
void Element::generate(Buffer *output)
{
char 	*data = 0;
char 	*skipName = 0;
int 	kindNum = 0;
	// Special case: processUpTo (the `{` and `}` modifiers in source grammar).
	// The element captures characters up to a literal terminator, optionally
	// consuming it (skipOverMatch from `}`). Expand to TWO emitted elements
	// matching the OLD hand-written setRules pattern:
	//   addTest(3, "^X", "label", 1, 999999, "");   — kSet capture
	//   addTest(1, "X",  "",      1, 1,      "");   — terminator (only if `}`)
	// X is the original litText; the captured set negates it. The runtime
	// matchSet stops at the terminator char, leaving it for the kLit (or
	// for the next element when `{` doesn't consume).
	if ( processUpTo )
		{
		output->appendString("addTest(3, \"^",0,0);
		PLGset::printText(litText,output);
		output->appendString("\", \"",0,0);
		if ( label )
			output->appendString(label,0,0);
		output->appendString("\", 1, 999999, \"\");",0,0);
		output->appendString("\n",0,0);
		// Propagate the noSkip flag from the source element. Source like
		// `'\n'}&` has noSkip=true (from `&`); without setNoSkip on each
		// emitted element, Element.match's default skip() runs before
		// the kLit terminator and eats the very char it's supposed to
		// match — Body's error fallback then can't consume single-line
		// content + newline, parse halts.
		if ( noSkip )
			{
			output->appendString("setNoSkip();",0,0);
			output->appendString("\n",0,0);
			}
		if ( skipOverMatch )
			{
			output->appendString("addTest(1, \"",0,0);
			PLGset::printText(litText,output);
			output->appendString("\", \"\", 1, 1, \"\");",0,0);
			output->appendString("\n",0,0);
			if ( noSkip )
				{
				output->appendString("setNoSkip();",0,0);
				output->appendString("\n",0,0);
				}
			}
		return;
		}
	switch (kind)
		{
		case 1:
			kindNum = 1;
			data = litText;
			break;
		case 2:
			kindNum = 2;
			// single-char data: rely on TAWK string-concat with the char
			data = "";
			// sentinel — emit chrChar inline below
			break;
		case 3:
			kindNum = 3;
			if ( setRef->name )
				data = setRef->name;
			else
			if ( setRef->specs )
				data = setRef->specs;
			else	data = "";
			break;
		case 4:
			kindNum = 4;
			data = "";
			break;
		case 5:
			kindNum = 5;
			data = "";
			break;
		case 6:
			kindNum = 6;
			data = ruleRef->name;
			break;
		case 7:
		case 8:
			// Fall back to raw form against currentAlt
			output->appendString("{\nElement elem = new Element();",0,0);
			output->appendString("\n",0,0);
			output->appendString("elem.minimum = ",0,0);
			output->appendInt(minimum,0,0);
			output->appendString(";",0,0);
			output->appendString("\n",0,0);
			output->appendString("elem.maximum = ",0,0);
			output->appendInt(maximum,0,0);
			output->appendString(";",0,0);
			output->appendString("\n",0,0);
			if ( kind == kKeyTable(kind) )
				{
				output->appendString("elem.kind = 7;",0,0);
				output->appendString("\n",0,0);
				// kKeyTable
				output->appendString("elem.tableRef = getTable(\"",0,0);
				output->appendString(tableRef->name,0,0);
				output->appendString("\");",0,0);
				output->appendString("\n",0,0);
				}
			else {
				output->appendString("elem.kind = 8;",0,0);
				output->appendString("\n",0,0);
				// kCondition stub
				}
			if ( label )
				{
				output->appendString("elem.label = \"",0,0);
				output->appendString(label,0,0);
				output->appendString("\";",0,0);
				output->appendString("\n",0,0);
				}
			if ( banged )
				{
				output->appendString("elem.banged = true;",0,0);
				output->appendString("\n",0,0);
				}
			if ( isIgnored )
				{
				output->appendString("elem.isIgnored = true;",0,0);
				output->appendString("\n",0,0);
				}
			if ( noSkip )
				{
				output->appendString("elem.noSkip = true;",0,0);
				output->appendString("\n",0,0);
				}
			output->appendString("currentAlt.elements.add((void*)elem);\n}",0,0);
			output->appendString("\n",0,0);
			return;
		default:
			::fprintf(stderr,"Element.generate: unknown kind %u\n",kind);
			return;
		}
	skipName = "";
	if ( skipSet )
		{
		if ( skipSet->name )
			skipName = skipSet->name;
		else
		if ( skipSet->specs )
			skipName = skipSet->specs;
		}
	output->appendString("addTest(",0,0);
	output->appendInt(kindNum,0,0);
	output->appendString(", \"",0,0);
	if ( kind == kChr(kind) )
		output->appendChar(chrChar,0,0);
	else
	if ( data )
		PLGset::printText(data,output);
	output->appendString("\", \"",0,0);
	output->appendString(label,0,0);
	output->appendString("\", ",0,0);
	output->appendInt(minimum,0,0);
	output->appendString(", ",0,0);
	output->appendInt(maximum,0,0);
	output->appendString(", \"",0,0);
	output->appendString(skipName,0,0);
	output->appendString("\");",0,0);
	output->appendString("\n",0,0);
	// Modifier follow-ups — addTest doesn't carry these directly
	if ( noSkip )
		{
		output->appendString("setNoSkip();",0,0);
		output->appendString("\n",0,0);
		}
	if ( banged )
		{
		output->appendString("setBanged();",0,0);
		output->appendString("\n",0,0);
		}
	if ( isIgnored )
		{
		output->appendString("setIgnored();",0,0);
		output->appendString("\n",0,0);
		}
}

PLGitem *Element::match(PLGparse *state)
{
char 		*savedCursor = 0;
PLGitem 	*result = 0;
	//trace?.enterElement(this, state);
	// Skip-set handling — done once, per element, before the kind-specific match
	if ( !noSkip )
		state->skip();
	// Guard check — fail fast if next char isn't in the FIRST set
	if ( guardSet && state->cursor < state->eof && !guardSet->contains(*state->cursor) )
		{
		//trace?.exitElement(this, state, null, "guarded");
		return 0;
		}
	savedCursor = state->cursor;
	result = matchByKind(state);
	// Apply repetition
	if ( minimum > 1 || maximum > 1 )
		result = applyRepetition(state,result);
	// Apply banged inversion — negative lookahead. If the sub-match
	// succeeded we must also REWIND the cursor back to where matchByKind
	// started; otherwise a zero-width "fail" still leaves cursor advanced
	// and breaks the next element's position assumption (this is the
	// root cause of `'Set'` greedy-matching the prefix of `SetVariable`).
	if ( banged )
		{
		if ( result )
			{
			state->cursor = savedCursor;
			result = 0;
			}
		else	result = PLGitem::itemEmpty;
		}
	// Apply isIgnored — match succeeded but discard the result
	if ( result && isIgnored )
		result = PLGitem::itemEmpty;
	// Apply label
	if ( result && label )
		result->itemLabel = label;
	//trace?.exitElement(this, state, result);
	return result;
}

PLGitem *Element::matchAny(PLGparse *state)
{
	if ( state->cursor >= state->eof )
		return 0;
PLGitem *result = new PLGitem(state->cursor,1);
	state->cursor++;
	return result;
}

PLGitem *Element::matchBalanced(PLGparse *state)
{
PLGitem 	*open = openElement->match(state);
	if ( !open )
		return 0;
char 		*bodyStart = state->cursor;
int 		depth = 1;
	while ( state->cursor < state->eof && depth > 0 )
		{
		char 		*saved = state->snapshot();
		PLGitem 	*nested = openElement->match(state);
		if ( nested )
			{
			depth++;
			continue;
			}
		state->restore(saved);
		PLGitem 	*close = closeElement->match(state);
		if ( close )
			{
			depth--;
			if ( depth == 0 )
				{
				PLGitem 	*result = new PLGitem(bodyStart,(int)(saved - bodyStart));
				result->terminator = close;
				return result;
				}
			continue;
			}
		state->restore(saved);
		state->cursor++;
		}
	// ordinary char inside body
	return 0;
	// unbalanced
}

PLGitem *Element::matchByKind(PLGparse *state)
{
	switch (kind)
		{
		case 1:
			return matchLit(state);
		case 2:
			return matchChr(state);
		case 3:
			return matchSet(state);
		case 4:
			return matchAny(state);
		case 5:
			return matchEof(state);
		case 6:
			return matchRuleRef(state);
		case 7:
			return matchKeyTable(state);
		case 8:
			return matchCondition(state);
		case 9:
			return matchVariable(state);
		case 10:
			return matchUpTo(state);
		case 11:
			return matchBalanced(state);
		default:
			::fprintf(stderr,"ERROR: unknown Element kind: %u\n",kind);
			return 0;
		}
}

PLGitem *Element::matchChr(PLGparse *state)
{
	if ( state->cursor >= state->eof || *state->cursor != chrChar )
		return 0;
PLGitem *result = new PLGitem(state->cursor,1);
	state->cursor++;
	return result;
}

PLGitem *Element::matchCondition(PLGparse *state)
{
PLGitem 	*result = PLGitem::itemEmpty;
	::printf("Warning: matchCondition is just a stub at this point\n");
	/*
	int ok = condFunc();
	PLGitem result = itemEmpty;
	result.conditionResult = ok;
	*/
	return result;
	// always "succeeds"; caller checks conditionResult
}

PLGitem *Element::matchEof(PLGparse *state)
{
	if ( state->cursor < state->eof )
		return 0;
	return PLGitem::itemEmpty;
}

PLGitem *Element::matchKeyTable(PLGparse *state)
{
KeyTableItem 	*hit = tableRef->matches(state->cursor);
	if ( !hit )
		return 0;
long 			len = ::strlen(hit->key);
PLGitem 		*result = new PLGitem(state->cursor,len);
	result->itemValue = (void*)hit;
	state->cursor += len;
	return result;
}

PLGitem *Element::matchLit(PLGparse *state)
{
char 	*at = state->cursor;
char 	*want = litText;
	while ( *want && at < state->eof && *at == *want )
		{
		at++;
		want++;
		}
	if ( *want )
		return 0;
	// didn't match the full literal
PLGitem *result = new PLGitem(state->cursor,at - state->cursor);
	state->cursor = at;
	return result;
}

PLGitem *Element::matchRuleRef(PLGparse *state)
{
	if ( state->debugRulePLG || ruleRef->debug )
		::printf("matchRuleRef: cursor: %s\n",state->cursor);
	return ruleRef->match(state);
}

PLGitem *Element::matchSet(PLGparse *state)
{
	if ( state->cursor >= state->eof )
		return 0;
	if ( !setRef->contains(*state->cursor) )
		return 0;
PLGitem *result = new PLGitem(state->cursor,1);
	state->cursor++;
	return result;
}

PLGitem *Element::matchUpTo(PLGparse *state)
{
char 	*saveStart = state->cursor;
	while ( state->cursor < state->eof )
		{
		// Handle escape: '\' followed by any char skips both
		if ( *state->cursor == '\\' && state->cursor + 1 < state->eof )
			{
			state->cursor += 2;
			continue;
			}
		char *posBefore = state->cursor;
		PLGitem *term = terminator->match(state);
		if ( term )
			{
			long 		len = posBefore - saveStart;
			PLGitem 	*result = new PLGitem(saveStart,len);
			result->terminator = term;
			if ( !consumeTerminator )
				state->cursor = posBefore;
			// leave terminator for next match
			return result;
			}
		state->cursor++;
		}
	state->cursor = saveStart;
	// didn't find it
	return 0;
}

PLGitem *Element::matchVariable(PLGparse *state)
{
char 	*want = *varPtr;
	if ( !want )
		return 0;
char 	*at = state->cursor;
	while ( *want && at < state->eof && *at == *want )
		{
		at++;
		want++;
		}
	if ( *want )
		return 0;
PLGitem *result = new PLGitem(state->cursor,at - state->cursor);
	state->cursor = at;
	return result;
}
