
Command!
.
    if reset
		{
		skipping = false;
		skipSet = null;
		}
    else {
		skipping = true;
		if set
			{
			skipSet = setTable[set.string()];
			set.unString();
			}
		else skipSet = defaultSkip;
		}
|.
    debugRulePLG = !debugRulePLG;
;

FileName!
.
    buffer.reset();
    parserName = name.toString();
    print(buffer) parserName ".twk";
	output.setFile(buffer.toString());
;

Header!
.
String  extended = "PLGparse";
    if name
        {
        parentParser = name.toString();
        extended = parentParser;
        }
    if head && head.itemLength head.toBuffer(output);
    print(headerBuffer) "
/"
"*****************************************************************************
	" parserName " class definition
*****************************************************************************/
class " parserName " extends " extended:"{":;
;

SetName!
.
int result;
String  name = set.string();
	if setTable[name] result = true;
    set.unString();
    if !result  return false;
;

Tail!
.
    ((plg)iTEM.testParser).tail = code;
;

/*****************************************************************************
	Odd rules used in grammar
*****************************************************************************/
resetSKIP
Alpha			:	[a-zA-Z0-9_.]+
				;

TrailingSpace	:	[ \f\r\t]* '\n'
				;

setSKIP
Command			:
					reset = 're'? 'setSKIP'
					set = SetAssignment?
				|   'dEBUG'
				;

CommentPart		:	'/*' <> '*/'
				|	'//' '\n'}&
				;

Comment			:
					#[/]
					CommentPart
				|   Command
				;

Extender        :
                    'eXTENDS'
                    name    = Name
                ;

FileName		:
					'/'?
					(Alpha '/')*
					name	= [A-Za-z0-9_]+
					'.g'
				;

Header			:	head    = '%%'}
                    name    = Extender?
				;

Integer			:
					[ \f\r\t\n]*
					[0-9]+
				;

Label			:   atLabel = Name '='
				;

Name			:   name = [._a-zA-Z0-9]+
				;

QuotedString	:
					singleQuote
					text = (('\\' ~. | [^'])+)
					singleQuote
				|
					text = [.$]
				;
			
SetName			:   set	= Name
				;

SetAssignment	:
					'='
					set	= SetName
				;

Tail			:	'%%' code = ~.+
				;

Word			:   what = [^ \f\r\t\n;]+
				;
