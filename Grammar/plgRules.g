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

FileName        :   path    = [/._a-zA-Z0-9]+
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

/*****************************************************************************
    Comment uses a CommentBody helper rule to handle the multi-char
    `*/` terminator. The naive `'/*' body = ~.+ '*/'` form does not work
    because kAny is greedy and does not backtrack — body+ would eat past
    the `*/` to EOF, then the closing `*/` element would fail. CommentBody's
    two-alt structure (any-non-asterisk OR asterisk-then-non-slash) stops
    cleanly when it sees `*/`, leaving the closer for the mandatory `'*/'`
    element to consume. Same pattern as the hand-written meta-grammar in
    PLG.twk's CommentPartBoDY.
*****************************************************************************/
Comment         :   '/*'
                    body    = CommentBody*
                    '*/'
                ;

CommentBody     :   notStar  = [^*]
                |
                    '*' nonSlash = [^/]
                ;

QuotedString    :   singleQuote
                    text    = [^']+
                    singleQuote
                ;

Command         :   'setSKIP'
                ;
