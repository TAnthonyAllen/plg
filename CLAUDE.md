# CLAUDE.md — PLG Repository

This file orients Claude Code (Clod) when working in this repository.
Read `projectBible.md` for the full ecosystem context (PLG/TAWK/Incant).

---

## What PLG Is

PLG is a parser/generator — a fast, declarative pattern matcher and C++ code generator.
It reads grammar files (`.g`), builds rule structures in memory, and generates C++ code (`setRules()`) that implements those rules efficiently.

PLG is one of three interwoven projects: **PLG** (recognizes), **TAWK** (transforms), **Incant** (reasons).
Working on PLG often involves the others. See `projectBible.md`.

---

## Repository Structure

```
plg/
├── PLGparse.twk/.C/.h    — Base parser class. Buffer-based input, cursor/eof,
│                           rules/setTable hashes, divertInput/revertInput
├── PLG.twk/.C/.h         — PLG-specific subclass. Contains setRules() and
│                           all plg grammar action callbacks
├── PLGrule.twk/.C/.h     — One grammar rule. Has alternatives, guardSet,
│                           action callbacks (defer/immediate/fail)
├── Alternative.twk/.C/.h — One option within a rule. Has elements, guardSet
├── Element.twk/.C/.h     — One match unit. Kind, min/max, setRef/litText/ruleRef
├── PLGitem.twk/.C/.h     — Match result. Buffer reference + offset + length
├── PLGrevision           — TAWK externals file (like a .h for tok's benefit)
├── setRules.twk          — Hand-written bootstrap setRules() for testing
├── translateSetRules.py  — Translates old PLG addTest() format to new Alternative/Element format
├── projectBible.md       — Ecosystem-wide architecture document
└── .gitignore
```

**Support classes** (not in this repo — see **https://github.com/TAnthonyAllen/support**):
- `Buffer`, `DoubleLinkList`, `DoubleLink`, `PLGset`, `BaseHash`, `Stak`, `Tape` — under `Frame/`
- `KeyMap`, `KeyTable` — under `KeyTable/`
- `BitMAP`, `Segment` — under `Maps/`
- TAWK external-declaration files (`.ext` and friends) — under `Include/`

The actual files live at `~/data/support/` (which resolves to
`~/Library/CloudStorage/Dropbox/data/support/`). The original
`InProcess/` paths are now **symlinks** pointing into the support repo:

- `InProcess/Frame`        → `~/data/support/Frame`
- `InProcess/Include`      → `~/data/support/Include`
- `InProcess/KeyTable`     → `~/data/support/KeyTable`
- `InProcess/Groups/Maps`  → `~/data/support/Maps`

So existing builds and `.twk` `include` directives that point at the
InProcess paths keep working.

---

## File Types

- `.twk` — TAWK source files (Tony's AWK). **These are the source of truth.** Edit these, not the generated files.
- `.C` / `.h` — Generated C++ output from TAWK. Regenerate with `tawk FileName.twk`
- `.g` — PLG grammar files. Fed to PLG to generate `setRules()`
- `PLGrevision` — TAWK externals. Tells TAWK about PLG classes, fields, and methods.

**Never edit `.C` or `.h` files directly** — changes will be overwritten next time TAWK runs.

---

## Build Workflow

```bash
# 1. Edit .twk source
# 2. Regenerate C++
tawk PLGparse.twk        # produces PLGparse.C and PLGparse.h
tawk PLGrule.twk         # produces PLGrule.C and PLGrule.h
tawk Alternative.twk     # etc.
tawk Element.twk
tawk PLGitem.twk
tawk PLG.twk             # largest file — contains setRules()

# 3. Build in Xcode or via command line
# 4. Run tests
```

**Important TAWK quirks:**
- Empty `//` comment lines reset field resolution context — remove them from method bodies
- `field = new;` sometimes fails type inference (actually, only when there is a preceding empty comment)— use `field = new ClassName();` explicitly in that case (best avoided by not having empty comments but lets fix the bug anyway)
- No `#include` generated in `.h` files — for inherited classes add `#include "Parent.h"` manually after every re-tawk
- After adding include guards to `.h` files, they must be re-added after re-tawk (tawk doesn't generate them)

---

## Core Architecture

### The Match Chain
```
PLG::parse(ruleName)
  → PLGrule::match(state)
    → Alternative::match(state, out)
      → Element::match(state)
        → Element::matchByKind(state)
          → matchSet / matchLit / matchRuleRef / matchAny / matchEof / etc.
            → Element::applyRepetition(state)
```

### Element Kinds (kind field — 4-bit enum)
```
kLit = 1        literal string match
kChr = 2        single character match  
kSet = 3        character set match (PLGset)
kAny = 4        match any character
kEof = 5        match end of input
kRuleRef = 6    match by calling another rule
kKeyTable = 7   keyword table match
kCondition = 8  conditional function match
kVariable = 9   variable match
kUpTo = 10      match up to delimiter
kBalanced = 11  balanced delimiter match
```

**IMPORTANT**: `kSet(button)` is a test macro `(button == 3)` — NOT assignable.
For assignment use the numeric value: `elem.kind = 3; // kSet`

### Buffer-Based Input
All input lives in Buffer objects. No raw pointer arithmetic. No writable string issues.
```
divertInput(s):  push current buffer → install new buffer → set cursor/eof
revertInput():   pop buffer → restore cursor/eof
```

### Safe DoubleLinkList Iteration
**CRITICAL**: Never use the default `next()` iterator in recursive contexts.
It uses shared `entry` state that gets clobbered by nested calls.

Always use the safe `for` loop pattern:
```cpp
for (DoubleLink *link = list->first; link; link = link->next) {
    MyClass *item = (MyClass*)link->value;
    // process item
}
```

In TAWK source:
```
for link = list.first {
    item = link.value;
    // process
    }
```

**Do not** add an explicit `link = link.next;` inside the body — TAWK's
`for x = list.first { ... }` already auto-advances. Explicit increment
double-advances and segfaults on multi-alternative rules.

---

## PLG Class (PLG.twk)

PLG extends PLGparse via **composition** (not inheritance) due to C++ incomplete type issues with TAWK-generated headers.

```
class PLG {
    PLGparse parser;    // contains via composition, not extends
    ...
}
```

`setRules()` lives in PLG. It builds all the PLGrule/Alternative/Element structures for parsing `.g` grammar files.

### The Bootstrapping Problem
PLG is supposed to generate `setRules()` but needs `setRules()` to parse `plg.g` to generate `setRules()`.

**Current bootstrap sequence:**
1. Old PLG binary runs on `plg.g` → generates `plg.twk` (old format)
2. `python3 translateSetRules.py plg.twk new_setRules.twk` → translates to new format
3. Paste `setRules()` from `new_setRules.twk` into `PLG.twk`
4. Re-tok `PLG.twk` → `PLG.C`
5. Compile and test

**Long term goal**: New PLG parses `plg.g` and generates its own `setRules()`. Self-hosting.

---

## Current State

### Working
- PLGparse, PLGrule, Alternative, Element, PLGitem compiling and running
- Buffer-based input with divertInput/revertInput
- matchSet, matchLit, matchRuleRef working correctly
- applyRepetition working
- Safe DoubleLinkList iteration established
- Proven test: input `,678` parsed by Max rule → `matched 4 chars: ,678`

### Known Issues / In Progress
- PLG composition (contains PLGparse) works but is a workaround — real fix is in TAWK
- generators (generateRules, PLGrule.generate, Alternative.generate, Element.generate) stubbed out — need implementation
- setRules() is hand-written bootstrap — needs to be generated by PLG itself
- Guards not yet implemented (stubbed) — optimization, not correctness issue
- kSet/kRuleRef etc. assigned as numeric literals due to macro conflict

### Next Steps
1. Implement `addTest()` in Element — reduces setRules() verbosity dramatically
2. Feed `testing.g` through parser instead of hardcoded setRules()
3. Expand to `plg.g` — full bootstrap
4. Implement generators properly
5. Guards

---

## Testing

Current test in `PLG::run()`:
```
setRules();
initialize();
setInput(",678");
PLGitem result = parse("Max");
// expected: matched 4 chars: ,678
```

Grammar under test is in `setRules.twk` — hand-written bootstrap for `Max` and `Integer` rules.

---

## Working Relationship

**Anthony** — architect, final authority on design.
**Claude (claude.ai chat)** — design, reasoning, architecture.
**Clod (Claude Code)** — execution, file changes, GitHub.

**Standing permissions**: Change any code in source directories without asking.
Trivial repo operations (commits, pushes for routine work) happen at Clod's
discretion; flag non-trivial or uncertain situations before acting. Same
commit-and-push freedom — no verification round-trip on routine work.

See `projectBible.md` for the full glossary, HWF protocol, and ecosystem context.
