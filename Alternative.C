#include <string.h>
#include <stdio.h>
#include "DoubleLinkList.h"
#include "DoubleLink.h"
#include "PLGitem.h"
#include "PLGset.h"
#include "BaseHash.h"
#include "Element.h"
#include "Buffer.h"
#include "PLGparse.h"
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
	Just a stub for now
*******************************************************************************/
void Alternative::generate(Buffer *output)
{
Element 	*elem = 0;
	output->appendString("{\nAlternative *alt = new Alternative();");
	output->appendString("\n");
	elements->resetIterator();
	while ( elem = (Element*)elements->next() )
		elem->generate(output);
	output->appendString("currentRule->addAlternative(alt);\n}");
	output->appendString("\n");
}

int Alternative::match(PLGparse *state, PLGitem *&out)
{
PLGitem 	*collected = new PLGitem();
DoubleLink 	*link = 0;
Element 	*elem = 0;
int 		result = 1;
	::printf("Alternative match elements: %d\n",elements->length);
	collected->itemStart = state->cursor;
	for ( link = elements->first; link; link = link->next )
		{
		elem = (Element*)link->value;
		::printf("\telement kind: %u\n",elem->kind);
		PLGitem *item = elem->match(state);
		if ( !item )
			{
			if ( elem->minimum > 0 )
				result = 0;
			break;
			}
		if ( item->itemLabel )
			{
			if ( !collected->children )
				collected->children = new BaseHash();
			collected->children->add(item->itemLabel,item);
			}
		}
	if ( result )
		{
		collected->itemLength = state->cursor - collected->itemStart;
		out = collected;
		}
	return result;
}
