#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "regex.h"
#include "PLGrgx.h"

PLGrgx::PLGrgx(char *xpr)
{
	compiled = 0;
	status = 0;
	expression = xpr;
	caseSensitive = 1;
	model = (regex_t*)::calloc(1,sizeof(regex_t));
}

void PLGrgx::compile()
{
int 	flags = 0;
char 	*errorText = 0;
	if ( compiled )
		return;
	if ( caseSensitive )
		flags = REG_EXTENDED | REG_NOSUB;
	else	flags = REG_EXTENDED | REG_NOSUB | REG_ICASE;
	status = ::regcomp(model,expression,flags);
	if ( status )
		{
		errorText = (char*)::malloc(200);
		::regerror(status,model,errorText,200);
		::fprintf(stderr,"%s\n",errorText);
		}
	else	compiled = 1;
}

/*****************************************************************************
	Checks if the string passed in matches the pattern
*****************************************************************************/
int PLGrgx::matches(char *text)
{
	if ( text )
		{
		compile();
		if ( !status )
			return !::regexec(model,text,0,0,0);
		}
	return 0;
}
