#include <string.h>
#include <stdio.h>
#include "DoubleLinkList.h"
#include "PLGrule.h"
#include "Alternative.h"
#include "DoubleLink.h"
#include "PLGitem.h"
#include "PLGset.h"
#include "Stak.h"
#include "BaseHash.h"
#include "Element.h"
#include "Buffer.h"
#include "PLGparse.h"
#include "PLG.h"

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
Alternative 	*alt = 0;
Element 		*elem = 0;
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
	alt = new Alternative();
	elem = new Element();
	elem->kind = 3;
	// kSet
	elem->minimum = 0;
	elem->maximum = 999999;
	parser->currentSet = parser->getSet(" \f\r\t\n");
	elem->setRef = parser->currentSet;
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 3;
	// kSet
	elem->minimum = 1;
	elem->maximum = 999999;
	parser->currentSet = parser->getSet("0-9");
	elem->setRef = parser->currentSet;
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("Max");
	alt = new Alternative();
	elem = new Element();
	elem->kind = 1;
	// kLit
	elem->minimum = 1;
	elem->maximum = 1;
	elem->litText = ",";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 1;
	elem->maximum = 1;
	elem->ruleRef = parser->getRule("Integer");
	elem->label = "maximum";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("Name");
	parser->currentSet = parser->getSet("._a-zA-Z0-9");
	alt = new Alternative();
	elem = new Element();
	elem->kind = 3;
	// kSet
	elem->minimum = 1;
	elem->maximum = 999999;
	elem->setRef = parser->currentSet;
	elem->label = "name";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("SetAssignment");
	alt = new Alternative();
	elem = new Element();
	elem->kind = 1;
	// kLit
	elem->minimum = 1;
	elem->maximum = 1;
	elem->litText = "=";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 1;
	elem->maximum = 1;
	elem->ruleRef = parser->getRule("SetName");
	elem->label = "set";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("BalanceRight");
	//currentRule.defer = BalanceRightplgAct;
	alt = new Alternative();
	elem = new Element();
	elem->kind = 1;
	// kLit
	elem->minimum = 1;
	elem->maximum = 1;
	elem->litText = "<>";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 1;
	elem->maximum = 1;
	elem->ruleRef = parser->getRule("Element");
	elem->label = "element";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 0;
	elem->maximum = 1;
	elem->ruleRef = parser->getRule("ElementType");
	elem->label = "type";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 1;
	// kLit
	elem->minimum = 0;
	elem->maximum = 1;
	elem->litText = "&";
	elem->label = "noSkip";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("Command");
	//currentRule.immediate = CommandplgNow;
	//currentRule.next = getRule("Command2");
	alt = new Alternative();
	elem = new Element();
	elem->kind = 1;
	// kLit
	elem->minimum = 0;
	elem->maximum = 1;
	elem->litText = "re";
	elem->label = "reset";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 1;
	// kLit
	elem->minimum = 1;
	elem->maximum = 1;
	elem->litText = "setSKIP";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 0;
	elem->maximum = 1;
	elem->ruleRef = parser->getRule("SetAssignment");
	elem->label = "set";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("Command2");
	//currentRule.immediate = Command2plgNow;
	alt = new Alternative();
	elem = new Element();
	elem->kind = 1;
	// kLit
	elem->minimum = 1;
	elem->maximum = 1;
	elem->litText = "dEBUG";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("FileName");
	//currentRule.immediate = FileNameplgNow;
	parser->currentSet = parser->getSet("A-Za-z0-9_");
	alt = new Alternative();
	elem = new Element();
	elem->kind = 1;
	// kLit
	elem->minimum = 0;
	elem->maximum = 1;
	elem->litText = "/";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 0;
	elem->maximum = 999999;
	elem->ruleRef = parser->getRule("FileNameBlock0");
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 3;
	// kSet
	elem->minimum = 1;
	elem->maximum = 999999;
	elem->setRef = parser->currentSet;
	elem->label = "name";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 1;
	// kLit
	elem->minimum = 1;
	elem->maximum = 1;
	elem->litText = ".g";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("SetVariable6");
	//currentRule.immediate = SetVariable6plgNow;
	parser->currentSet = excludeSet;
	alt = new Alternative();
	elem = new Element();
	elem->kind = 1;
	// kLit
	elem->minimum = 1;
	elem->maximum = 1;
	elem->litText = "Debug";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 3;
	// kSet
	elem->minimum = -1;
	elem->maximum = 1;
	elem->setRef = parser->currentSet;
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 0;
	elem->maximum = 999999;
	elem->ruleRef = parser->getRule("DebugRule");
	elem->label = "rulename";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 1;
	// kLit
	elem->minimum = 1;
	elem->maximum = 1;
	elem->litText = ";";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("Header");
	//currentRule.immediate = HeaderplgNow;
	parser->currentRule->doNotGuard = 1;
	alt = new Alternative();
	elem = new Element();
	elem->kind = 1;
	// kLit
	elem->minimum = 0;
	elem->maximum = 1;
	elem->litText = "%%";
	elem->label = "head";
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 0;
	elem->maximum = 1;
	elem->ruleRef = parser->getRule("Extender");
	elem->label = "name";
	elem->skipSet = parser->getSet("defaultSKIP");
	// currentTest.skipOverMatch = true;
	// currentTest.processUpTo = true;
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("ActionEnd");
	parser->currentSet = parser->getSet(".|;");
	alt = new Alternative();
	elem = new Element();
	elem->kind = 1;
	// kLit
	elem->minimum = 1;
	elem->maximum = 1;
	elem->litText = "\n";
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 3;
	// kSet
	elem->minimum = 1;
	elem->maximum = 1;
	elem->setRef = parser->currentSet;
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("SetName");
	//currentRule.immediate = SetNameplgNow;
	alt = new Alternative();
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 1;
	elem->maximum = 1;
	elem->ruleRef = parser->getRule("Name");
	elem->label = "set";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("Tail");
	//currentRule.immediate = TailplgNow;
	alt = new Alternative();
	elem = new Element();
	elem->kind = 1;
	// kLit
	elem->minimum = 1;
	elem->maximum = 1;
	elem->litText = "%%";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 4;
	// kAny
	elem->minimum = 1;
	elem->maximum = 999999;
	elem->label = "code";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("Alpha");
	parser->currentSet = parser->getSet("a-zA-Z0-9_.");
	alt = new Alternative();
	elem = new Element();
	elem->kind = 3;
	// kSet
	elem->minimum = 1;
	elem->maximum = 999999;
	elem->setRef = parser->currentSet;
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("TrailingSpace");
	parser->currentSet = parser->getSet("t");
	alt = new Alternative();
	elem = new Element();
	elem->kind = 3;
	// kSet
	elem->minimum = 0;
	elem->maximum = 999999;
	elem->setRef = parser->currentSet;
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 1;
	// kLit
	elem->minimum = 1;
	elem->maximum = 1;
	elem->litText = "\n";
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("CommentPart");
	//currentRule.next = getRule("CommentPart2");
	alt = new Alternative();
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 1;
	elem->maximum = 1;
	elem->ruleRef = parser->getRule("CommentPartBalancE");
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("Body7");
	//currentRule.immediate = Body7plgNow;
	parser->currentRule->doNotGuard = 1;
	alt = new Alternative();
	elem = new Element();
	elem->kind = 1;
	// kLit
	elem->minimum = 0;
	elem->maximum = 1;
	elem->litText = "\n";
	elem->label = "error";
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("CommentPartBalancE");
	//currentRule.immediate = balancE;
	alt = new Alternative();
	elem = new Element();
	elem->kind = 1;
	// kLit
	elem->minimum = 1;
	elem->maximum = 1;
	elem->litText = "/*";
	elem->label = "start";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 0;
	elem->maximum = 999999;
	elem->ruleRef = parser->getRule("CommentPartBoDY");
	elem->label = "body";
	elem->skipSet = parser->getSet("defaultSKIP");
	// currentTest.leftBalance = true;
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 1;
	// kLit
	elem->minimum = 1;
	elem->maximum = 1;
	elem->litText = "*/";
	elem->label = "finish";
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("DebugTextBalancE");
	//currentRule.immediate = balancE;
	parser->currentSet = singleQuote;
	parser->currentSet = singleQuote;
	alt = new Alternative();
	elem = new Element();
	elem->kind = 3;
	// kSet
	elem->minimum = 1;
	elem->maximum = 1;
	elem->setRef = parser->currentSet;
	elem->label = "start";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 0;
	elem->maximum = 999999;
	elem->ruleRef = parser->getRule("DebugTextBoDY");
	elem->label = "body";
	elem->skipSet = parser->getSet("defaultSKIP");
	// currentTest.leftBalance = true;
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 3;
	// kSet
	elem->minimum = 1;
	elem->maximum = 1;
	elem->setRef = parser->currentSet;
	elem->label = "finish";
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("CommentPartBoDY");
	//currentRule.immediate = balancEbody;
	//currentRule.next = getRule("CommentPartAny");
	alt = new Alternative();
	elem = new Element();
	elem->kind = 1;
	// kLit
	elem->minimum = 0;
	elem->maximum = 1;
	elem->litText = "/*";
	elem->label = "begin";
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 1;
	// kLit
	elem->minimum = 0;
	elem->maximum = 1;
	elem->litText = "*/";
	elem->label = "end";
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("SetVariable4");
	//currentRule.immediate = SetVariable4plgNow;
	parser->currentSet = excludeSet;
	//currentRule.next = getRule("SetVariable5");
	alt = new Alternative();
	elem = new Element();
	elem->kind = 1;
	// kLit
	elem->minimum = 1;
	elem->maximum = 1;
	elem->litText = "Set";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 3;
	// kSet
	elem->minimum = -1;
	elem->maximum = 1;
	elem->setRef = parser->currentSet;
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 1;
	elem->maximum = 1;
	elem->ruleRef = parser->getRule("Name");
	elem->label = "name";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 0;
	elem->maximum = 1;
	elem->ruleRef = parser->getRule("StringSet");
	elem->label = "set";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 1;
	// kLit
	elem->minimum = 0;
	elem->maximum = 1;
	elem->litText = ";";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("CommentPartAny");
	//currentRule.immediate = balancEbail;
	alt = new Alternative();
	elem = new Element();
	elem->kind = 4;
	// kAny
	elem->minimum = 1;
	elem->maximum = 1;
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("QuotedStringBlock1Block2");
	parser->currentSet = parser->getSet("'");
	alt = new Alternative();
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 1;
	elem->maximum = 1;
	elem->ruleRef = parser->getRule("QuotedStringBlock1Block3");
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 3;
	// kSet
	elem->minimum = 1;
	elem->maximum = 1;
	elem->setRef = parser->currentSet;
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("CommentPart2");
	alt = new Alternative();
	elem = new Element();
	elem->kind = 1;
	// kLit
	elem->minimum = 1;
	elem->maximum = 1;
	elem->litText = "//";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 1;
	// kLit
	elem->minimum = 0;
	elem->maximum = 1;
	elem->litText = "\n";
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("DebugText2");
	parser->currentSet = parser->getSet("a-zA-Z0-9");
	alt = new Alternative();
	elem = new Element();
	elem->kind = 3;
	// kSet
	elem->minimum = 1;
	elem->maximum = 999999;
	elem->setRef = parser->currentSet;
	elem->label = "text";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("Comment");
	parser->currentSet = parser->getSet("/");
	parser->currentRule->guardSet = parser->currentSet;
	//currentRule.next = getRule("Comment2");
	alt = new Alternative();
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 1;
	elem->maximum = 1;
	elem->ruleRef = parser->getRule("CommentPart");
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("Balanced");
	//currentRule.defer = BalancedplgAct;
	alt = new Alternative();
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 1;
	elem->maximum = 1;
	elem->ruleRef = parser->getRule("Element");
	elem->label = "left";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 0;
	elem->maximum = 1;
	elem->ruleRef = parser->getRule("ElementType");
	elem->label = "type";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 1;
	// kLit
	elem->minimum = 0;
	elem->maximum = 1;
	elem->litText = "&";
	elem->label = "noSkip";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 0;
	elem->maximum = 1;
	elem->ruleRef = parser->getRule("BalanceRight");
	elem->label = "right";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("Comment2");
	alt = new Alternative();
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 1;
	elem->maximum = 1;
	elem->ruleRef = parser->getRule("Command");
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("Extender");
	alt = new Alternative();
	elem = new Element();
	elem->kind = 1;
	// kLit
	elem->minimum = 1;
	elem->maximum = 1;
	elem->litText = "eXTENDS";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 1;
	elem->maximum = 1;
	elem->ruleRef = parser->getRule("Name");
	elem->label = "name";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("OptionClause");
	alt = new Alternative();
	elem = new Element();
	elem->kind = 1;
	// kLit
	elem->minimum = 1;
	elem->maximum = 1;
	elem->litText = "|";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 1;
	elem->maximum = 1;
	elem->ruleRef = parser->getRule("RuleOption");
	elem->label = "dummy";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("FileNameBlock0");
	alt = new Alternative();
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 1;
	elem->maximum = 1;
	elem->ruleRef = parser->getRule("Alpha");
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 1;
	// kLit
	elem->minimum = 1;
	elem->maximum = 1;
	elem->litText = "/";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("Integer");
	parser->currentSet = parser->getSet("n");
	parser->currentSet = parser->getSet("0-9");
	alt = new Alternative();
	elem = new Element();
	elem->kind = 3;
	// kSet
	elem->minimum = 0;
	elem->maximum = 999999;
	elem->setRef = parser->currentSet;
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 3;
	// kSet
	elem->minimum = 1;
	elem->maximum = 999999;
	elem->setRef = parser->currentSet;
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("DebugRule");
	alt = new Alternative();
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 1;
	elem->maximum = 1;
	elem->ruleRef = parser->getRule("Name");
	elem->label = "name";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 0;
	elem->maximum = 1;
	elem->ruleRef = parser->getRule("DebugOption");
	elem->label = "option";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("Body6");
	//currentRule.next = getRule("Body7");
	alt = new Alternative();
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 1;
	elem->maximum = 1;
	elem->ruleRef = parser->getRule("ActionRule");
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("Label");
	alt = new Alternative();
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 1;
	elem->maximum = 1;
	elem->ruleRef = parser->getRule("Name");
	elem->label = "atLabel";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 1;
	// kLit
	elem->minimum = 1;
	elem->maximum = 1;
	elem->litText = "=";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("QuotedString");
	parser->currentSet = singleQuote;
	parser->currentSet = singleQuote;
	//currentRule.next = getRule("QuotedString2");
	alt = new Alternative();
	elem = new Element();
	elem->kind = 3;
	// kSet
	elem->minimum = 1;
	elem->maximum = 1;
	elem->setRef = parser->currentSet;
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 1;
	elem->maximum = 1;
	elem->ruleRef = parser->getRule("QuotedStringBlock1");
	elem->label = "text";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 3;
	// kSet
	elem->minimum = 1;
	elem->maximum = 1;
	elem->setRef = parser->currentSet;
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("QuotedStringBlock1");
	alt = new Alternative();
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 1;
	elem->maximum = 999999;
	elem->ruleRef = parser->getRule("QuotedStringBlock1Block2");
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("QuotedStringBlock1Block3");
	alt = new Alternative();
	elem = new Element();
	elem->kind = 1;
	// kLit
	elem->minimum = 1;
	elem->maximum = 1;
	elem->litText = "\\";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 4;
	// kAny
	elem->minimum = 1;
	elem->maximum = 1;
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("QuotedString2");
	parser->currentSet = parser->getSet(".$");
	alt = new Alternative();
	elem = new Element();
	elem->kind = 3;
	// kSet
	elem->minimum = 1;
	elem->maximum = 1;
	elem->setRef = parser->currentSet;
	elem->label = "text";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("Word");
	parser->currentSet = parser->getSet("n;");
	alt = new Alternative();
	elem = new Element();
	elem->kind = 3;
	// kSet
	elem->minimum = 1;
	elem->maximum = 999999;
	elem->setRef = parser->currentSet;
	elem->label = "what";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("ActionOption");
	//currentRule.defer = ActionOptionplgAct;
	//currentRule.next = getRule("ActionOption2");
	alt = new Alternative();
	elem = new Element();
	elem->kind = 1;
	// kLit
	elem->minimum = 1;
	elem->maximum = 999999;
	elem->litText = "|";
	elem->label = "option";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("ActionOption2");
	//currentRule.defer = ActionOption2plgAct;
	alt = new Alternative();
	elem = new Element();
	elem->kind = 1;
	// kLit
	elem->minimum = 0;
	elem->maximum = 1;
	elem->litText = ".";
	elem->label = "immediate";
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 1;
	// kLit
	elem->minimum = -1;
	elem->maximum = 1;
	elem->litText = ";";
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 1;
	elem->maximum = 1;
	elem->ruleRef = parser->getRule("ActionBody");
	elem->label = "action";
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("ActionRule");
	//currentRule.immediate = ActionRuleplgNow;
	alt = new Alternative();
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 1;
	elem->maximum = 1;
	elem->ruleRef = parser->getRule("Name");
	elem->label = "ruleName";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 1;
	// kLit
	elem->minimum = 1;
	elem->maximum = 1;
	elem->litText = "!";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 1;
	elem->maximum = 999999;
	elem->ruleRef = parser->getRule("ActionOption");
	elem->label = "list";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 1;
	// kLit
	elem->minimum = 1;
	elem->maximum = 1;
	elem->litText = ";";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("Alternative2");
	//currentRule.defer = Alternative2plgAct;
	parser->currentSet = parser->getSet(")|");
	parser->currentRule->guardSet = parser->currentSet;
	//currentRule.next = getRule("Alternative3");
	alt = new Alternative();
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 0;
	elem->maximum = 1;
	elem->ruleRef = parser->getRule("Guard");
	elem->label = "guard";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 0;
	elem->maximum = 1;
	elem->ruleRef = parser->getRule("Label");
	elem->label = "atLabel";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 1;
	elem->maximum = 1;
	elem->ruleRef = parser->getRule("Alternative2Block5");
	elem->label = "atElement";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("ActionBody");
	alt = new Alternative();
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = -1;
	elem->maximum = 1;
	elem->ruleRef = parser->getRule("ActionEnd");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 0;
	elem->maximum = 1;
	elem->ruleRef = parser->getRule("ActionEnd");
	elem->label = "body";
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("Alternative");
	//currentRule.defer = AlternativeplgAct;
	//currentRule.next = getRule("Alternative2");
	alt = new Alternative();
	elem = new Element();
	elem->kind = 1;
	// kLit
	elem->minimum = 1;
	elem->maximum = 1;
	elem->litText = "FAIL";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 1;
	elem->maximum = 1;
	elem->ruleRef = parser->getRule("AlternativeBlock4");
	elem->label = "exception";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("Block");
	//currentRule.defer = BlockplgAct;
	alt = new Alternative();
	elem = new Element();
	elem->kind = 1;
	// kLit
	elem->minimum = 1;
	elem->maximum = 1;
	elem->litText = "(";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 1;
	elem->maximum = 999999;
	elem->ruleRef = parser->getRule("Alternative");
	elem->label = "atAlternative";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 0;
	elem->maximum = 999999;
	elem->ruleRef = parser->getRule("Clause");
	elem->label = "clause";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 1;
	// kLit
	elem->minimum = 1;
	elem->maximum = 1;
	elem->litText = ")";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 0;
	elem->maximum = 1;
	elem->ruleRef = parser->getRule("ElementType");
	elem->label = "type";
	elem->skipSet = parser->getSet("defaultSKIP");
	// currentTest.errorMessage = "Missing right parenthesis";
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 1;
	// kLit
	elem->minimum = 0;
	elem->maximum = 1;
	elem->litText = "&";
	elem->label = "noSkip";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("Clause");
	//currentRule.defer = ClauseplgAct;
	alt = new Alternative();
	elem = new Element();
	elem->kind = 1;
	// kLit
	elem->minimum = 1;
	elem->maximum = 1;
	elem->litText = "|";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 1;
	elem->maximum = 999999;
	elem->ruleRef = parser->getRule("Alternative");
	elem->label = "atAlternative";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("Element");
	//currentRule.immediate = ElementplgNow;
	//currentRule.defer = ElementplgAct;
	//currentRule.next = getRule("Element2");
	alt = new Alternative();
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = -1;
	elem->maximum = 1;
	elem->ruleRef = parser->getRule("Command");
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 1;
	elem->maximum = 1;
	elem->ruleRef = parser->getRule("Name");
	elem->label = "atElement";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("Element2");
	//currentRule.defer = Element2plgAct;
	//currentRule.next = getRule("Element3");
	alt = new Alternative();
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 1;
	elem->maximum = 1;
	elem->ruleRef = parser->getRule("QuotedString");
	elem->label = "atElement";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("Body2");
	//currentRule.next = getRule("Body3");
	alt = new Alternative();
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 1;
	elem->maximum = 1;
	elem->ruleRef = parser->getRule("SetVariable");
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("Element3");
	//currentRule.defer = Element3plgAct;
	//currentRule.next = getRule("Element4");
	alt = new Alternative();
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 1;
	elem->maximum = 1;
	elem->ruleRef = parser->getRule("StringSet");
	elem->label = "atElement";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("Body3");
	//currentRule.next = getRule("Body4");
	alt = new Alternative();
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 1;
	elem->maximum = 1;
	elem->ruleRef = parser->getRule("Include");
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("Element4");
	//currentRule.defer = Element4plgAct;
	parser->currentSet = parser->getSet(".$");
	alt = new Alternative();
	elem = new Element();
	elem->kind = 1;
	// kLit
	elem->minimum = 1;
	elem->maximum = 1;
	elem->litText = "~";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 3;
	// kSet
	elem->minimum = 1;
	elem->maximum = 1;
	elem->setRef = parser->currentSet;
	elem->label = "atElement";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("Body4");
	//currentRule.next = getRule("Body5");
	alt = new Alternative();
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 1;
	elem->maximum = 1;
	elem->ruleRef = parser->getRule("Tail");
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("ElementType");
	//currentRule.defer = ElementTypeplgAct;
	//currentRule.next = getRule("ElementType2");
	alt = new Alternative();
	elem = new Element();
	elem->kind = 1;
	// kLit
	elem->minimum = 1;
	elem->maximum = 1;
	elem->litText = "";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 1;
	elem->maximum = 1;
	elem->ruleRef = parser->getRule("Integer");
	elem->label = "minimum";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 0;
	elem->maximum = 1;
	elem->ruleRef = parser->getRule("Max");
	elem->label = "maximum";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 1;
	// kLit
	elem->minimum = 1;
	elem->maximum = 1;
	elem->litText = "";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("ElementType2");
	//currentRule.defer = ElementType2plgAct;
	parser->currentSet = parser->getSet("*+?!%");
	alt = new Alternative();
	elem = new Element();
	elem->kind = 3;
	// kSet
	elem->minimum = 1;
	elem->maximum = 1;
	elem->setRef = parser->currentSet;
	elem->label = "option";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("ForwardReference");
	//currentRule.immediate = ForwardReferenceplgNow;
	alt = new Alternative();
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 1;
	elem->maximum = 1;
	elem->ruleRef = parser->getRule("Name");
	elem->label = "name";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("Guard");
	//currentRule.defer = GuardplgAct;
	//currentRule.next = getRule("Guard2");
	alt = new Alternative();
	elem = new Element();
	elem->kind = 1;
	// kLit
	elem->minimum = 1;
	elem->maximum = 1;
	elem->litText = "#";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 1;
	elem->maximum = 1;
	elem->ruleRef = parser->getRule("StringSet");
	elem->label = "set";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("Guard2");
	//currentRule.defer = Guard2plgAct;
	alt = new Alternative();
	elem = new Element();
	elem->kind = 1;
	// kLit
	elem->minimum = 1;
	elem->maximum = 1;
	elem->litText = "#doNotGuard";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("Include");
	//currentRule.immediate = IncludeplgNow;
	//currentRule.next = getRule("Include2");
	alt = new Alternative();
	elem = new Element();
	elem->kind = 1;
	// kLit
	elem->minimum = 1;
	elem->maximum = 1;
	elem->litText = "include(";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 1;
	// kLit
	elem->minimum = 0;
	elem->maximum = 1;
	elem->litText = ")";
	elem->label = "file";
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("ParseInclude");
	alt = new Alternative();
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 1;
	elem->maximum = 999999;
	elem->ruleRef = parser->getRule("Body");
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("Include2");
	//currentRule.immediate = Include2plgNow;
	//currentRule.next = getRule("Include3");
	alt = new Alternative();
	elem = new Element();
	elem->kind = 1;
	// kLit
	elem->minimum = 1;
	elem->maximum = 1;
	elem->litText = "insert(";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 1;
	// kLit
	elem->minimum = 0;
	elem->maximum = 1;
	elem->litText = ")";
	elem->label = "file";
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("DebugTextBoDY");
	//currentRule.immediate = balancEbody;
	parser->currentSet = singleQuote;
	parser->currentSet = singleQuote;
	//currentRule.next = getRule("DebugTextAny");
	alt = new Alternative();
	elem = new Element();
	elem->kind = 3;
	// kSet
	elem->minimum = 0;
	elem->maximum = 1;
	elem->setRef = parser->currentSet;
	elem->label = "begin";
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 3;
	// kSet
	elem->minimum = 0;
	elem->maximum = 1;
	elem->setRef = parser->currentSet;
	elem->label = "end";
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("Include3");
	//currentRule.immediate = Include3plgNow;
	alt = new Alternative();
	elem = new Element();
	elem->kind = 1;
	// kLit
	elem->minimum = 1;
	elem->maximum = 1;
	elem->litText = "-%";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 1;
	// kLit
	elem->minimum = 0;
	elem->maximum = 1;
	elem->litText = "%-";
	elem->label = "code";
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("Rule");
	//currentRule.immediate = RuleplgNow;
	alt = new Alternative();
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 1;
	elem->maximum = 1;
	elem->ruleRef = parser->getRule("Name");
	elem->label = "ruleName";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 1;
	// kLit
	elem->minimum = 1;
	elem->maximum = 1;
	elem->litText = ":";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 1;
	elem->maximum = 1;
	elem->ruleRef = parser->getRule("RuleOptions");
	elem->label = "options";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 1;
	// kLit
	elem->minimum = 1;
	elem->maximum = 1;
	elem->litText = ";";
	elem->skipSet = parser->getSet("defaultSKIP");
	// currentTest.errorMessage = "Rule specification failed";
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("RuleOption");
	//currentRule.defer = RuleOptionplgAct;
	parser->currentRule->doNotGuard = 1;
	alt = new Alternative();
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 1;
	elem->maximum = 999999;
	elem->ruleRef = parser->getRule("Alternative");
	elem->label = "atAlternative";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("RuleOptions");
	//currentRule.defer = RuleOptionsplgAct;
	alt = new Alternative();
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 1;
	elem->maximum = 1;
	elem->ruleRef = parser->getRule("RuleOption");
	elem->label = "first";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 0;
	elem->maximum = 999999;
	elem->ruleRef = parser->getRule("OptionClause");
	elem->label = "others";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("SetVariable");
	//currentRule.immediate = SetVariableplgNow;
	parser->currentSet = excludeSet;
	//currentRule.next = getRule("SetVariable2");
	alt = new Alternative();
	elem = new Element();
	elem->kind = 1;
	// kLit
	elem->minimum = 1;
	elem->maximum = 1;
	elem->litText = "Variable";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 3;
	// kSet
	elem->minimum = -1;
	elem->maximum = 1;
	elem->setRef = parser->currentSet;
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 1;
	elem->maximum = 999999;
	elem->ruleRef = parser->getRule("Name");
	elem->label = "variable";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 1;
	// kLit
	elem->minimum = 1;
	elem->maximum = 1;
	elem->litText = ";";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("SetVariable3");
	//currentRule.immediate = SetVariable3plgNow;
	parser->currentSet = excludeSet;
	//currentRule.next = getRule("SetVariable4");
	alt = new Alternative();
	elem = new Element();
	elem->kind = 1;
	// kLit
	elem->minimum = 1;
	elem->maximum = 1;
	elem->litText = "KeyWord";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 3;
	// kSet
	elem->minimum = -1;
	elem->maximum = 1;
	elem->setRef = parser->currentSet;
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 1;
	elem->maximum = 1;
	elem->ruleRef = parser->getRule("Name");
	elem->label = "name";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 0;
	elem->maximum = 999999;
	elem->ruleRef = parser->getRule("Word");
	elem->label = "list";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 1;
	// kLit
	elem->minimum = 1;
	elem->maximum = 1;
	elem->litText = ";";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("SetVariable5");
	//currentRule.immediate = SetVariable5plgNow;
	parser->currentSet = excludeSet;
	//currentRule.next = getRule("SetVariable6");
	alt = new Alternative();
	elem = new Element();
	elem->kind = 1;
	// kLit
	elem->minimum = 1;
	elem->maximum = 1;
	elem->litText = "Condition";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 3;
	// kSet
	elem->minimum = -1;
	elem->maximum = 1;
	elem->setRef = parser->currentSet;
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 1;
	elem->maximum = 999999;
	elem->ruleRef = parser->getRule("Name");
	elem->label = "condishun";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 1;
	// kLit
	elem->minimum = 1;
	elem->maximum = 1;
	elem->litText = ";";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("Start");
	//currentRule.immediate = StartplgNow;
	//currentRule.next = getRule("Start2");
	alt = new Alternative();
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 0;
	elem->maximum = 1;
	elem->ruleRef = parser->getRule("Header");
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 1;
	elem->maximum = 999999;
	elem->ruleRef = parser->getRule("Body");
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("StringSet");
	//currentRule.defer = StringSetplgAct;
	//currentRule.next = getRule("StringSet2");
	alt = new Alternative();
	elem = new Element();
	elem->kind = 1;
	// kLit
	elem->minimum = 1;
	elem->maximum = 1;
	elem->litText = "[";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 1;
	// kLit
	elem->minimum = 0;
	elem->maximum = 1;
	elem->litText = "]";
	elem->label = "text";
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("StringSet2");
	//currentRule.immediate = StringSet2plgNow;
	//currentRule.defer = StringSet2plgAct;
	alt = new Alternative();
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 1;
	elem->maximum = 1;
	elem->ruleRef = parser->getRule("Name");
	elem->label = "name";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("AlternativeBlock4");
	alt = new Alternative();
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 1;
	elem->maximum = 1;
	elem->ruleRef = parser->getRule("Name");
	elem->label = "exception";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 1;
	elem->maximum = 1;
	elem->ruleRef = parser->getRule("QuotedString");
	elem->label = "exception";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("Alternative2Block5");
	alt = new Alternative();
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 1;
	elem->maximum = 1;
	elem->ruleRef = parser->getRule("Balanced");
	elem->label = "atElement";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 1;
	elem->maximum = 1;
	elem->ruleRef = parser->getRule("Block");
	elem->label = "atElement";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("Alternative3");
	parser->currentSet = commentSet;
	parser->currentRule->guardSet = parser->currentSet;
	alt = new Alternative();
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 1;
	elem->maximum = 1;
	elem->ruleRef = parser->getRule("Comment");
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("Body");
	parser->currentSet = commentSet;
	parser->currentRule->guardSet = parser->currentSet;
	//currentRule.next = getRule("Body2");
	alt = new Alternative();
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 1;
	elem->maximum = 1;
	elem->ruleRef = parser->getRule("Comment");
	elem->label = "comment";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("Body5");
	//currentRule.next = getRule("Body6");
	alt = new Alternative();
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 1;
	elem->maximum = 1;
	elem->ruleRef = parser->getRule("Rule");
	elem->label = "atRule";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("DebugText");
	//currentRule.next = getRule("DebugText2");
	alt = new Alternative();
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 1;
	elem->maximum = 1;
	elem->ruleRef = parser->getRule("DebugTextBalancE");
	elem->label = "text";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("DebugTextAny");
	//currentRule.immediate = balancEbail;
	alt = new Alternative();
	elem = new Element();
	elem->kind = 4;
	// kAny
	elem->minimum = 1;
	elem->maximum = 1;
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("DebugOption");
	alt = new Alternative();
	elem = new Element();
	elem->kind = 1;
	// kLit
	elem->minimum = 1;
	elem->maximum = 1;
	elem->litText = "=";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 1;
	elem->maximum = 1;
	elem->ruleRef = parser->getRule("DebugText");
	elem->label = "option";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("SetVariable2");
	parser->currentSet = excludeSet;
	//currentRule.next = getRule("SetVariable3");
	alt = new Alternative();
	elem = new Element();
	elem->kind = 1;
	// kLit
	elem->minimum = 1;
	elem->maximum = 1;
	elem->litText = "Rules";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 3;
	// kSet
	elem->minimum = -1;
	elem->maximum = 1;
	elem->setRef = parser->currentSet;
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 1;
	elem->maximum = 999999;
	elem->ruleRef = parser->getRule("ForwardReference");
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 1;
	// kLit
	elem->minimum = 1;
	elem->maximum = 1;
	elem->litText = ";";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("Start2");
	alt = new Alternative();
	elem = new Element();
	elem->kind = 6;
	// kRuleRef
	elem->minimum = 1;
	elem->maximum = 1;
	elem->ruleRef = parser->getRule("Start2Block6");
	elem->label = "error";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
	parser->currentRule = parser->getRule("Start2Block6");
	parser->currentSet = parser->getSet("\n");
	alt = new Alternative();
	elem = new Element();
	elem->kind = 3;
	// kSet
	elem->minimum = 1;
	elem->maximum = 999999;
	elem->setRef = parser->currentSet;
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	elem = new Element();
	elem->kind = 1;
	// kLit
	elem->minimum = 1;
	elem->maximum = 1;
	elem->litText = "\n";
	elem->skipSet = parser->getSet("defaultSKIP");
	alt->elements->add(elem);
	parser->currentRule->alternatives->add(alt);
}
