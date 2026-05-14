/*****************************************************************************
	A class wrapper for regex functionality
*****************************************************************************/

class PLGrgx
{
public:
regex_t *model;
char *expression;
struct 
	{
	unsigned int caseSensitive:1;
	unsigned int compiled:1;
	unsigned int status:1;
	};
PLGrgx(char *xpr);
void compile();
int matches(char *text);
};
