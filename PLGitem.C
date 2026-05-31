#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "DoubleLinkList.h"
#include "PLGrule.h"
#include "DoubleLink.h"
#include "BaseHash.h"
#include "PLGparse.h"
#include "PLGitem.h"
PLGitem *PLGitem::itemEmpty;

/*****************************************************************************
	Constructors
*****************************************************************************/
PLGitem::PLGitem()
{
	itemLabel = 0;
	children = 0;
	terminator = 0;
	itemNext = 0;
	itemValue = (void*)0;
	amount = 0;
	conditionResult = 0;
	savedChar = 0;
	deferred = 0;
	deferRule = 0;
	itemStart = 0;
	itemLength = 0;
}

PLGitem::PLGitem(char *s)
{
	itemLabel = 0;
	children = 0;
	terminator = 0;
	itemNext = 0;
	itemValue = (void*)0;
	amount = 0;
	conditionResult = 0;
	savedChar = 0;
	deferred = 0;
	deferRule = 0;
	itemStart = s;
	itemLength = ::strlen(s);
}

PLGitem::PLGitem(char *start, long length)
{
	itemLabel = 0;
	children = 0;
	terminator = 0;
	itemNext = 0;
	itemValue = (void*)0;
	amount = 0;
	conditionResult = 0;
	savedChar = 0;
	deferred = 0;
	deferRule = 0;
	itemStart = start;
	itemLength = length;
}

/*****************************************************************************
	Compare this item to the item passed in.
*****************************************************************************/
int PLGitem::compare(PLGitem *item)
{
	if ( item )
		{
		int 	result = (int)(itemLength - item->itemLength);
		if ( !result )
			result = ::strncmp(itemStart,item->itemStart,itemLength);
		return result;
		}
	return 1;
}

/*****************************************************************************
    getAmount — the old PLGitem stored atoi() of a numeric match in an
    `amount` field. The new surface keeps only the matched text, so action
    code that wants the numeric value calls getAmount() to recompute it.
*****************************************************************************/
int PLGitem::getAmount()
{
	return ::atoi(toString());
}

/*****************************************************************************
    getLabel — direct accessor for a labeled child by name. Returns null
    on absent key. plg's simpler equivalent of incant's getLabelGroup;
    no wrapper-peel loop because PLGitem has no GROUP/Method/Rule
    distinctions to walk through. Consumed by labels-as-locals shorthand
    in .act splice content (via `external PLGitem { initializer getLabel; }`).
*****************************************************************************/
PLGitem *PLGitem::getLabel(char *name)
{
PLGitem 	*result = (PLGitem*)children->get(name);
	return result;
}

/*****************************************************************************
    runDeferred — walk this item's deferred list (each entry is a PLGitem
    whose deferRule field names the rule whose defer callback to fire).
    Cascade is built bottom-up: PLGrule.match adds (rule, result) to
    result.deferred when the rule has a defer callback; Alternative.match
    bubbles sub-item entries up into the collected item's deferred list.
*****************************************************************************/
void PLGitem::runDeferred(PLGparse *state)
{
DoubleLink 	*link = 0;
PLGitem 	*item = 0;
	if ( !deferred )
		return;
	for ( link = deferred->first; link; link = link->next )
		{
		item = (PLGitem*)link->value;
		if ( item && item->deferRule )
			item->deferRule->defer(state,item);
		}
}

/*****************************************************************************
	string returns a temporary string derived from the input stream this item
    was created in. It adds a null eof to end the string, changing the stream
*****************************************************************************/
char *PLGitem::string()
{
	// null terminate in place
	savedChar = *(itemStart + itemLength);
	*(itemStart + itemLength) = 0;
	return itemStart;
}

/*****************************************************************************
	Returns a copy of the item text
*****************************************************************************/
char *PLGitem::toString()
{
char 	*text = (char*)::malloc(itemLength + 1);
	::strncpy(text,itemStart,itemLength);
	*(text + itemLength) = 0;
	return text;
}

/*****************************************************************************
	unString resets the input stream modified by calling string()
*****************************************************************************/
void PLGitem::unString()
{
	if ( !savedChar )
		return;
	*(itemStart + itemLength) = savedChar;
	savedChar = 0;
}
