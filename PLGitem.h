class BaseHash;
class DoubleLinkList;
class PLGrule;
class Alternative;
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
int amount;
struct 
	{
	unsigned int conditionResult:1;
	unsigned int flag1:1;
	unsigned int flag4:1;
	};
char savedChar;
DoubleLinkList *deferred;
PLGrule *deferRule;
Alternative *deferAlt;
static PLGitem *itemEmpty;
PLGitem();
PLGitem(char *s);
PLGitem(char *start, long length);
int compare(PLGitem *item);
int getAmount();
PLGitem *getLabel(char *name);
void runDeferred(PLGparse *state);
char *string();
char *toString();
void unString();
};
