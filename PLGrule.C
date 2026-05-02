#include <string.h>
#include <stdio.h>
#include "DoubleLinkList.h"
#include "Alternative.h"
#include "DoubleLink.h"
#include "PLGitem.h"
#include "PLGset.h"
#include "Buffer.h"
#include "PLGparse.h"
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
	::printf("PLGrule: %s at: %s\n",name,state->cursor);
	for ( link = alternatives->first; link; link = link->next )
		{
		alt = (Alternative*)link->value;
		if ( alt->match(state,result) )
			return result;
		}
	return 0;
}

/*****************************************************************************
	Set the guard sets for this rule
*****************************************************************************/
PLGset *PLGrule::setGuard()
{
	// needs rewriting
	return 0;
}

/*****************************************************************************
	This method writes out the actions associated with this rule.
*****************************************************************************/
void PLGrule::writeActions(char *parserName, Buffer *output)
{
	// TODO: implement
}
