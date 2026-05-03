#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "StringRoutines.h"
#include "DoubleLinkList.h"
#include "PLGrule.h"
#include "Alternative.h"
#include "DoubleLink.h"
#include "PLGset.h"
#include "Stak.h"
#include "BaseHash.h"
#include "Buffer.h"
#include "PLGparse.h"
#include "PLGitem.h"
#include "PLG.h"

/*******************************************************************************
	AlternativeplgAct — fires after an Alternative meta-rule match completes
	(deferred). Creates a new C++ Alternative object, appends it to the
	currentRule (set up by RuleplgNow), and parks it on state.currentAlt so
	a future ElementplgAct can populate its elements list.
*******************************************************************************/
void AlternativeplgAct(PLGparse *state, PLGitem *iTEM)
{
Alternative 	*alt = 0;
	if ( !state->currentRule )
		{
		::fprintf(stderr,"AlternativeplgAct: no currentRule — RuleplgNow must run first\n");
		return;
		}
	alt = new Alternative();
	state->currentRule->addAlternative(alt);
	state->currentAlt = alt;
	::printf("AlternativeplgAct fired: added empty alt to rule '%s'\n",state->currentRule->name);
}

/*******************************************************************************
	RuleOptionsplgAct — fires after a RuleOptions match completes (deferred).
	Stub for now; real implementation will populate the current rule's
	alternatives from the captured RuleOption sub-items.
*******************************************************************************/
void RuleOptionsplgAct(PLGparse *state, PLGitem *iTEM)
{
	::printf("RuleOptionsplgAct fired\n");
}

/*******************************************************************************
	RuleplgNow — immediate action fired when a Rule alternative matches in
	Testing.g (or any .g file the parser is processing). Looks up the
	"ruleName" capture, calls state.getRule() so a fresh rule entry is
	created in parser.rules, then cascades the deferred action chain
	(RuleOptionsplgAct etc.) for the rule's body.
*******************************************************************************/
int RuleplgNow(PLGparse *state, PLGitem *iTEM)
{
PLGitem 	*ruleName = 0;
char 		*name = 0;
	if ( !iTEM || !iTEM->children )
		return 0;
	ruleName = (PLGitem*)iTEM->children->get("ruleName");
	if ( !ruleName )
		return 0;
	name = ruleName->toString();
	::printf("RuleplgNow: matched rule '%s'\n",name);
	state->currentRule = state->getRule(name);
	iTEM->runDeferred(state);
	return 1;
}

/*******************************************************************************
	PLG constructors
*******************************************************************************/
PLG::PLG()
{
	alternateSet = 0;
	characterSet = 0;
	commandSet = 0;
	commentSet = 0;
	elementSet = 0;
	excludeSet = 0;
	nameSet = 0;
	numberSet = 0;
	singleQuote = 0;
	plgRuleTable = 0;
	plgSetTable = 0;
	output = 0;
	buffer = 0;
	headerBuffer = 0;
	includeBuffer = 0;
	parseBuffer = 0;
	ruleStack = 0;
	testStack = 0;
	parentParser = 0;
	tail = 0;
	ruleActionName = 0;
	alternateFlag = 0;
	blockCount = 0;
	optionCount = 0;
	regexCount = 0;
	testFlag = 0;
	hasActions = 0;
	hasConditions = 0;
	hasKeys = 0;
	hasSets = 0;
	hasVariables = 0;
	parser = 0;
	init();
}

PLG::PLG(char *input)
{
	alternateSet = 0;
	characterSet = 0;
	commandSet = 0;
	commentSet = 0;
	elementSet = 0;
	excludeSet = 0;
	nameSet = 0;
	numberSet = 0;
	singleQuote = 0;
	plgRuleTable = 0;
	plgSetTable = 0;
	output = 0;
	buffer = 0;
	headerBuffer = 0;
	includeBuffer = 0;
	parseBuffer = 0;
	ruleStack = 0;
	testStack = 0;
	parentParser = 0;
	tail = 0;
	ruleActionName = 0;
	alternateFlag = 0;
	blockCount = 0;
	optionCount = 0;
	regexCount = 0;
	testFlag = 0;
	hasActions = 0;
	hasConditions = 0;
	hasKeys = 0;
	hasSets = 0;
	hasVariables = 0;
	parser = 0;
	init();
	parser->setInput(input);
}

void PLG::init()
{
	parser = new PLGparse();
	output = ::bufferFactory3("plgOutput",10000);
	buffer = ::bufferFactory2("plgBuffer");
	headerBuffer = ::bufferFactory2("plgHeader");
	includeBuffer = ::bufferFactory2("plgInclude");
	parseBuffer = ::bufferFactory2("plgParse");
	parser->parserName = "PLG";
	ruleStack = new Stak();
	testStack = new Stak();
	parser = new PLGparse();
}

/*******************************************************************************
	process — load a grammar file, install fresh rule/set tables so action
	callbacks can populate them during parse, run the parser starting at the
	"Start" rule, dump what landed in the fresh tables, then emit a .twk
	file via generateRules() (currently still using the meta-grammar tables).
*******************************************************************************/
void PLG::process(char *filename)
{
char 		*content = 0;
PLGitem 	*result = 0;
Buffer 		*twkOut = 0;
char 		*outFile = 0;
char 		*dot = 0;
BaseHash 	*metaRules = 0;
BaseHash 	*metaSetTable = 0;
	setRules();
	parser->initialize();
	content = ::getStringFromFile(filename);
	if ( !content )
		{
		::fprintf(stderr,"process: could not load %s\n",filename);
		return;
		}
	// Two-table pattern: save meta-grammar (the rules that *parse* .g files),
	// install fresh empty tables for action callbacks to populate during the
	// parse of `content`. Restore meta-grammar after so generateRules emits
	// the meta-grammar (until we wire up the rest of the callback chain).
	// Grab the Start rule reference BEFORE swapping so we can parse via
	// the direct rule pointer instead of a name-lookup against the fresh
	// (empty) table.
PLGrule 	*startRule = (PLGrule*)parser->rules->get("Start");
	if ( !startRule )
		{
		::fprintf(stderr,"process: no Start rule in meta-grammar\n");
		return;
		}
	metaRules = parser->rules;
	metaSetTable = parser->setTable;
	parser->rules = new BaseHash();
	parser->setTable = new BaseHash();
	parser->setInput(content);
	result = parser->parse(startRule);
	if ( result )
		::printf("parsed %ld chars from %s\n",result->itemLength,filename);
	else	::printf("parse failed on %s\n",filename);
	::printf("=== fresh rules table after parse (callback-populated) ===\n");
	parser->rules->listKeys();
	::printf("=== fresh setTable after parse ===\n");
	parser->setTable->listKeys();
	::printf("=== end fresh tables ===\n");
	// Restore meta-grammar tables for generateRules.
	parser->rules = metaRules;
	parser->setTable = metaSetTable;
	// Build output path: foo.g -> foo.twk
	outFile = (char*)::malloc(::strlen(filename) + 8);
	::strcpy(outFile,filename);
	dot = strrchr(outFile,'.');
	if ( dot )
		{
		*dot = '\0';
		}
	::strcat(outFile,".twk");
	twkOut = ::bufferFactory3("twk-output",50000);
	parser->generateRules(twkOut);
	twkOut->setFile(outFile);
	twkOut->flush();
	::printf("wrote %s\n",outFile);
	::free(outFile);
}

/*******************************************************************************
	Method to test the parse
*******************************************************************************/
void PLG::run()
{
	setRules();
	parser->initialize();
	parser->setInput(",678");
PLGitem *result = parser->parse("Max");
	if ( result )
		::printf("matched %ld chars: %s\n",result->itemLength,result->toString());
	else	::printf("no match\n");
}

/*******************************************************************************
	setRules copied over from plg.twk
*******************************************************************************/
void PLG::setRules()
{
	parser->setSkip();
	alternateSet = parser->getSet("alternateSet","#([~'a-zA-Z");
	characterSet = parser->getSet("characterSet","[a-zA-Z");
	commandSet = parser->getSet("commandSet","derst");
	commentSet = parser->getSet("commentSet","derst/");
	elementSet = parser->getSet("elementSet","[(a-zA-Z~'");
	excludeSet = parser->getSet("excludeSet","A-Za-z:.|");
	nameSet = parser->getSet("nameSet","a-zA-Z");
	numberSet = parser->getSet("numberSet","0-9");
	singleQuote = parser->getSet("singleQuote","'");
	parser->currentRule = parser->getRule("Integer");
	parser->currentAlt = new Alternative();
	parser->addTest(3," \f\r\t\n","",0,999999,"");
	parser->addTest(3,"0-9","",1,999999,"");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentRule = parser->getRule("Max");
	parser->currentAlt = new Alternative();
	parser->addTest(1,",","",1,1,"defaultSKIP");
	parser->addTest(6,"Integer","maximum",1,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentRule = parser->getRule("Name");
	parser->currentSet = parser->getSet("._a-zA-Z0-9");
	parser->currentAlt = new Alternative();
	parser->addTest(3,"._a-zA-Z0-9","name",1,999999,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentRule = parser->getRule("SetAssignment");
	parser->currentAlt = new Alternative();
	parser->addTest(1,"=","",1,1,"defaultSKIP");
	parser->addTest(6,"SetName","set",1,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentRule = parser->getRule("BalanceRight");
	//currentRule.defer = BalanceRightplgAct;
	parser->currentAlt = new Alternative();
	parser->addTest(1,"<>","",1,1,"defaultSKIP");
	parser->addTest(6,"Element","element",1,1,"defaultSKIP");
	parser->addTest(6,"ElementType","type",0,1,"defaultSKIP");
	parser->addTest(1,"&","noSkip",0,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentRule = parser->getRule("Command");
	//currentRule.immediate = CommandplgNow;
	//currentRule.next = getRule("Command2");
	parser->currentAlt = new Alternative();
	parser->addTest(1,"re","reset",0,1,"defaultSKIP");
	parser->addTest(1,"setSKIP","",1,1,"defaultSKIP");
	parser->addTest(6,"SetAssignment","set",0,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentAlt = new Alternative();
	parser->addTest(1,"dEBUG","",1,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentRule = parser->getRule("FileName");
	//currentRule.immediate = FileNameplgNow;
	parser->currentSet = parser->getSet("A-Za-z0-9_");
	parser->currentAlt = new Alternative();
	parser->addTest(1,"/","",0,1,"defaultSKIP");
	parser->addTest(6,"FileNameBlock0","",0,999999,"defaultSKIP");
	parser->addTest(3,"A-Za-z0-9_","name",1,999999,"defaultSKIP");
	parser->addTest(1,".g","",1,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentRule = parser->getRule("Header");
	//currentRule.immediate = HeaderplgNow;
	parser->currentRule->doNotGuard = 1;
	parser->currentAlt = new Alternative();
	parser->addTest(1,"%%","head",0,1,"");
	parser->addTest(6,"Extender","name",0,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentRule = parser->getRule("ActionEnd");
	parser->currentSet = parser->getSet(".|;");
	parser->currentAlt = new Alternative();
	parser->addTest(1,"\n","",1,1,"");
	parser->addTest(3,".|;","",1,1,"");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentRule = parser->getRule("SetName");
	//currentRule.immediate = SetNameplgNow;
	parser->currentAlt = new Alternative();
	parser->addTest(6,"Name","set",1,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentRule = parser->getRule("Tail");
	//currentRule.immediate = TailplgNow;
	parser->currentAlt = new Alternative();
	parser->addTest(1,"%%","",1,1,"defaultSKIP");
	parser->addTest(4,"","code",1,999999,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentRule = parser->getRule("Alpha");
	parser->currentSet = parser->getSet("a-zA-Z0-9_.");
	parser->currentAlt = new Alternative();
	parser->addTest(3,"a-zA-Z0-9_.","",1,999999,"");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentRule = parser->getRule("TrailingSpace");
	parser->currentSet = parser->getSet("t");
	parser->currentAlt = new Alternative();
	parser->addTest(3,"t","",0,999999,"");
	parser->addTest(1,"\n","",1,1,"");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentRule = parser->getRule("CommentPart");
	//currentRule.next = getRule("CommentPart2");
	parser->currentAlt = new Alternative();
	parser->addTest(6,"CommentPartBalancE","",1,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentAlt = new Alternative();
	parser->addTest(1,"//","",1,1,"defaultSKIP");
	parser->addTest(1,"\n","",0,1,"");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentRule = parser->getRule("CommentPartBalancE");
	//currentRule.immediate = balancE;
	parser->currentAlt = new Alternative();
	parser->addTest(1,"/*","start",1,1,"defaultSKIP");
	parser->addTest(6,"CommentPartBoDY","body",0,999999,"defaultSKIP");
	parser->addTest(1,"*/","finish",1,1,"");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentRule = parser->getRule("DebugTextBalancE");
	//currentRule.immediate = balancE;
	parser->currentSet = singleQuote;
	parser->currentSet = singleQuote;
	parser->currentAlt = new Alternative();
	parser->addTest(3,"'","start",1,1,"defaultSKIP");
	parser->addTest(6,"DebugTextBoDY","body",0,999999,"defaultSKIP");
	parser->addTest(3,"'","finish",1,1,"");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentRule = parser->getRule("CommentPartBoDY");
	//currentRule.immediate = balancEbody;
	//currentRule.next = getRule("CommentPartAny");
	parser->currentAlt = new Alternative();
	parser->addTest(1,"/*","begin",0,1,"");
	parser->addTest(1,"*/","end",0,1,"");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentAlt = new Alternative();
	parser->addTest(4,"","",1,1,"");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentRule = parser->getRule("QuotedStringBlock1Block2");
	// Source had `currentSet = getSet("'"); saveTest.setAlternate(currentTest);`
	// — alternation between escape-sequence and any-non-quote-char.
	// Inlined as two alts; second uses negated set "^'" to mean "any char
	// except '".
	parser->currentAlt = new Alternative();
	parser->addTest(6,"QuotedStringBlock1Block3","",1,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentAlt = new Alternative();
	parser->addTest(3,"^'","",1,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentRule = parser->getRule("Comment");
	parser->currentSet = parser->getSet("/");
	parser->currentRule->guardSet = parser->currentSet;
	//currentRule.next = getRule("Comment2");
	parser->currentAlt = new Alternative();
	parser->addTest(6,"CommentPart","",1,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentAlt = new Alternative();
	parser->addTest(6,"Command","",1,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentRule = parser->getRule("Balanced");
	//currentRule.defer = BalancedplgAct;
	parser->currentAlt = new Alternative();
	parser->addTest(6,"Element","left",1,1,"defaultSKIP");
	parser->addTest(6,"ElementType","type",0,1,"defaultSKIP");
	parser->addTest(1,"&","noSkip",0,1,"defaultSKIP");
	parser->addTest(6,"BalanceRight","right",0,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentRule = parser->getRule("Extender");
	parser->currentAlt = new Alternative();
	parser->addTest(1,"eXTENDS","",1,1,"defaultSKIP");
	parser->addTest(6,"Name","name",1,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentRule = parser->getRule("OptionClause");
	parser->currentAlt = new Alternative();
	parser->addTest(1,"|","",1,1,"defaultSKIP");
	parser->addTest(6,"RuleOption","dummy",1,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentRule = parser->getRule("FileNameBlock0");
	parser->currentAlt = new Alternative();
	parser->addTest(6,"Alpha","",1,1,"defaultSKIP");
	parser->addTest(1,"/","",1,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentRule = parser->getRule("Integer");
	parser->currentSet = parser->getSet("n");
	parser->currentSet = parser->getSet("0-9");
	parser->currentAlt = new Alternative();
	parser->addTest(3,"0-9","",0,999999,"defaultSKIP");
	parser->addTest(3,"0-9","",1,999999,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentRule = parser->getRule("DebugRule");
	parser->currentAlt = new Alternative();
	parser->addTest(6,"Name","name",1,1,"defaultSKIP");
	parser->addTest(6,"DebugOption","option",0,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentRule = parser->getRule("Label");
	parser->currentAlt = new Alternative();
	parser->addTest(6,"Name","atLabel",1,1,"defaultSKIP");
	parser->addTest(1,"=","",1,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentRule = parser->getRule("QuotedString");
	parser->currentAlt = new Alternative();
	parser->addTest(3,"'","",1,1,"defaultSKIP");
	parser->addTest(6,"QuotedStringBlock1","text",1,1,"defaultSKIP");
	parser->addTest(3,"'","",1,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentAlt = new Alternative();
	parser->addTest(3,".$","text",1,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentRule = parser->getRule("QuotedStringBlock1");
	parser->currentAlt = new Alternative();
	parser->addTest(6,"QuotedStringBlock1Block2","",1,999999,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentRule = parser->getRule("QuotedStringBlock1Block3");
	parser->currentAlt = new Alternative();
	parser->addTest(1,"\\","",1,1,"defaultSKIP");
	parser->addTest(4,"","",1,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentRule = parser->getRule("Word");
	parser->currentSet = parser->getSet("n;");
	parser->currentAlt = new Alternative();
	parser->addTest(3,"n;","what",1,999999,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentRule = parser->getRule("ActionOption");
	//currentRule.defer = ActionOptionplgAct;
	//currentRule.next = getRule("ActionOption2");
	parser->currentAlt = new Alternative();
	parser->addTest(1,"|","option",1,999999,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentAlt = new Alternative();
	parser->addTest(1,".","immediate",0,1,"");
	parser->addTest(1,";","",-1,1,"");
	parser->addTest(6,"ActionBody","action",1,1,"");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentRule = parser->getRule("ActionRule");
	//currentRule.immediate = ActionRuleplgNow;
	parser->currentAlt = new Alternative();
	parser->addTest(6,"Name","ruleName",1,1,"defaultSKIP");
	parser->addTest(1,"!","",1,1,"defaultSKIP");
	parser->addTest(6,"ActionOption","list",1,999999,"defaultSKIP");
	parser->addTest(1,";","",1,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentRule = parser->getRule("ActionBody");
	parser->currentAlt = new Alternative();
	parser->addTest(6,"ActionEnd","",-1,1,"");
	parser->addTest(6,"ActionEnd","body",0,1,"");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentRule = parser->getRule("Alternative");
	parser->currentRule->defer = ::AlternativeplgAct;
	// (was: FAIL AlternativeBlock4 — inlined as two alts since
	//  AlternativeBlock4 was an anonymous alternation Name|QuotedString)
	parser->currentAlt = new Alternative();
	parser->addTest(1,"FAIL","",1,1,"defaultSKIP");
	parser->addTest(6,"Name","exception",1,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentAlt = new Alternative();
	parser->addTest(1,"FAIL","",1,1,"defaultSKIP");
	parser->addTest(6,"QuotedString","exception",1,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	// (was: Guard? Label? Alternative2Block5 — inlined as two alts since
	//  Alternative2Block5 was an anonymous alternation Balanced|Block)
	parser->currentAlt = new Alternative();
	parser->addTest(6,"Guard","guard",0,1,"defaultSKIP");
	parser->addTest(6,"Label","atLabel",0,1,"defaultSKIP");
	parser->addTest(6,"Balanced","atElement",1,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentAlt = new Alternative();
	parser->addTest(6,"Guard","guard",0,1,"defaultSKIP");
	parser->addTest(6,"Label","atLabel",0,1,"defaultSKIP");
	parser->addTest(6,"Block","atElement",1,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentAlt = new Alternative();
	parser->addTest(6,"Comment","",1,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentRule = parser->getRule("Block");
	//currentRule.defer = BlockplgAct;
	parser->currentAlt = new Alternative();
	parser->addTest(1,"(","",1,1,"defaultSKIP");
	parser->addTest(6,"Alternative","atAlternative",1,999999,"defaultSKIP");
	parser->addTest(6,"Clause","clause",0,999999,"defaultSKIP");
	parser->addTest(1,")","",1,1,"defaultSKIP");
	parser->addTest(6,"ElementType","type",0,1,"defaultSKIP");
	parser->addTest(1,"&","noSkip",0,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentRule = parser->getRule("Clause");
	//currentRule.defer = ClauseplgAct;
	parser->currentAlt = new Alternative();
	parser->addTest(1,"|","",1,1,"defaultSKIP");
	parser->addTest(6,"Alternative","atAlternative",1,999999,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentRule = parser->getRule("Element");
	//currentRule.immediate = ElementplgNow;
	//currentRule.defer = ElementplgAct;
	//currentRule.next = getRule("Element2");
	parser->currentAlt = new Alternative();
	parser->addTest(6,"Command","",-1,1,"defaultSKIP");
	parser->addTest(6,"Name","atElement",1,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentAlt = new Alternative();
	parser->addTest(6,"QuotedString","atElement",1,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentAlt = new Alternative();
	parser->addTest(6,"StringSet","atElement",1,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentAlt = new Alternative();
	parser->addTest(1,"~","",1,1,"defaultSKIP");
	parser->addTest(3,".$","atElement",1,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentRule = parser->getRule("ElementType");
	//currentRule.defer = ElementTypeplgAct;
	//currentRule.next = getRule("ElementType2");
	parser->currentAlt = new Alternative();
	parser->addTest(1,"","",1,1,"defaultSKIP");
	parser->addTest(6,"Integer","minimum",1,1,"defaultSKIP");
	parser->addTest(6,"Max","maximum",0,1,"defaultSKIP");
	parser->addTest(1,"","",1,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentAlt = new Alternative();
	parser->addTest(3,"*+?!%","option",1,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentRule = parser->getRule("ForwardReference");
	//currentRule.immediate = ForwardReferenceplgNow;
	parser->currentAlt = new Alternative();
	parser->addTest(6,"Name","name",1,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentRule = parser->getRule("Guard");
	//currentRule.defer = GuardplgAct;
	//currentRule.next = getRule("Guard2");
	parser->currentAlt = new Alternative();
	parser->addTest(1,"#","",1,1,"defaultSKIP");
	parser->addTest(6,"StringSet","set",1,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentAlt = new Alternative();
	parser->addTest(1,"#doNotGuard","",1,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentRule = parser->getRule("Include");
	//currentRule.immediate = IncludeplgNow;
	//currentRule.next = getRule("Include2");
	parser->currentAlt = new Alternative();
	parser->addTest(1,"include(","",1,1,"defaultSKIP");
	parser->addTest(1,")","file",0,1,"");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentAlt = new Alternative();
	parser->addTest(1,"insert(","",1,1,"defaultSKIP");
	parser->addTest(1,")","file",0,1,"");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentAlt = new Alternative();
	parser->addTest(1,"-%","",1,1,"defaultSKIP");
	parser->addTest(1,"%-","code",0,1,"");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentRule = parser->getRule("ParseInclude");
	parser->currentAlt = new Alternative();
	parser->addTest(6,"Body","",1,999999,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentRule = parser->getRule("DebugTextBoDY");
	//currentRule.immediate = balancEbody;
	parser->currentSet = singleQuote;
	parser->currentSet = singleQuote;
	//currentRule.next = getRule("DebugTextAny");
	parser->currentAlt = new Alternative();
	parser->addTest(3,"'","begin",0,1,"");
	parser->addTest(3,"'","end",0,1,"");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentAlt = new Alternative();
	parser->addTest(4,"","",1,1,"");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentRule = parser->getRule("Rule");
	parser->currentRule->immediate = ::RuleplgNow;
	parser->currentAlt = new Alternative();
	parser->addTest(6,"Name","ruleName",1,1,"defaultSKIP");
	parser->addTest(1,":","",1,1,"defaultSKIP");
	parser->addTest(6,"RuleOptions","options",1,1,"defaultSKIP");
	parser->addTest(1,";","",1,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentRule = parser->getRule("RuleOption");
	//currentRule.defer = RuleOptionplgAct;
	parser->currentRule->doNotGuard = 1;
	parser->currentAlt = new Alternative();
	parser->addTest(6,"Alternative","atAlternative",1,999999,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentRule = parser->getRule("RuleOptions");
	parser->currentRule->defer = ::RuleOptionsplgAct;
	parser->currentAlt = new Alternative();
	parser->addTest(6,"RuleOption","first",1,1,"defaultSKIP");
	parser->addTest(6,"OptionClause","others",0,999999,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentRule = parser->getRule("SetVariable");
	//currentRule.immediate = SetVariableplgNow;
	parser->currentSet = excludeSet;
	//currentRule.next = getRule("SetVariable2");
	parser->currentAlt = new Alternative();
	parser->addTest(1,"Variable","",1,1,"defaultSKIP");
	parser->addTest(3,"A-Za-z:.|","",-1,1,"");
	parser->addTest(6,"Name","variable",1,999999,"defaultSKIP");
	parser->addTest(1,";","",1,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentAlt = new Alternative();
	parser->addTest(1,"Rules","",1,1,"defaultSKIP");
	parser->addTest(3,"A-Za-z:.|","",-1,1,"");
	parser->addTest(6,"ForwardReference","",1,999999,"defaultSKIP");
	parser->addTest(1,";","",1,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentAlt = new Alternative();
	parser->addTest(1,"KeyWord","",1,1,"defaultSKIP");
	parser->addTest(3,"A-Za-z:.|","",-1,1,"");
	parser->addTest(6,"Name","name",1,1,"defaultSKIP");
	parser->addTest(6,"Word","list",0,999999,"defaultSKIP");
	parser->addTest(1,";","",1,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentAlt = new Alternative();
	parser->addTest(1,"Set","",1,1,"defaultSKIP");
	parser->addTest(3,"A-Za-z:.|","",-1,1,"");
	parser->addTest(6,"Name","name",1,1,"defaultSKIP");
	parser->addTest(6,"StringSet","set",0,1,"defaultSKIP");
	parser->addTest(1,";","",0,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentAlt = new Alternative();
	parser->addTest(1,"Condition","",1,1,"defaultSKIP");
	parser->addTest(3,"A-Za-z:.|","",-1,1,"");
	parser->addTest(6,"Name","condishun",1,999999,"defaultSKIP");
	parser->addTest(1,";","",1,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentAlt = new Alternative();
	parser->addTest(1,"Debug","",1,1,"defaultSKIP");
	parser->addTest(3,"A-Za-z0-9_","",-1,1,"");
	parser->addTest(6,"DebugRule","rulename",0,999999,"defaultSKIP");
	parser->addTest(1,";","",1,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentRule = parser->getRule("Start");
	//currentRule.immediate = StartplgNow;
	//currentRule.next = getRule("Start2");
	parser->currentAlt = new Alternative();
	parser->addTest(6,"Header","",0,1,"defaultSKIP");
	parser->addTest(6,"Body","",1,999999,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentAlt = new Alternative();
	parser->addTest(6,"Start2Block6","error",1,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentRule = parser->getRule("StringSet");
	// Source: defer = StringSetplgAct; original grammar had '[' then ']'
	// with the action callback doing content matching imperatively. In
	// grammar terms this is `[ (^])*  ]` — opener, any chars except
	// closer, closer.
	parser->currentAlt = new Alternative();
	parser->addTest(1,"[","",1,1,"defaultSKIP");
	parser->addTest(3,"^]","text",0,999999,"");
	parser->addTest(1,"]","",1,1,"");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentAlt = new Alternative();
	parser->addTest(6,"Name","name",1,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	// AlternativeBlock4 and Alternative2Block5 deleted — inlined into
	// Alternative's alternative list above.
	parser->currentRule = parser->getRule("Body");
	parser->currentSet = commentSet;
	parser->currentRule->guardSet = parser->currentSet;
	//currentRule.next = getRule("Body2");
	parser->currentAlt = new Alternative();
	parser->addTest(6,"Comment","comment",1,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentAlt = new Alternative();
	parser->addTest(6,"SetVariable","",1,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentAlt = new Alternative();
	parser->addTest(6,"Include","",1,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentAlt = new Alternative();
	parser->addTest(6,"Tail","",1,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentAlt = new Alternative();
	parser->addTest(6,"Rule","atRule",1,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentAlt = new Alternative();
	parser->addTest(6,"ActionRule","",1,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentAlt = new Alternative();
	parser->addTest(1,"\n","error",0,1,"");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentRule = parser->getRule("DebugText");
	//currentRule.next = getRule("DebugText2");
	parser->currentAlt = new Alternative();
	parser->addTest(6,"DebugTextBalancE","text",1,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentAlt = new Alternative();
	parser->addTest(3,"a-zA-Z0-9","text",1,999999,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentRule = parser->getRule("DebugOption");
	parser->currentAlt = new Alternative();
	parser->addTest(1,"=","",1,1,"defaultSKIP");
	parser->addTest(6,"DebugText","option",1,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentRule = parser->getRule("Start2Block6");
	parser->currentSet = parser->getSet("\n");
	parser->currentAlt = new Alternative();
	parser->addTest(3,"\n","",1,999999,"defaultSKIP");
	parser->addTest(1,"\n","",1,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
}
// Ignoring declaration of unused variable alt in method: setRules()
// Ignoring declaration of unused variable elem in method: setRules()
/*	Warning: the following methods were referenced but not declared
	strrchr(char*,char)
*/
