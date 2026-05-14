#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "StringRoutines.h"
#include "DoubleLinkList.h"
#include "PLGrule.h"
#include "Alternative.h"
#include "DoubleLink.h"
#include "Stak.h"
#include "BaseHash.h"
#include "Element.h"
#include "Buffer.h"
#include "KeyTable.h"
#include "PLGparse.h"
#include "PLGitem.h"
#include "PLGset.h"
#include "PLG.h"

/*******************************************************************************
	AlternativeplgAct — fires after a plg-Alternative meta-rule match
	(deferred). A plg-Alternative is one element-with-modifiers; its only
	job at this layer is to surface any labeled-capture name (`name=`) so
	the immediately-following ElementplgAct can stamp it onto the Element
	it creates. Does NOT create a C++ Alternative — that's RuleOptionplgAct.
*******************************************************************************/
void AlternativeplgAct(PLGparse *state, PLGitem *iTEM)
{
PLGitem 	*atLabel = 0;
PLGitem 	*atElement = 0;
	state->pendingLabel = 0;
	state->pendingNoSkip = 0;
	if ( iTEM && iTEM->children )
		{
		// iTEM.children["atLabel"] is the Label meta-rule match (`Name '='`).
		// To get just the Name string, drill into Label's own children
		// where the Name is also captured under "atLabel".
		atLabel = (PLGitem*)iTEM->children->get("atLabel");
		if ( atLabel && atLabel->children )
			{
			PLGitem 	*nameItem = (PLGitem*)atLabel->children->get("atLabel");
			if ( nameItem )
				state->pendingLabel = nameItem->toString();
			}
		// The `&` (noSkip) modifier sits on the Balanced/Block wrapper that
		// is captured as `atElement`. Pull it out so the next ElementplgAct
		// can stamp elem.noSkip.
		atElement = (PLGitem*)iTEM->children->get("atElement");
		if ( atElement && atElement->children )
			{
			PLGitem 	*ns = (PLGitem*)atElement->children->get("noSkip");
			if ( ns )
				state->pendingNoSkip = 1;
			}
		}
	::printf("AlternativeplgAct fired: pendingLabel=%s pendingNoSkip=%u\n",state->pendingLabel,state->pendingNoSkip);
}

/*******************************************************************************
    BlockplgAct — fires (deferred) when a paren-block ( A | B | ... ) matches
    in the source grammar. Decomposes the block by:
      1. Creating a helper rule named <parentRuleName>Block<state.helperCount++>
      2. Swapping state.currentRule/currentAlt to helper, then running the
         captured atAlternative chain's deferred actions — each ElementplgAct
         fires against the helper rule, populating its first alt.
      3. Each Clause becomes another helper alt (same swap pattern).
      4. Restoring parent currentRule/currentAlt and adding ONE kRuleRef
         element pointing to helper. Any pendingLabel is stamped onto that
         kRuleRef so caller-side labels survive decomposition.
    Recursive nesting works naturally — nested () inside the alts triggers
    nested BlockplgAct, captures parent name = outer helper name, producing
    parent-prefixed names like ParentBlock0Block1.
    Counter is reset per user-rule in RuleplgNow.
*******************************************************************************/
void BlockplgAct(PLGparse *state, PLGitem *iTEM)
{
char 			*parentName = 0;
char 			*helperName = 0;
Buffer 			*buf = 0;
PLGrule 		*savedRule = 0;
Alternative 	*savedAlt = 0;
PLGrule 		*helper = 0;
Alternative 	*helperAlt = 0;
PLGitem 		*atAlt = 0;
PLGitem 		*clauseItem = 0;
DoubleLink 		*dlink = 0;
PLGitem 		*deferEntry = 0;
Element 		*elem = 0;
char 			*label = 0;
int 			noSkip = 0;
	if ( !iTEM )
		return;
	if ( !state->currentRule || !state->currentAlt )
		{
		::fprintf(stderr,"BlockplgAct: no currentRule/currentAlt\n");
		return;
		}
	label = state->pendingLabel;
	noSkip = state->pendingNoSkip;
	state->pendingLabel = 0;
	state->pendingNoSkip = 0;
	parentName = state->currentRule->name;
	buf = ::bufferFactory3("blockHelper",256);
	buf->appendString(parentName,0,0);
	buf->appendString("Block",0,0);
	buf->appendInt(state->helperCount++,0,0);
	helperName = buf->toString();
	::printf("BlockplgAct: creating helper '%s' under parent '%s'\n",helperName,parentName);
	savedRule = state->currentRule;
	savedAlt = state->currentAlt;
	helper = state->getRule(helperName);
	state->currentRule = helper;
	helperAlt = new Alternative();
	helper->alternatives->add(helperAlt);
	state->currentAlt = helperAlt;
	atAlt = (PLGitem*)iTEM->children->get("atAlternative");
	while ( atAlt )
		{
		atAlt->runDeferred(state);
		// Clear deferRules on every defer entry we just fired so the outer
		// runDeferred (which has these same entries bubbled up) skips them
		// instead of re-firing them against the parent rule.
		if ( atAlt->deferred )
			for ( dlink = atAlt->deferred->first; dlink; dlink = dlink->next )
				{
				deferEntry = (PLGitem*)dlink->value;
				if ( deferEntry )
					deferEntry->deferRule = 0;
				}
		atAlt = atAlt->itemNext;
		}
	clauseItem = (PLGitem*)iTEM->children->get("clause");
	while ( clauseItem && clauseItem->itemLength > 0 )
		{
		helperAlt = new Alternative();
		helper->alternatives->add(helperAlt);
		state->currentAlt = helperAlt;
		clauseItem->runDeferred(state);
		if ( clauseItem->deferred )
			for ( dlink = clauseItem->deferred->first; dlink; dlink = dlink->next )
				{
				deferEntry = (PLGitem*)dlink->value;
				if ( deferEntry )
					deferEntry->deferRule = 0;
				}
		clauseItem = clauseItem->itemNext;
		}
	state->currentRule = savedRule;
	state->currentAlt = savedAlt;
	elem = new Element();
	elem->kind = 6;
	elem->ruleRef = helper;
	elem->minimum = 1;
	elem->maximum = 1;
	if ( label )
		elem->label = label;
	if ( noSkip )
		elem->noSkip = 1;
	state->currentAlt->elements->add(elem);
	::printf("BlockplgAct: added kRuleRef -> '%s' on parent '%s' label='%s' noSkip=%u\n",helperName,parentName,elem->label,elem->noSkip);
}

/*******************************************************************************
	ElementTypeplgAct — fires after an ElementType meta-rule match
	(deferred). Modifies the most-recently-added element on
	state.currentAlt — sets min/max from explicit `{N M}` form, or maps
	the option char to min/max + flags:
	  *  -> min=0,  max=999999
	  +  -> min=1,  max=999999
	  ?  -> min=0,  max=1
	  !  -> min=-1, banged=true, label cleared
	  %  -> isIgnored=true
	(`{`/`}` from the original spec aren't in the current option charset
	`*+?!%`, so omitted; the corresponding processUpTo/skipOverMatch
	fields don't exist on the new Element class either.)
*******************************************************************************/
void ElementTypeplgAct(PLGparse *state, PLGitem *iTEM)
{
Element 	*elem = 0;
DoubleLink 	*lastLink = 0;
PLGitem 	*minimum = 0;
PLGitem 	*maximum = 0;
PLGitem 	*option = 0;
char 		opt = 0;
	if ( !state->currentAlt )
		{
		::fprintf(stderr,"ElementTypeplgAct: no currentAlt\n");
		return;
		}
	if ( !iTEM || !iTEM->children )
		return;
	lastLink = state->currentAlt->elements->last;
	if ( !lastLink )
		{
		::fprintf(stderr,"ElementTypeplgAct: currentAlt has no elements to modify\n");
		return;
		}
	elem = (Element*)lastLink->value;
	// Alt 1: explicit `{Integer Max?}` form — minimum and (optional) maximum
	minimum = (PLGitem*)iTEM->children->get("minimum");
	if ( minimum )
		{
		elem->minimum = ::atoi(minimum->toString());
		elem->maximum = elem->minimum;
		maximum = (PLGitem*)iTEM->children->get("maximum");
		if ( maximum )
			elem->maximum = ::atoi(maximum->toString());
		::printf("ElementTypeplgAct: explicit min=%d max=%d\n",elem->minimum,elem->maximum);
		return;
		}
	// Alt 2: single option char from *+?!%
	option = (PLGitem*)iTEM->children->get("option");
	if ( !option )
		{
		::fprintf(stderr,"ElementTypeplgAct: no option child\n");
		return;
		}
	opt = *option->itemStart;
	if ( opt == '*' )
		{
		elem->minimum = 0;
		elem->maximum = 999999;
		}
	if ( opt == '+' )
		{
		elem->minimum = 1;
		elem->maximum = 999999;
		}
	if ( opt == '?' )
		{
		elem->minimum = 0;
		elem->maximum = 1;
		}
	if ( opt == '!' )
		{
		elem->minimum = -1;
		elem->banged = 1;
		elem->label = 0;
		}
	if ( opt == '%' )
		elem->isIgnored = 1;
	if ( opt == '{' )
		{
		elem->minimum = 0;
		elem->maximum = 1;
		elem->processUpTo = 1;
		elem->skipSet = 0;
		}
	if ( opt == '}' )
		{
		elem->minimum = 0;
		elem->maximum = 1;
		elem->processUpTo = 1;
		elem->skipOverMatch = 1;
		elem->skipSet = 0;
		}
	::printf("ElementTypeplgAct: opt='%c' min=%d max=%d\n",opt,elem->minimum,elem->maximum);
}

/*******************************************************************************
	ElementplgAct — fires after an Element meta-rule match completes
	(deferred). Discriminates on the captured atElement's first character
	to determine kind, fills kind-specific data, and appends the new
	Element to currentAlt.elements. min/max default to 1; ElementType's
	defer handles repetition (separate task). Label handling is also
	separate.

	Discrimination:
	  ' or "    QuotedString  -> kLit (1), litText = quoted content
	  [         StringSet     -> kSet (3), setRef = state.getSet(content)
	  .         (~ . form)    -> kAny (4)
	  $         (~ $ form)    -> kEof (5)
	  letter    Name          -> table lookup chain:
	                              keyWordTable    -> kKeyTable (7)
	                              conditionTable  -> kCondition (8)
	                              variableTable   -> kVariable (9)
	                              setTable        -> kSet (3)
	                              else            -> kRuleRef (6)
*******************************************************************************/
void ElementplgAct(PLGparse *state, PLGitem *iTEM)
{
Element 	*elem = 0;
PLGitem 	*atElement = 0;
char 		*text = 0;
char 		*spec = 0;
char 		*litText = 0;
char 		*name = 0;
char 		first = 0;
int 		len = 0;
void 		*ptr = 0;
PLGset 		*pset = 0;
PLGrule 	*prule = 0;
int 		handled = 0;
	if ( !state->currentAlt )
		{
		::fprintf(stderr,"ElementplgAct: no currentAlt — AlternativeplgAct must run first\n");
		return;
		}
	if ( !iTEM || !iTEM->children )
		return;
	atElement = (PLGitem*)iTEM->children->get("atElement");
	if ( !atElement )
		{
		::fprintf(stderr,"ElementplgAct: no atElement child\n");
		return;
		}
	text = atElement->toString();
	if ( !text || !*text )
		{
		::fprintf(stderr,"ElementplgAct: empty atElement text\n");
		return;
		}
	first = *text;
	elem = new Element();
	elem->minimum = 1;
	elem->maximum = 1;
	// QuotedString -> kLit; strip surrounding quotes
	if ( !handled && (first == '\'' || first == '"') )
		{
		elem->kind = 1;
		len = ::strlen(text);
		if ( len >= 2 )
			{
			litText = (char*)::malloc(len - 1);
			::strncpy(litText,text + 1,len - 2);
			*(litText + len - 2) = 0;
			elem->litText = litText;
			}
		handled = 1;
		}
	// Inline StringSet -> kSet; prefer captured "text" child, else strip brackets
	if ( !handled && first == '[' )
		{
		// Inline StringSet: bracket-strip atElement.toString() to get the
		// spec. (Don't use children["text"] — its first-char skip drops a
		// significant leading space, e.g. `[ \f\r\t\n]` becomes `\f\r\t\n`.)
		elem->kind = 3;
		len = ::strlen(text);
		if ( len >= 2 )
			{
			spec = (char*)::malloc(len - 1);
			::strncpy(spec,text + 1,len - 2);
			*(spec + len - 2) = 0;
			}
		else	spec = "";
		elem->setRef = state->getSet(spec);
		handled = 1;
		}
	if ( !handled && first == '.' )
		{
		elem->kind = 4;
		handled = 1;
		}
	if ( !handled && first == '$' )
		{
		elem->kind = 5;
		handled = 1;
		}
	// Name -> lookup chain (keyWord -> condition -> variable -> set -> rule)
	if ( !handled )
		{
		name = text;
		ptr = state->keyWordTable->get(name);
		if ( ptr )
			{
			elem->kind = 7;
			elem->tableRef = (KeyTable*)ptr;
			handled = 1;
			}
		}
	if ( !handled )
		{
		ptr = state->conditionTable->get(name);
		if ( ptr )
			{
			elem->kind = 8;
			elem->condFunc = ptr;
			handled = 1;
			}
		}
	if ( !handled )
		{
		ptr = state->variableTable->get(name);
		if ( ptr )
			{
			elem->kind = 9;
			elem->varPtr = (char**)ptr;
			handled = 1;
			}
		}
	if ( !handled )
		{
		pset = (PLGset*)state->setTable->get(name);
		if ( pset )
			{
			elem->kind = 3;
			elem->setRef = pset;
			handled = 1;
			}
		}
	if ( !handled )
		{
		elem->kind = 6;
		prule = state->getRule(name);
		elem->ruleRef = prule;
		}
	// Apply any pending label from AlternativeplgAct (the `name=` form).
	if ( state->pendingLabel )
		{
		elem->label = state->pendingLabel;
		state->pendingLabel = 0;
		}
	// Apply pending noSkip from Balanced/Block's `&` modifier.
	if ( state->pendingNoSkip )
		{
		elem->noSkip = 1;
		state->pendingNoSkip = 0;
		}
	state->currentAlt->elements->add(elem);
	::printf("ElementplgAct fired: kind=%u text='%s' label='%s' added to alt in rule '%s'\n",elem->kind,text,elem->label,state->currentRule->name);
}

int ForwardDeclplgNow(PLGparse *state, PLGitem *iTEM)
{
PLGitem 	*n = 0;
char 		*name = 0;
char 		*s = 0;
int 		i = 0;
char 		*first = 0;
	if ( !iTEM || !iTEM->children )
		return 0;
	n = (PLGitem*)iTEM->children->get("names");
	if ( !n )
		return 0;
	// Head's length was overwritten by Element.applyRepetition to span
	// the full repetition. Extract the head's NAME (alpha-prefix) by
	// scanning from its start until the first non-name char.
	s = n->itemStart;
	i = 0;
	while ( *(s + i) && (((*(s + i) >= 'A') && (*(s + i) <= 'Z')) || ((*(s + i) >= 'a') && (*(s + i) <= 'z')) || ((*(s + i) >= '0') && (*(s + i) <= '9')) || (*(s + i) == '_')) )
		i++;
	first = (char*)::malloc(i + 1);
	::strncpy(first,s,i);
	*(first + i) = 0;
	::printf("ForwardDeclplgNow: pre-registering '%s'\n",first);
	state->getRule(first);
	// Subsequent matches are linked via head.next — each has its own
	// proper start/length, so toString() works correctly.
	n = n->itemNext;
	while ( n )
		{
		name = n->toString();
		::printf("ForwardDeclplgNow: pre-registering '%s'\n",name);
		state->getRule(name);
		n = n->itemNext;
		}
	return 1;
}

/*******************************************************************************
	RuleplgNow — immediate action fired when a Rule alternative matches in
	Testing.g (or any .g file the parser is processing). Looks up the
	"ruleName" capture, calls state.getRule() so a fresh rule entry is
	created in parser.rules, then cascades the deferred action chain
	(RuleOptionsplgAct etc.) for the rule's body.
*******************************************************************************/
/*******************************************************************************
	IncludeplgNow — fires when an Include directive matches. Resolves the
	filename (sourceDir + name), loads the content, and runs Body.match
	in a loop on a diverted input until the include is exhausted. The
	outer parse resumes seamlessly via revertInput. PLGincludes-style
	non-grammar headers gracefully no-op when the diverted parse can't
	make progress.
*******************************************************************************/
int IncludeplgNow(PLGparse *state, PLGitem *iTEM)
{
PLGitem 	*fileItem = 0;
char 		*filename = 0;
char 		*content = 0;
char 		*fullPath = 0;
PLGitem 	*bodyResult = 0;
char 		*before = 0;
int 		dirLen = 0;
int 		fileLen = 0;
int 		iters = 0;
	if ( !iTEM || !iTEM->children )
		return 0;
	fileItem = (PLGitem*)iTEM->children->get("file");
	if ( !fileItem )
		{
		::fprintf(stderr,"IncludeplgNow: no file capture\n");
		return 0;
		}
	filename = fileItem->toString();
	if ( !filename || !*filename )
		{
		::fprintf(stderr,"IncludeplgNow: empty filename\n");
		return 0;
		}
	if ( *filename == '/' || !state->sourceDir )
		fullPath = filename;
	else {
		dirLen = ::strlen(state->sourceDir);
		fileLen = ::strlen(filename);
		fullPath = (char*)::malloc(dirLen + fileLen + 1);
		::strcpy(fullPath,state->sourceDir);
		::strcat(fullPath,filename);
		}
	::printf("IncludeplgNow: resolving '%s' -> '%s'\n",filename,fullPath);
	content = ::getStringFromFile(fullPath);
	if ( !content )
		{
		::fprintf(stderr,"IncludeplgNow: could not load '%s' — skipping\n",fullPath);
		return 0;
		}
	if ( !state->bodyRule )
		{
		::fprintf(stderr,"IncludeplgNow: bodyRule not set — process() must run first\n");
		return 0;
		}
	content = state->stripComments(content);
	state->divertInput(content);
	iters = 0;
	while ( state->cursor < state->eof )
		{
		before = state->cursor;
		bodyResult = state->bodyRule->match(state);
		if ( !bodyResult || state->cursor == before )
			break;
		iters++;
		}
	::printf("IncludeplgNow: '%s' parsed %d Body items\n",filename,iters);
	state->revertInput();
	return 1;
}

/*******************************************************************************
	RuleOptionplgAct — fires after a RuleOption match completes (deferred).
	A plg "RuleOption" is a SEQUENCE of plg-Alternatives separated by `|`
	at the RuleOptions level. Each RuleOption maps to ONE C++ Alternative
	whose elements are the SEQUENCE of items inside it. So this callback
	is what creates the C++ Alternative bucket — appends it to currentRule
	and parks it on state.currentAlt for incoming ElementplgAct calls.
*******************************************************************************/
void RuleOptionplgAct(PLGparse *state, PLGitem *iTEM)
{
Alternative 	*alt = 0;
	if ( !state->currentRule )
		{
		::fprintf(stderr,"RuleOptionplgAct: no currentRule\n");
		return;
		}
	alt = new Alternative();
	state->currentRule->addAlternative(alt);
	state->currentAlt = alt;
	::printf("RuleOptionplgAct fired: new alt in rule '%s'\n",state->currentRule->name);
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
	state->helperCount = 0;
	// per-rule reset for BlockplgAct helper naming
	iTEM->runDeferred(state);
	return 1;
}

/*******************************************************************************
    SetVariableplgNow — fires when SetVariable matches. Dispatches by first-
    keyword character to register captured names in the parser's tables.
    The Set branch is the round-trip fix: registering excludeSet/singleQuote
    etc. in setTable makes later element refs resolve via getSet instead of
    falling through to getRule as zombie empty rules.
*******************************************************************************/
int SetVariableplgNow(PLGparse *state, PLGitem *iTEM)
{
char 		*text = 0;
char 		keyword = 0;
PLGitem 	*nameItem = 0;
PLGitem 	*listItem = 0;
char 		*itemName = 0;
PLGset 		*newSet = 0;
KeyTable 	*keyTable = 0;
char 		*p = 0;
char 		*firstWord = 0;
int 		wordLen = 0;
char 		ch = 0;
	if ( !iTEM )
		return 0;
	text = iTEM->toString();
	if ( !text || !*text )
		return 0;
	keyword = *text;
	if ( keyword == 'S' )
		{
		nameItem = (PLGitem*)iTEM->children->get("name");
		if ( !nameItem )
			return 0;
		itemName = nameItem->toString();
		newSet = new PLGset(itemName);
		newSet->name = itemName;
		state->setTable->add(itemName,(void*)newSet);
		::printf("SetVariableplgNow: Set '%s' registered\n",itemName);
		return 1;
		}
	if ( keyword == 'K' )
		{
		// KeyWord <name> <words>* ;  — register a KeyTable group
		nameItem = (PLGitem*)iTEM->children->get("name");
		if ( !nameItem )
			return 0;
		itemName = nameItem->toString();
		keyTable = new KeyTable(itemName);
		listItem = (PLGitem*)iTEM->children->get("list");
		if ( listItem && listItem->itemLength > 0 )
			{
			// Head item's length spans the full Word* repetition; scan to
			// first word boundary (whitespace or ;) instead of toString().
			// Subsequent items via .next have correct individual lengths.
			p = listItem->itemStart;
			wordLen = 0;
			ch = *(p + wordLen);
			while ( ch && ch != ' ' && ch != '\t' && ch != '\n' && ch != '\r' && ch != '\f' && ch != ';' )
				{
				wordLen++;
				ch = *(p + wordLen);
				}
			firstWord = (char*)::malloc(wordLen + 1);
			::strncpy(firstWord,p,wordLen);
			*(firstWord + wordLen) = 0;
			keyTable->add(firstWord);
			listItem = listItem->itemNext;
			while ( listItem )
				{
				keyTable->add(listItem->toString());
				listItem = listItem->itemNext;
				}
			}
		state->keyWordTable->add(itemName,(void*)keyTable);
		::printf("SetVariableplgNow: KeyWord '%s' registered\n",itemName);
		return 1;
		}
	::printf("SetVariableplgNow: keyword '%c' (Variable/Rules/Condition/Debug — port from BeforeRefactor when needed)\n",keyword);
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

/*******************************************************************************
	dumpRules — walk a rules table and print each rule's alternatives and
	their elements. Used to verify callback-built rule structures look
	right (kinds, min/max, labels, set/litText/ruleRef contents).
*******************************************************************************/
void PLG::dumpRules(BaseHash *table)
{
PLGrule 		*rule = 0;
Alternative 	*alt = 0;
Element 		*elem = 0;
DoubleLink 		*altLink = 0;
DoubleLink 		*elemLink = 0;
int 			altNum = 0;
int 			elemNum = 0;
char 			*kindName = 0;
char 			*data = 0;
	if ( !table )
		return;
	::printf("=== rule structures ===\n");
	table->hashList->resetIterator();
	while ( rule = (PLGrule*)table->hashList->next() )
		{
		::printf("rule '%s' (%d alts):\n",rule->name,rule->alternatives->length);
		altNum = 0;
		for ( altLink = rule->alternatives->first; altLink; altLink = altLink->next )
			{
			alt = (Alternative*)altLink->value;
			altNum++;
			::printf("  alt %d:\n",altNum);
			elemNum = 0;
			for ( elemLink = alt->elements->first; elemLink; elemLink = elemLink->next )
				{
				elem = (Element*)elemLink->value;
				elemNum++;
				kindName = "?";
				data = "";
				if ( elem->kind == 1 )
					{
					kindName = "kLit";
					if ( elem->litText )
						data = elem->litText;
					}
				if ( elem->kind == 3 )
					{
					kindName = "kSet";
					if ( elem->setRef && elem->setRef->name )
						data = elem->setRef->name;
					}
				if ( elem->kind == 4 )
					kindName = "kAny";
				if ( elem->kind == 5 )
					kindName = "kEof";
				if ( elem->kind == 6 )
					{
					kindName = "kRuleRef";
					if ( elem->ruleRef )
						data = elem->ruleRef->name;
					}
				if ( elem->kind == 7 )
					kindName = "kKeyTable";
				if ( elem->kind == 8 )
					kindName = "kCondition";
				if ( elem->kind == 9 )
					kindName = "kVariable";
				::printf("    elem %d: %s(%s) min=%d max=%d label=%s\n",elemNum,kindName,data,elem->minimum,elem->maximum,elem->label);
				}
			}
		}
	::printf("=== end rule structures ===\n");
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
	// plg's path contract is CWD-relative: includes and regen output both
	// resolve through the current working directory, not the input file's
	// path. Caller is responsible for being in the right place.
	parser->sourceDir = "";
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
	// Stash the meta-grammar's Body rule so IncludeplgNow can sub-parse
	// included content via Body.match — its rule pointer survives the
	// table swap below even though name-lookup against parser.rules
	// would miss after the swap.
	parser->bodyRule = (PLGrule*)parser->rules->get("Body");
	metaRules = parser->rules;
	metaSetTable = parser->setTable;
	parser->rules = new BaseHash();
	parser->setTable = new BaseHash();
	content = parser->stripComments(content);
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
	dumpRules(parser->rules);
	// Demo: if the parsed table contains a "Test" rule, run it against
	// a tiny input ("42") and dump the result's children to prove that
	// labeled captures land in result.children. divertInput pushes a
	// fresh buffer; revertInput restores Testing.g's input afterwards.
PLGrule 	*testRule = (PLGrule*)parser->rules->get("Test");
	if ( testRule )
		{
		::printf("=== running parsed Test rule on '42' ===\n");
		parser->divertInput("42");
		PLGitem *testResult = testRule->match(parser);
		if ( testResult )
			{
			::printf("Test matched %ld chars: '%s'\n",testResult->itemLength,testResult->toString());
			if ( testResult->children )
				{
				PLGitem 	*v = (PLGitem*)testResult->children->get("value");
				if ( v )
					::printf("  testResult.children['value'] = '%s'\n",v->toString());
				else	::printf("  no 'value' in children\n");
				}
			else	::printf("  no children on result\n");
			}
		else	::printf("Test rule did not match '42'\n");
		parser->revertInput();
		::printf("=== end Test demo ===\n");
		}
	// Build output path: foo.g -> foo.regen.twk in CWD. Strip any leading
	// directory from filename so output lands where plg was fired, not
	// next to the source file. The .regen suffix avoids overwriting an
	// existing foo.twk source.
char 		*base = strrchr(filename,'/');
	if ( base )
		base++;
	else	base = filename;
	outFile = (char*)::malloc(::strlen(base) + 16);
	::strcpy(outFile,base);
	dot = strrchr(outFile,'.');
	if ( dot )
		{
		*dot = '\0';
		}
	::strcat(outFile,".regen.twk");
	// Generate from the FRESH parser.rules (the rules parsed out of
	// Testing.g) BEFORE restoring the meta-grammar. This is the real
	// bootstrap output: setRules code derived from the user's grammar
	// file, not a copy of the meta-grammar.
	twkOut = ::bufferFactory3("twk-output",50000);
	parser->generateRules(twkOut);
	twkOut->setFile(outFile);
	twkOut->flush();
	::printf("wrote %s\n",outFile);
	::free(outFile);
	// Restore meta-grammar tables (so future parses still work).
	parser->rules = metaRules;
	parser->setTable = metaSetTable;
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
	// Header items live before the first `%%`. Each Header is an Include
	// directive or a Comment. Header* in Start handles repetition.
	parser->currentAlt = new Alternative();
	parser->addTest(6,"Include","",1,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentAlt = new Alternative();
	parser->addTest(6,"Comment","",1,1,"defaultSKIP");
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
	// Trailer: everything after the second `%%`. Any chars to EOF.
	// Doesn't begin with `%%` — the second `%%` is consumed by Start.
	parser->currentRule = parser->getRule("Trailer");
	parser->currentRule->doNotGuard = 1;
	parser->currentAlt = new Alternative();
	parser->addTest(4,"","code",1,999999,"");
	parser->currentRule->alternatives->add(parser->currentAlt);
	// ForwardDecl: `Rules <names>+ ;` pre-registers rule names so
	// subsequent Rule definitions referencing them succeed even if
	// those rules haven't been seen yet. ForwardDeclplgNow fires
	// immediately and calls state.getRule(name) for each name.
	parser->currentRule = parser->getRule("ForwardDecl");
	parser->currentRule->immediate = ::ForwardDeclplgNow;
	parser->currentRule->doNotGuard = 1;
	parser->currentAlt = new Alternative();
	parser->addTest(1,"Rules","",1,1,"defaultSKIP");
	parser->addTest(6,"Name","names",1,999999,"defaultSKIP");
	parser->addTest(1,";","",1,1,"defaultSKIP");
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
	parser->currentRule->doNotGuard = 1;
	// Match any char that isn't the start of a closing `*/` marker:
	//   alt 1: any char except `*`
	//   alt 2: `*` followed by any char except `/`
	// This way Body+ stops cleanly when it reaches `*/`, leaving the
	// closer for CommentPartBalancE's mandatory `*/` element to consume.
	parser->currentAlt = new Alternative();
	parser->addTest(3,"^*","",1,1,"");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentAlt = new Alternative();
	parser->addTest(1,"*","",1,1,"");
	parser->addTest(3,"^/","",1,1,"");
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
	parser->currentSet = parser->getSet("^ \f\r\t\n;");
	parser->currentAlt = new Alternative();
	parser->addTest(3,"^ \f\r\t\n;","what",1,999999,"defaultSKIP");
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
	parser->currentRule->defer = ::BlockplgAct;
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
	parser->currentRule->defer = ::ElementplgAct;
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
	parser->currentRule->defer = ::ElementTypeplgAct;
	//currentRule.next = getRule("ElementType2");
	parser->currentAlt = new Alternative();
	parser->addTest(1,"{","",1,1,"defaultSKIP");
	parser->addTest(6,"Integer","minimum",1,1,"defaultSKIP");
	parser->addTest(6,"Max","maximum",0,1,"defaultSKIP");
	parser->addTest(1,"}","",1,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentAlt = new Alternative();
	parser->addTest(3,"*+?!%{}","option",1,1,"defaultSKIP");
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
	parser->currentRule->immediate = ::IncludeplgNow;
	//currentRule.next = getRule("Include2");
	// Bare form: `include Name` (no parens) — used at top of plg.g.
	parser->currentAlt = new Alternative();
	parser->addTest(1,"include","",1,1,"defaultSKIP");
	parser->addTest(6,"Name","file",1,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	// Paren forms: capture filename via ^) set between `(` and `)`.
	parser->currentAlt = new Alternative();
	parser->addTest(1,"include(","",1,1,"defaultSKIP");
	parser->addTest(3,"^)","file",1,999999,"");
	parser->addTest(1,")","",1,1,"");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentAlt = new Alternative();
	parser->addTest(1,"insert(","",1,1,"defaultSKIP");
	parser->addTest(3,"^)","file",1,999999,"");
	parser->addTest(1,")","",1,1,"");
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
	parser->currentRule->defer = ::RuleOptionplgAct;
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
	parser->currentRule->immediate = ::SetVariableplgNow;
	parser->currentSet = excludeSet;
	//currentRule.next = getRule("SetVariable2");
	parser->currentAlt = new Alternative();
	parser->addTest(1,"Variable","",1,1,"defaultSKIP");
	parser->addTest(3,"A-Za-z:.|","",-1,1,"");
	parser->setNoSkip();
	// mirror the `&` modifier in `excludeSet!&`
	parser->addTest(6,"Name","variable",1,999999,"defaultSKIP");
	parser->addTest(1,";","",1,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentAlt = new Alternative();
	parser->addTest(1,"Rules","",1,1,"defaultSKIP");
	parser->addTest(3,"A-Za-z:.|","",-1,1,"");
	parser->setNoSkip();
	// mirror the `&` modifier in `excludeSet!&`
	parser->addTest(6,"ForwardReference","",1,999999,"defaultSKIP");
	parser->addTest(1,";","",1,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentAlt = new Alternative();
	parser->addTest(1,"KeyWord","",1,1,"defaultSKIP");
	parser->addTest(3,"A-Za-z:.|","",-1,1,"");
	parser->setNoSkip();
	// mirror the `&` modifier in `excludeSet!&`
	parser->addTest(6,"Name","name",1,1,"defaultSKIP");
	parser->addTest(6,"Word","list",0,999999,"defaultSKIP");
	parser->addTest(1,";","",1,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentAlt = new Alternative();
	parser->addTest(1,"Set","",1,1,"defaultSKIP");
	parser->addTest(3,"A-Za-z:.|","",-1,1,"");
	parser->setNoSkip();
	// mirror the `&` modifier in `excludeSet!&`
	parser->addTest(6,"Name","name",1,1,"defaultSKIP");
	parser->addTest(6,"StringSet","set",0,1,"defaultSKIP");
	parser->addTest(1,";","",1,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentAlt = new Alternative();
	parser->addTest(1,"Condition","",1,1,"defaultSKIP");
	parser->addTest(3,"A-Za-z:.|","",-1,1,"");
	parser->setNoSkip();
	// mirror the `&` modifier in `excludeSet!&`
	parser->addTest(6,"Name","condishun",1,999999,"defaultSKIP");
	parser->addTest(1,";","",1,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentAlt = new Alternative();
	parser->addTest(1,"Debug","",1,1,"defaultSKIP");
	parser->addTest(3,"A-Za-z0-9_","",-1,1,"");
	parser->setNoSkip();
	// mirror the `&` modifier in `excludeSet!&`
	parser->addTest(6,"DebugRule","rulename",0,999999,"defaultSKIP");
	parser->addTest(1,";","",1,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentRule = parser->getRule("Start");
	//currentRule.immediate = StartplgNow;
	//currentRule.next = getRule("Start2");
	// Yacc-style three-section structure:
	//   Header*  '%%'  Body+  '%%'?  Trailer?
	// Header* matches pre-`%%` declarations (Include, Comment).
	// First `%%` opens the rules section.
	// Body+ matches rule definitions.
	// Optional second `%%` closes the rules section.
	// Trailer? consumes anything after the second `%%` (e.g. C++ main).
	parser->currentAlt = new Alternative();
	parser->addTest(6,"Header","",0,999999,"defaultSKIP");
	parser->addTest(1,"%%","",1,1,"defaultSKIP");
	parser->addTest(6,"Body","",1,999999,"defaultSKIP");
	parser->addTest(1,"%%","",0,1,"defaultSKIP");
	parser->addTest(6,"Trailer","",0,1,"defaultSKIP");
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
	// ForwardDecl ahead of SetVariable so the `Rules <names>;` form
	// fires ForwardDeclplgNow (pre-registers names) instead of being
	// silently swallowed by SetVariable's alt 2.
	parser->currentAlt = new Alternative();
	parser->addTest(6,"ForwardDecl","",1,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	// SetVariable's keyword alts (`'Set'`, `'Variable'`, `'Rules'`, etc.)
	// use `excludeSet!&` for word-boundary enforcement. With banged(!) and
	// noSkip(&) propagation in place via ElementTypeplgAct + pendingNoSkip,
	// those word boundaries hold and Rule no longer needs to be tried
	// first to avoid keyword-into-identifier greedy matching.
	parser->currentAlt = new Alternative();
	parser->addTest(6,"SetVariable","",1,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	parser->currentAlt = new Alternative();
	parser->addTest(6,"Include","",1,1,"defaultSKIP");
	parser->currentRule->alternatives->add(parser->currentAlt);
	// Tail removed from Body alts: `%%` now bounds the rules section
	// at Start level, not via a Body alternative that swallows the rest.
	// (ForwardDecl alt is registered ahead of SetVariable above.)
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
// Ignoring declaration of unused variable key in method: dumpRules(BaseHash*)
// Ignoring declaration of unused variable alt in method: setRules()
// Ignoring declaration of unused variable elem in method: setRules()
/*	Warning: the following methods were referenced but not declared
	strrchr(char*,char)
*/
// Ignoring declaration of unused variable textItem in method: ElementplgAct(PLGparse*,PLGitem*)
