class DoubleLinkList;
class PLGset;
class PLGparse;
class PLGitem;
class Alternative;
class Buffer;
/*******************************************************************************
	Class that embodies a Rule in plg
*******************************************************************************/

class PLGrule
{
public:
char *name;
DoubleLinkList *alternatives;
PLGset *guardSet;
PLGparse *parentParser;
void (*defer)(PLGparse *state, PLGitem *i);
int (*immediate)(PLGparse *state, PLGitem *i);
PLGitem *deferAction;
PLGitem *failMethod;
PLGitem *immediateAction;
struct 
	{
	unsigned int balanceRule:1;
	unsigned int balanceAny:1;
	unsigned int balanceBody:1;
	unsigned int balanceBail:1;
	unsigned int doNotGuard:1;
	unsigned int debug:1;
	unsigned int guardComputed:1;
	};
PLGrule(char *s);
void addAlternative(Alternative *alt);
void declareActions(Buffer *output);
void generate(Buffer *output);
PLGitem *match(PLGparse *state);
PLGset *setGuard();
void writeActions(char *parserName, Buffer *output);
};
