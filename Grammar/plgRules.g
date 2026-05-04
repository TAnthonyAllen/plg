/*****************************************************************************
    plg_rules.g — clean rule definitions for the names that plg.g
    forward-references but doesn't define inline. Loaded via include
    so the meta-grammar's fresh rules table picks them up.
    Simplified where the original parts.g definitions used grammar
    constructs the meta-grammar can't yet parse (escape sequences,
    complex Block / BalanceRight forms). The structural intent is
    preserved — the runtime semantics are pragmatic approximations.
*****************************************************************************/

Set singleQuote     [']
Set excludeSet      [A-Za-z:.|]

Name            :   name    = [._a-zA-Z0-9]+
                ;

Label           :   atLabel = Name '='
                ;

Integer         :
                    [ \f\r\t\n]*
                    [0-9]+
                ;

Word            :   what    = [^ \f\r\t\n;]+
                ;

SetName         :   set     = Name
                ;

SetAssignment   :   '='
                    set     = SetName
                ;

Header          :   atInclude = Include
                ;

Tail            :   '%%'
                    code    = ~.+
                ;

Trailer         :   code    = ~.+
                ;

Comment         :   '/*'
                    body    = ~.+
                    '*/'
                ;

QuotedString    :   singleQuote
                    text    = [^']+
                    singleQuote
                ;

Command         :   'setSKIP'
                ;
