%%
Set	excludeSet		[A-Za-z:.|];

include(Testing.act)

Name			:   name = [._a-zA-Z0-9]+
				;

StringSet       :   '[' text = ']'}&
				|
					name	= Name
				;

Test			:	'Set'		excludeSet!&
					name	= Name
					set     = StringSet?
                    ';'?

				;
%%
