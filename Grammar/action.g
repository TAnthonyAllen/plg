
ActionOption!
    for option
        optionCount += option.itemLength;
|
    buffer.reset();
    ruleActionName.toBuffer(buffer);
    if optionCount > 1 print(buffer) optionCount;
	action.value = (void*)((long)optionCount);
    currentRule = getRule(buffer.toString());
    if immediate
		currentRule.immediateAction = action;
    else currentRule.deferAction = action;
;

ActionRule!
.
    PLGitem atOption = list;
    cout "Action: " ruleName "\n";
    ruleActionName = ruleName;
    optionCount = 1;
    while atOption
		{
		atOption.runDeferred();
		atOption = atOption.next;
		}
    hasActions = true;
;

/*****************************************************************************
	Grammar for specifying actions
*****************************************************************************/
ActionEnd	:   '\n'& [.|;]&
			;

ActionBody	:   ActionEnd!&
				body = ActionEnd{
			;

ActionOption	:
				option = '|'+
			|
				immediate =	'.'?&
                ';'!&
				action	  = ActionBody&
			;

ActionRule	:   ruleName = Name
                '!'
				list	 = ActionOption+
				';'
			;
