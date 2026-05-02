class BaseHash;
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
static PLGitem *itemEmpty;
PLGitem();
PLGitem(char *s);
PLGitem(char *start, long length);
char *string();
char *toString();
void unString();
};
