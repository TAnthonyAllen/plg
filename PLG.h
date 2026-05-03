class PLGset;
class BaseHash;
class Buffer;
class Stak;
class PLGitem;
class PLGparse;
/*******************************************************************************
    PLG class definition
*******************************************************************************/

class PLG
{
public:
PLGset *alternateSet;
PLGset *characterSet;
PLGset *commandSet;
PLGset *commentSet;
PLGset *elementSet;
PLGset *excludeSet;
PLGset *nameSet;
PLGset *numberSet;
PLGset *singleQuote;
BaseHash *plgRuleTable;
BaseHash *plgSetTable;
Buffer *output;
Buffer *buffer;
Buffer *headerBuffer;
Buffer *includeBuffer;
Buffer *parseBuffer;
Stak *ruleStack;
Stak *testStack;
char *parentParser;
PLGitem *tail;
PLGitem *ruleActionName;
int alternateFlag;
int blockCount;
int optionCount;
int regexCount;
int testFlag;
struct 
	{
	unsigned int hasActions:1;
	unsigned int hasConditions:1;
	unsigned int hasKeys:1;
	unsigned int hasSets:1;
	unsigned int hasVariables:1;
	};
PLGparse *parser;
PLG();
PLG(char *input);
void init();
void process(char *filename);
void run();
void setRules();
};
int RuleplgNow(PLGparse *state, PLGitem *iTEM);
