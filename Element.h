class PLGset;
class PLGrule;
class KeyTable;
class PLGitem;
class PLGparse;
class Buffer;
/*******************************************************************************
	Class that embodies the Element rule in plg
*******************************************************************************/

class Element
{
public:
char *label;
int minimum;
int maximum;
char *errorMessage;
PLGset *skipSet;
PLGset *guardSet;
struct 
	{
	unsigned int banged:1;
	unsigned int isIgnored:1;
	unsigned int noSkip:1;
	unsigned int processUpTo:1;
	unsigned int skipOverMatch:1;
	unsigned int kind:4;
	};
#define kLit(button) (button == 1)
#define kChr(button) (button == 2)
#define kSet(button) (button == 3)
#define kAny(button) (button == 4)
#define kEof(button) (button == 5)
#define kRuleRef(button) (button == 6)
#define kKeyTable(button) (button == 7)
#define kCondition(button) (button == 8)
#define kVariable(button) (button == 9)
#define kUpTo(button) (button == 10)
#define kBalanced(button) (button == 11)
char *litText;
char chrChar;
PLGset *setRef;
PLGrule *ruleRef;
KeyTable *tableRef;
void *condFunc;
char **varPtr;
Element *terminator;
struct 
	{
	unsigned int consumeTerminator:1;
	};
Element *openElement;
Element *closeElement;
PLGitem *applyRepetition(PLGparse *state, PLGitem *firstResult);
void generate(Buffer *output);
PLGitem *match(PLGparse *state);
PLGitem *matchAny(PLGparse *state);
PLGitem *matchBalanced(PLGparse *state);
PLGitem *matchByKind(PLGparse *state);
PLGitem *matchChr(PLGparse *state);
PLGitem *matchCondition(PLGparse *state);
PLGitem *matchEof(PLGparse *state);
PLGitem *matchKeyTable(PLGparse *state);
PLGitem *matchLit(PLGparse *state);
PLGitem *matchRuleRef(PLGparse *state);
PLGitem *matchSet(PLGparse *state);
PLGitem *matchUpTo(PLGparse *state);
PLGitem *matchVariable(PLGparse *state);
};
