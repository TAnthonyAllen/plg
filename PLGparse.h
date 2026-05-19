class Buffer;
class PLGrule;
class Alternative;
class PLGset;
class Stak;
class BaseHash;
class PLGitem;
/*******************************************************************************
	PLGparse defines the environment for a test version of plg and provides
    the code to run it
*******************************************************************************/

class PLGparse
{
public:
Buffer *buffer;
char *cursor;
char *eof;
char *parserName;
PLGrule *currentRule;
Alternative *currentAlt;
PLGset *currentSet;
PLGset *defaultSkip;
PLGset *skipSet;
Stak *skipStack;
Stak *inputStack;
int depth;
int helperCount;
BaseHash *rules;
BaseHash *setTable;
BaseHash *keyWordTable;
BaseHash *conditionTable;
BaseHash *variableTable;
char *pendingLabel;
struct 
	{
	unsigned int pendingNoSkip:1;
	};
char *sourceDir;
PLGrule *bodyRule;
PLGset *alternateSet;
PLGset *characterSet;
PLGset *commandSet;
PLGset *commentSet;
PLGset *elementSet;
PLGset *excludeSet;
PLGset *nameSet;
PLGset *numberSet;
PLGset *singleQuote;
struct 
	{
	unsigned int debugRulePLG:1;
	unsigned int debugGuardPLG:1;
	unsigned int doNotGuard:1;
	unsigned int setsInitialized:1;
	unsigned int skipping:1;
	};
Buffer *spliceAccumulator;
Stak *actionNames;
PLGparse();
PLGparse(char *input);
void addTest(int kind, char *data, char *label, int min, int max, char *skipSet);
void divertInput(char *s);
void generateRules(Buffer *output, char *baseName);
PLGrule *getRule(char *name);
PLGset *getSet(char *specs);
PLGset *getSet(char *name, char *specs);
PLGset *getSet(PLGset *named, char *specs);
void initialize();
PLGitem *parse(PLGrule *rule);
PLGitem *parse(char *name);
char *parseActDeclarations(char *input);
void reportError(Alternative *alt, char *message);
void restore(char *s);
void revertInput();
void setBanged();
void setIgnored();
void setInput(char *s);
void setInput(PLGitem *item);
void setNoSkip();
void setSkip();
void skip();
char *snapshot();
char *stripComments(char *input);
void summary(int threshold);
};
extern "C" int foundIn(PLGset *set, PLGitem *item);
