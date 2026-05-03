class BaseHash;
class DoubleLinkList;
class PLGrule;
class PLGparse;
/*******************************************************************************
	Class that embodies an item (like a PLGitem)
*******************************************************************************/

class PLGitem
{
public:
char *itemStart;
long itemLength;
char *itemLabel;
BaseHash *children;
PLGitem *terminator;
PLGitem *itemNext;
void *itemValue;
struct 
	{
	unsigned int conditionResult:1;
	};
char savedChar;
DoubleLinkList *deferred;
PLGrule *deferRule;
static PLGitem *itemEmpty;
PLGitem();
PLGitem(char *s);
PLGitem(char *start, long length);
void runDeferred(PLGparse *state);
char *string();
char *toString();
void unString();
};
