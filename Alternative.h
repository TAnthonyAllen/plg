class DoubleLinkList;
class PLGset;
class PLGitem;
class PLGparse;
class Buffer;
/*******************************************************************************
	Class that embodies the Alternative rule in plg
*******************************************************************************/

class Alternative
{
public:
DoubleLinkList *elements;
PLGset *guardSet;
PLGitem *immediateAction;
PLGitem *deferAction;
char *actionName;
int (*immediate)(PLGparse *state, PLGitem *i);
void (*defer)(PLGparse *state, PLGitem *i);
Alternative();
void generate(char *parserName, Buffer *output);
int match(PLGparse *state, PLGitem *&out);
void writeActions(char *parserName, Buffer *output);
void writeCaptures(Buffer *output);
};
