class Buffer;
class PLGrule;
class PLGset;
class Stak;
class BaseHash;
class Alternative;
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
PLGset *currentSet;
PLGset *defaultSkip;
PLGset *skipSet;
Stak *skipStack;
Stak *inputStack;
int depth;
BaseHash *rules;
BaseHash *setTable;
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
Alternative *currentAlt;
PLGparse();
PLGparse(char *input);
void addTest(int kind, char *data, char *label, int min, int max, char *skipSet);
void divertInput(char *s);
void generateRules(Buffer *output);
PLGrule *getRule(char *name);
PLGset *getSet(char *specs);
PLGset *getSet(char *name, char *specs);
PLGset *getSet(PLGset *named, char *specs);
void initialize();
PLGitem *parse(char *name);
PLGitem *parse(PLGrule *rule);
void reportError(Alternative *alt, char *message);
void restore(char *s);
void revertInput();
void setInput(char *s);
void setInput(PLGitem *item);
void setSkip();
void skip();
char *snapshot();
};
int main();
