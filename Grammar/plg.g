include PLGincludes

%%
insert(plg.rtn)

Set	alternateSet	[#([~'a-zA-Z] ;
Set characterSet	[[a-zA-Z] ;
Set	commandSet		[derst] ;
Set	commentSet		[derst/] ;
Set elementSet		[[(a-zA-Z~'] ;
Set	excludeSet		[A-Za-z:.|] ;
Set	nameSet			[a-zA-Z] ;
Set numberSet       [0-9] ;
Set	singleQuote		['] ;

Rules		Max
			Name
			SetAssignment
			;
include(plgRules.g)
/* include(action.g) deferred until Action-blocks feature lands; current parser cannot ingest ActionEnd/ActionBody/ActionOption bodies */
include(plg.act)

/*****************************************************************************
	Main parser grammar
*****************************************************************************/
setSKIP
Alternative		:   'FAIL'	exception = ( Name | QuotedString )
				|
					guard		= Guard?
					atLabel		= Label?
                    #[^)|]
					atElement	= ( Balanced | Block )
				|
					#commentSet
					Comment
				;

BalanceRight	:   '<>'
					element	= Element
					type	= ElementType?
					noSkip	= '&'?
				;

Balanced		:   left	= Element
					type	= ElementType?
					noSkip	= '&'?
					right	= BalanceRight?
				;

Block			:   '('
					atAlternative = Alternative+
					clause	= Clause*
					')'
					FAIL 'Missing right parenthesis'
					type	= ElementType?
					noSkip		= '&'?
				;

Body			:   #commentSet
					comment = Comment
				|
					SetVariable
				|
					Include
				|
					atRule	= Rule
				|
                	error	= '\n'}&
				;

StringSet       :   '[' text = ']'}&
				|
					name	= Name
				;

Clause			:   '|'
					atAlternative = Alternative+
				;

Element			:   Command!
					atElement	= Name
				|
					atElement	= QuotedString
				|
					atElement	= StringSet
				|
					'~' atElement	= [.$]
				;

ElementType		:   '{' minimum = Integer maximum = Max? '}'
				|
                	option		= [*+?!%{}]
				;

ForwardReference:
					name	= Name
				;

Guard			:	'#'
					set	= StringSet
				|
					'#doNotGuard'
				;

Include			:   'include(' file = ')'}
				|
					'insert(' file = ')'}
                |
                    '-%'
                    code    = '%-'}&
				|
					'include' file = FileName
				;

Max				:   ',' maximum = Integer
				;


OptionClause	:   '|'
					dummy = RuleOption
				;

ParseInclude	:   Body+
				;

Rule			:   ruleName = Name
					':'
					options	= RuleOptions
					FAIL 'Rule specification failed'
					';'
				;

RuleOption		:   #doNotGuard
                    atAlternative	= Alternative+
				;

RuleOptions		:   first	= RuleOption
					others	= OptionClause*
				;

DebugText       :
                    text    = singleQuote <> singleQuote
                |
                    text    = [a-zA-Z0-9]+
                ;

DebugOption     :
                    '='
                    option  = DebugText
                ;

DebugRule       :
                    name        = Name
                    option      = DebugOption?
                ;

SetVariable		:   'Variable'	excludeSet!&
					variable	= Name+
					';'
				|
                	'Rules'		excludeSet!&
					ForwardReference+
					';'
				|
                	'KeyWord'	excludeSet!&
					name	= Name
					list	= Word*
					';'
				|
                	'Set'		excludeSet!&
					name	= Name
					set     = StringSet?
                    ';'
                |
                    'Condition'	excludeSet!&
					condishun	= Name+
					';'
                |
                    'Debug'    excludeSet!&
                    rulename    = DebugRule*
                    ';'
				;

Start			:	Header*
					'%%'
					Body+
					'%%'?
					Trailer?
				|
                	error = ([^\n]+ '\n')
				;
%%
/*****************************************************************************
	main program
*****************************************************************************/
int
main( int argc, char **argv )
{
plg	parser	= new();
int i;
    cout "Arguments";
    for ( i = 0; i < argc; i++ )
        cout ,argv[i];
    cout :;
    if argc == 2
		{
		//parser.debug = true;
		parser.process(argv[1]);
		cout "Parsed it\n";
		//parser.summary(5);
		}
    else {
		cerr "usage: plg sourceFile" "\n";
		}
}
