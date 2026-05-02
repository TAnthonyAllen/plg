#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "BaseHash.h"
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
	conditionResult = 0;
	savedChar = 0;
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
	conditionResult = 0;
	savedChar = 0;
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
	conditionResult = 0;
	savedChar = 0;
	itemStart = start;
	itemLength = length;
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
