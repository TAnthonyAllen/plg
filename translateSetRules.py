#!/usr/bin/env python3
"""
Translate old PLG setRules() addTest() format to new Alternative/Element format.
Reads plg.twk, extracts setRules(), outputs new tok-style setRules().

addTest type mapping:
    1 -> kAny      (4)
    2 -> kChr      (2)  
    3 -> kCondition (8)
    4 -> kEof      (5)
    5 -> kRuleRef  (6)
    6 -> kSet      (3)
    7 -> kLit      (1)
    8 -> kKeyTable (7)
    9 -> kVariable (9)
"""

import re
import sys

TYPE_MAP = {
    '1': ('kAny',      4),
    '2': ('kChr',      2),
    '3': ('kCondition',8),
    '4': ('kEof',      5),
    '5': ('kRuleRef',  6),
    '6': ('kSet',      3),
    '7': ('kLit',      1),
    '8': ('kKeyTable', 7),
    '9': ('kVariable', 9),
}

def extract_setrules(text):
    """Extract the body of setRules() from the file."""
    start = text.find('void setRules()')
    if start == -1:
        print("ERROR: could not find setRules()", file=sys.stderr)
        return None
    # Find opening brace
    brace_start = text.find('{', start)
    if brace_start == -1:
        return None
    # Find matching closing brace
    depth = 0
    i = brace_start
    while i < len(text):
        if text[i] == '{':
            depth += 1
        elif text[i] == '}':
            depth -= 1
            if depth == 0:
                return text[brace_start+1:i].strip()
        i += 1
    return None

def parse_addtest(line):
    """Parse addTest(type, data, label, min, max, skipSet) call."""
    # Match addTest(...)
    m = re.match(r'\s*addTest\((.+)\);\s*$', line)
    if not m:
        return None
    args_str = m.group(1)
    # Split args carefully (commas inside parens/quotes don't count)
    args = []
    depth = 0
    current = ''
    in_string = False
    for c in args_str:
        if c == '"' and not in_string:
            in_string = True
            current += c
        elif c == '"' and in_string:
            in_string = False
            current += c
        elif c == '(' and not in_string:
            depth += 1
            current += c
        elif c == ')' and not in_string:
            depth -= 1
            current += c
        elif c == ',' and depth == 0 and not in_string:
            args.append(current.strip())
            current = ''
        else:
            current += c
    if current.strip():
        args.append(current.strip())
    
    if len(args) < 6:
        return None
    
    return {
        'type': args[0],
        'data': args[1],
        'label': args[2],
        'min': args[3],
        'max': args[4],
        'skipset': args[5],
    }

def translate_data(test_type, data):
    """Translate data argument based on type."""
    if test_type == '5':  # kRuleRef
        m = re.match(r'\(void\*\)getRule\("(.+)"\)', data)
        if m:
            return f'getRule("{m.group(1)}")'
        return data
    elif test_type == '6':  # kSet
        if data == '(void*)currentSet':
            return 'currentSet'
        return data
    elif test_type == '7':  # kLit
        m = re.match(r'\(void\*\)"(.+)"', data)
        if m:
            return f'"{m.group(1)}"'
        return data
    elif test_type == '8':  # kKeyTable
        m = re.match(r'\(void\*\)(\w+)', data)
        if m:
            return m.group(1)
        return data
    return data

def translate_setrules(body):
    """Translate setRules body to new format."""
    lines = body.split('\n')
    out = []
    out.append('void setRules()')
    out.append('{')
    
    current_rule = None
    current_alt_elements = []
    pending_modifiers = []  # currentTest.xxx lines
    
    def flush_alternative():
        """Output current alternative and reset."""
        nonlocal current_alt_elements, pending_modifiers
        if not current_alt_elements:
            return
        out.append('    {')
        out.append('    Alternative alt = new;')
        for elem_lines in current_alt_elements:
            out.append('    {')
            out.append('    Element elem = new;')
            for el in elem_lines:
                out.append(f'    {el}')
            out.append('    alt.elements += elem;')
            out.append('    }')
        out.append(f'    currentRule.alternatives += alt;')
        out.append('    }')
        current_alt_elements = []
        pending_modifiers = []

    i = 0
    while i < len(lines):
        line = lines[i]
        stripped = line.strip()
        
        # Skip empty lines and comments
        if not stripped or stripped.startswith('//'):
            out.append(line)
            i += 1
            continue
        
        # getSet calls (named sets)
        m = re.match(r'(\w+)\s*=\s*getSet\(\s*"(\w+)"\s*,\s*"(.+)"\s*\)\s*;', stripped)
        if m:
            out.append(f'    {stripped}')
            i += 1
            continue

        # currentSet = getSet(...) 
        if stripped.startswith('currentSet = getSet('):
            out.append(f'    {stripped}')
            i += 1
            continue
            
        # currentRule = getRule(...)
        m = re.match(r'currentRule\s*=\s*getRule\("(.+)"\)\s*;', stripped)
        if m:
            flush_alternative()
            current_rule = m.group(1)
            out.append(f'    currentRule = getRule("{current_rule}");')
            i += 1
            continue
        
        # currentRule.xxx = yyy (action callbacks, flags)
        m = re.match(r'currentRule\.(\w+)\s*=\s*(.+);', stripped)
        if m:
            out.append(f'    currentRule.{m.group(1)} = {m.group(2)};')
            i += 1
            continue
        
        # currentRule.next = getRule(...) — retired, skip
        if 'currentRule.next' in stripped:
            out.append(f'    // RETIRED: {stripped}')
            i += 1
            continue

        # currentTest.xxx = yyy — store as modifier for current element
        if stripped.startswith('currentTest.'):
            pending_modifiers.append(stripped)
            i += 1
            continue

        # addTest(...)
        test = parse_addtest(stripped)
        if test:
            type_str = test['type']
            kind_name, kind_val = TYPE_MAP.get(type_str, ('kUnknown', 0))
            
            elem_lines = []
            elem_lines.append(f'elem.kind = {kind_val}; // {kind_name}')
            elem_lines.append(f'elem.minimum = {test["min"]};')
            
            max_val = test['max']
            if max_val == '268435455':
                max_val = '999999'
            elem_lines.append(f'elem.maximum = {max_val};')
            
            data = translate_data(type_str, test['data'])
            
            if type_str == '5':  # kRuleRef
                elem_lines.append(f'elem.ruleRef = {data};')
            elif type_str == '6':  # kSet
                elem_lines.append(f'elem.setRef = currentSet;')
            elif type_str == '7':  # kLit
                elem_lines.append(f'elem.litText = {data};')
            elif type_str == '8':  # kKeyTable
                elem_lines.append(f'elem.tableRef = {data};')
            elif type_str == '1':  # kAny
                pass  # no data needed
            elif type_str == '4':  # kEof
                pass  # no data needed
            
            label = test['label']
            if label != '(String)null' and label != 'null':
                elem_lines.append(f'elem.label = {label};')
            
            skipset = test['skipset']
            if skipset != '(String)null' and skipset != 'null':
                elem_lines.append(f'elem.skipSet = getSet({skipset});')
            
            # Add any pending modifiers
            for mod in pending_modifiers:
                elem_lines.append(f'// {mod}')
            pending_modifiers = []
            
            current_alt_elements.append(elem_lines)
            i += 1
            continue
        
        # Everything else passes through
        out.append(f'    {stripped}')
        i += 1
    
    flush_alternative()
    out.append('}')
    return '\n'.join(out)

def main():
    if len(sys.argv) < 2:
        print("Usage: translate_setrules.py input.twk [output.twk]", file=sys.stderr)
        sys.exit(1)
    
    with open(sys.argv[1], 'r') as f:
        text = f.read()
    
    body = extract_setrules(text)
    if not body:
        print("ERROR: could not extract setRules()", file=sys.stderr)
        sys.exit(1)
    
    result = translate_setrules(body)
    
    if len(sys.argv) >= 3:
        with open(sys.argv[2], 'w') as f:
            f.write(result)
        print(f"Written to {sys.argv[2]}")
    else:
        print(result)

if __name__ == '__main__':
    main()
