#include <string.h>
#include <stdio.h>
#include "StringRoutines.h"
#include "DoubleLinkList.h"
#include "PLGrule.h"
#include "DoubleLink.h"
#include "BaseHash.h"
#include "Element.h"
#include "Buffer.h"
#include "PLGparse.h"
#include "PLGitem.h"
#include "PLGset.h"
#include "Alternative.h"

/*******************************************************************************
	Alternative constructor
*******************************************************************************/
Alternative::Alternative()
{
	guardSet = 0;
	elements = new DoubleLinkList();
}

/*******************************************************************************
	Emit an alternative in addTest-compatible form: assign currentAlt, let
	each element emit its own addTest()/setX() lines, then attach currentAlt
	to currentRule. Matches the hand-written setRules() convention so
	generated output can be pasted in directly.
*******************************************************************************/
void Alternative::generate(Buffer *output)
{
Element 	*elem = 0;
	output->appendString("currentAlt = new Alternative();",0,0);
	output->appendString("\n",0,0);
	elements->resetIterator();
	while ( elem = (Element*)elements->next() )
		elem->generate(output);
	output->appendString("currentRule.alternatives += currentAlt;",0,0);
	output->appendString("\n",0,0);
}

int Alternative::match(PLGparse *state, PLGitem *&out)
{
PLGitem 	*collected = new PLGitem();
DoubleLink 	*link = 0;
DoubleLink 	*dlink = 0;
Element 	*elem = 0;
int 		result = 1;
int 		elemIdx = 0;
	collected->itemStart = state->cursor;
	for ( link = elements->first; link; link = link->next )
		{
		char 	*what = "?";
		elem = (Element*)link->value;
		elemIdx++;
		// Verbose per-element trace: index, kind, target, cursor before.
		// For kRuleRef/kLit we print the ref-name or literal so the trace
		// tells us which sub-match was attempted at which offset.
		// NOTE: kRuleRef etc. are TEST macros not values (CLAUDE.md TAWK
		// quirks) — use numeric kind values for assignment/comparison.
		if ( elem->kind == 6 && elem->ruleRef )
			what = elem->ruleRef->name;
		else
		if ( elem->kind == 1 && elem->litText )
			what = elem->litText;
		PLGitem *item = elem->match(state);
		if ( !item )
			{
			// Banged elements (`!`, min=-1) ARE required even though their
			// minimum looks optional — null from a banged element means
			// the negative lookahead saw what it was excluding, so the
			// alternative must fail.
			if ( elem->minimum > 0 || elem->banged )
				{
				result = 0;
elementFail:
				if ( state->debugRulePLG )
					{
					::indent(StringRoutines::debugIndent,"  ",0);
					::printf("elem[%d] failed",elemIdx);
					if ( elem->label )
						::printf(" label: %s",elem->label);
					else	::printf(" w/no label");
					::printf(" at: %s\n",::headToCount(state->cursor,10));
					}
				break;
				}
			// optional element didn't match — keep going, cursor unchanged
			}
		else {
elementSuccess:
			if ( state->debugRulePLG )
				{
				::indent(StringRoutines::debugIndent,"  ",0);
				::printf("elem[%d] succeeded",elemIdx);
				if ( item->itemLabel )
					::printf(" label: %s",item->itemLabel);
				else	::printf(" w/no label");
				::printf(" at: %s\n",::headToCount(state->cursor,10));
				}
			if ( item->itemLabel )
				{
				if ( !collected->children )
					collected->children = new BaseHash();
				collected->children->add(item->itemLabel,item);
				}
			// Cascade deferred entries up — each sub-match may have
			// accumulated rule defers that need to fire after the
			// enclosing rule completes.
			if ( item->deferred )
				{
				if ( !collected->deferred )
					collected->deferred = new DoubleLinkList();
				for ( dlink = item->deferred->first; dlink; dlink = dlink->next )
					collected->deferred->add(dlink->value);
				}
			}
		}
	if ( result )
		{
		collected->itemLength = state->cursor - collected->itemStart;
		out = collected;
		}
	return result;
}
