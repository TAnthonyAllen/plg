# The PLG/TAWK/Incant Project Bible
*A living document. Every sentence earns its place.*

---

## The Ecosystem in One Breath

**PLG** finds the pieces — a fast, declarative pattern matcher. Domain-specific, stateless, embeddable.  
**TAWK** shapes the expression — a source-to-source transpiler. Reads tawk syntax, outputs C++.  
**Incant** understands the meaning — reflexive, homoiconic, stack-aware, self-describing.

*PLG recognizes. TAWK transforms. Incant reasons.*

The three are one ecosystem. Working on any one inevitably involves the others.  
To refactor TAWK you first need to fix PLG. To finish Incant you need both.  
The design follows elegance; the classifications are just other nerds recognizing what was already there.

---

## Repositories

| Repo | URL | Status |
|------|-----|--------|
| PLG | https://github.com/TAnthonyAllen/plg | public ✅ |
| Incant | https://github.com/TAnthonyAllen/incant | public ✅ |
| Support | https://github.com/TAnthonyAllen/support | public ✅ |
| TAWK | https://github.com/TAnthonyAllen/tawk | public ✅ |

Each repo has a `CLAUDE.md` for Clod orientation and this bible (mirrored).

---

## Local Directory Map

```
/Users/anthony/Library/CloudStorage/Dropbox/data/
    InProcess/
        Parse/      — PLG source (plg repo)
        Tokf/       — TAWK source (tawk repo)
        Groups/     — Incant source (incant repo)
        Include/    — symlink → ~/data/support/Include
        Frame/      — symlink → ~/data/support/Frame
        Groups/Maps — symlink → ~/data/support/Maps
    support/        — shared support classes (support repo)
        Frame/      — Buffer, DoubleLinkList, PLGset, BaseHash, Stak, Tape, StringRoutines
        Include/    — TAWK externals (frame, maps, globals, plg.ext, groups.ext)
        KeyTable/   — keyword lookup
        Maps/       — BitMAP, Segment
```

---

## Language Classifications

| Project | Type | Key Properties |
|---------|------|----------------|
| PLG | DSL / pattern matcher | Declarative, stateless, fast, embeddable |
| TAWK | Transpiler | Source-to-source, metaprogramming, syntactic sugar for C++ |
| Incant | General purpose | Reflexive, homoiconic, stack-aware, generates to C++ |

**DSL** — Domain-Specific Language. Designed for one purpose. PLG only matches patterns.  
**Reflexive** — the language can examine and describe itself.  
**Homoiconic** — code and data have the same structure. A GroupItem field IS the rule that describes it.  
**Transpiler** — compiles to source code (C++) rather than machine code.  
**Declarative** — describes what to match, not how to match it.

---

## Architecture

### PLG Core Classes (new architecture)
- **PLGparse** — base parser. Buffer-based input, cursor/eof, rules/setTable hashes, divertInput/revertInput stack, addTest(), getRule(), getSet(), parse()
- **PLGrule** — one grammar rule. Has alternatives (DoubleLinkList), guardSet, action callbacks (defer/immediate/fail)
- **Alternative** — one option within a rule. Has elements (DoubleLinkList), guardSet
- **Element** — one match unit. Kind (kLit/kChr/kSet/kAny/kEof/kRuleRef/kKeyTable/kCondition/kVariable/kUpTo/kBalanced), min/max, setRef/litText/ruleRef etc.
- **PLGitem** — match result. Buffer reference + offset (not raw pointer), itemLength, toString()
- **PLG** — contains PLGparse via composition. setRules() and all plg-specific grammar/actions live here.

### Key Design Decisions
1. **Buffer-based input** — all input lives in Buffer objects. divertInput pushes current buffer, installs new one. No raw pointer arithmetic. No writable string problems.
2. **Safe iteration** — use `for (link = list->first; link; link = link->next)` pattern everywhere. The explicit `link = link->next` inside the loop body is WRONG — TAWK's for loop already advances. Never add a manual advance inside the body.
3. **Composition over inheritance** — PLG contains a PLGparse field rather than extending it (workaround for C++ incomplete type issues with TAWK-generated headers).
4. **toString() only** — PLGitem no longer null-terminates in place. toString() returns a malloc'd copy. string()/unString() retained for backward compatibility with support code.
5. **GC friendly** — tape allocator retired. New/delete or GC handles memory.
6. **Single exit point** — match methods use result variable + break pattern for debuggability.
7. **extern "C" bridge functions** — functions that bridge two types (neither owns the other) are extern "C" free functions. Pattern established by plg action callbacks (plgNow, plgAct). foundIn(PLGset, PLGitem) follows this pattern.
8. **Method ordering** — alphabetical by convention (Anthony's preference for Xcode navigability). Not a TAWK requirement.
9. **TAWK error propagation** — unresolved references are embedded as errors in generated C++ at the point of use. This is a feature — errors appear in context exactly where they matter. Xcode flags them.

### The Match Chain
```
PLG::parse(name)
  → PLGrule::match(state)
    → Alternative::match(state, out)
      → Element::match(state)
        → Element::matchByKind(state)
          → matchSet / matchLit / matchRuleRef / etc.
            → Element::applyRepetition(state)
```

### Input Diversion
```
divertInput(s):
    inputStack.push(buffer)
    buffer = new Buffer(s)
    cursor = buffer.start
    eof = buffer.end

revertInput():
    buffer = inputStack.pop()
    cursor = buffer.current
    eof = buffer.end
```

### addTest() — setRules() shorthand
```
addTest(kind, data, label, min, max, skipSet)
```
Creates an Element, sets all fields, adds to currentAlt. Reduces setRules() from 8 lines per element to 1.
`currentAlt` is a PLGparse field set before calling addTest(). `currentRule` similarly.

### Guards
Each PLGrule has a guardSet — the FIRST set of characters that could start any of its alternatives. Before trying alternatives, the parser checks if the current character is in the guardSet. Fast rejection without trying every alternative.

`setGuard()` is called from `initialize()` after `setRules()`. It recurses into kRuleRef elements with cycle protection via `guardComputed` flag. Rules with `doNotGuard = true` return null — their callers must propagate null upward (null guard = accept anything; empty PLGset = reject everything — these are NOT the same).

Zero-advance-stop: if a rule "succeeds" without consuming input, it's treated as failure. Prevents infinite loops on optional elements.

### Two-Table Pattern (process())
```
plgRuleTable = parser.rules;      // save meta-grammar
plgSetTable  = parser.setTable;
parser.rules    = new;            // fresh tables for output
parser.setTable = new;
parse(start);                     // callbacks build into fresh tables
parser.rules    = plgRuleTable;   // restore meta-grammar
parser.setTable = plgSetTable;
```
Action callbacks (RuleplgNow, ElementplgAct, etc.) fire during parse and populate the fresh tables with new rule structures.

---

## The Bootstrapping Problem

PLG is supposed to generate setRules() but needs setRules() to parse plg.g to generate setRules().

**Current bootstrap sequence:**
1. Old PLG binary runs on `plg.g` → generates `plg.twk` (old addTest format)
2. `python3 translateSetRules.py plg.twk new_setRules.twk` → translates to new format
3. Paste `setRules()` from `new_setRules.twk` into `PLG.twk`
4. Run `tawk PLG.twk` → `PLG.C`
5. Compile and test

**Long term goal**: New PLG parses `plg.g` and generates its own `setRules()`. Self-hosting.

**Same problem exists for TAWK**: building tawk-from-source requires a working tawk binary. Currently solved by `~/bin/tok`. Any clean-checkout build story needs to address this.

---

## TAWK Known Issues (autopsy table)

1. **Empty `//` comment lines** reset field resolution context — causes `new` to fail type inference and field references to be lost. Fix: context tracking should be comment-blind.
2. **Implicit field resolution** is complex machinery. Incant solves this elegantly with `lastREF`/`@context`. TAWK refactor: replace implicit resolution with explicit `@field` context markers.
3. **No include guards** generated in .h files. Must add manually for inherited classes.
4. **`new` type inference** inconsistent — `alt = new` sometimes fails to resolve to `new Alternative()`. Workaround: explicit `alt = new Alternative()`. Known quirk, not a blocker.
5. **No `#include` in .h files** — inherited classes need parent .h included but TAWK never generates includes in .h files. Must add manually after every re-tawk.
6. **Unused field warning** — TAWK silently drops unused declared fields instead of warning. Fix: emit warning, keep field.
7. **`kSet` macro conflict** — `kSet(button)` is a test macro, not an assignable value. Kind assignment requires numeric literal. Fix: separate test macros from enum values.
8. **`extern "C"`** in tawk-generated files gets clobbered on re-tawk. Free functions needing C linkage must live in hand-written non-generated files.
9. **No include search paths** — all includes must be absolute paths. No `-I` flag support. Add in TAWK refactor.
10. **TAWK iteration trap** — `for (link = list->first; link; link = link->next)` already advances. Adding `link = link->next` inside the body causes double-advance and SIGSEGV on multi-alt rules. Never add manual advance inside the body.
11. **TAWK external as forward reference** — a bare `DefinE;` in incant and an empty tok external block are the same concept: "this exists, details later." TAWK refactor: make this more incant-like.

---

## Incant / PLG Convergence

- `match()` — atomic recognition. Did this pattern occur here?
- `parse()` — semantic processing. What does it mean that it occurred?
- In PLG these are separate. In Incant they collapse because the data structure IS the grammar IS the result.
- **The bridge**: PLGitem is a lightweight match result. GroupItem wraps PLGitem when semantics are needed. Promotion is lazy — only when incant needs to reason about the result.
- **Input diversion**: same concept in both. PLG: push/pop Buffer. Incant: push/pop GroupItem containing buffer. Same contract, appropriate implementation for each.
- **defer**: in PLG a callback convention (runDeferred). In Incant a first-class keyword in the grammar itself. Same fundamental insight — some actions must happen after the match completes, not during.
- **Labels in Incant**: isLabel flag marks a field as a parse result, named the same as the rule that produced it. isRule marks a field as a rule definition. Same tag name, different flags. Everything inside a label context is isLabel — mixing is a bug (iron maiden for the programmer).
- **The north star**: PLG written in Incant, parsing itself. Not today. But the architecture points there.

---

## The Long Game — Incant as Distributed Virtual OS

*This is where it's all heading. HWF territory, but grounded.*

A GroupItem is not just a data structure — it's a deployable unit. It runs wherever there's a runtime: Mac, iOS, Android, Windows, cloud. GroupItems message each other across those boundaries. Location transparent — you don't care if the GroupItem you're talking to is local or in AWS.

Work distribution is natural: *"You have the GPU cluster, handle the matrix math. Send me the result."* The GroupItem doesn't know or care where the work happens. It just messages and receives.

**The file system becomes GroupItems.** Incant doesn't sit ON the file system — incant REPRESENTS it. Files are fields. Directories are groups with members. Navigation is group traversal. MongoDB or any storage is the persistence layer — incant's in-memory GroupItem graph serializes transparently.

**Claude is one node in the network.** A GroupItem in AWS with access to large datasets. You message it, it messages back. The API call is just messaging. The response is just a GroupItem. Claude is not a tool called from incant — Claude is a field type in incant, indistinguishable from any other GroupItem except it reasons instead of computes.

**JSON is just four or five incant rules.** An incant field definition says the same thing as JSON with a fraction of the noise. The ugliness of JSON is the syntax tax.

**The JIT is the enabling technology.** Without JIT, incant is an interpreter. With JIT, incant compiles GroupItems into deployable units that can be shipped to wherever the work needs to happen. The JIT is what closes the self-definition loop — when incant actions can become compiled methods, the C++ bootstrap layer starts shrinking.

**iMessage, WhatsApp, as a GroupItem** — not a separate app. One field definition. *"Oh by the way, look what I can do in just one of my little fields."*

---

## generateRules() / setRules() relationship

`generateRules()` is PLG's primitive bytecode compiler — it promotes interpreted grammar structures to compiled C++.  
Incant's bytecode work is the same idea at higher abstraction.  
`parse()` orchestrates. `match()` is atomic. `generateRules()` compiles.

---

## Support Library Architecture

Support classes are shared across PLG, TAWK, and Incant. They live in the support repo.

**Dependency rule**: support classes must not depend on PLG/TAWK/Incant classes. If a dependency creeps in, extract a bridge function (`extern "C"`) rather than adding an include.

**Known bridge**: `foundIn(PLGset*, PLGitem*)` — lives in PLGparse.C as `extern "C"`. PLGset needs to check if a PLGitem matches a set, but PLGset must not include PLGitem.h.

**Target architecture**: support as a static library (libsupport.a), linked into each tool. ✅ Done.

**PLGset note**: PLGset was previously in Parse (PLG source). Moved to support/Frame. It's general-purpose pattern-set machinery used by all three projects.

---

## Working Relationship

**Anthony** — architect, domain expert, final authority on design decisions. Coding as thinking out loud.  
**Clay** (Claude at claude.ai) — design, reasoning, architecture, HWF navigation, code review. Renamed from Claude for brevity.  
**Clod** (Claude Code) — execution, file edits, GitHub maintenance, build verification. Named for brevity, not disparagement.

**Standing permissions**: Clod may change any code in source directories without asking. Ask before GitHub pushes.  
**HWF protocol**: When design goes near a cliff, flag it. The face plants teach as much as the successes.  
**Convention of one**: tawk not tok in source (Tony's awk). Conventions are conventions even with one member.  
**The cha cha**: Clay designs, Clod executes, Anthony architects. Each dancer has a distinct role.  
**Clod protocols**: Say "got it" when a message lands. Say "ready" when done and waiting for input. Stop generating option menus — just do the needful.

---

## Current State (last updated: session 3)

### Working
- PLGparse, PLGrule, Alternative, Element, PLGitem compiling and running
- Buffer-based input with divertInput/revertInput
- matchSet, matchLit, matchRuleRef working correctly
- applyRepetition working
- Safe DoubleLinkList iteration established
- addTest() implemented in PLGparse
- PLGset moved to support repo
- Guards implemented — kRuleRef recursion, cycle protection, min=0 walk-through, noGuard propagation, zero-advance-stop
- .next chain merges complete — BodyN, AlternativeBlockN etc. merged into multi-alt rules
- Alternative.match optional element handling fixed
- Two-table process() pattern implemented
- RuleplgNow fires — fresh rules table populated with Max + Integer from Testing.g parse ✅
- Testing.g parses end-to-end — 90/91 bytes consumed ✅
- GitHub repos: plg, incant, support, tawk all public ✅
- Xcode project rebuilt via xcodegen/project.yml — reproducible ✅
- CLAUDE.md in all four repos ✅
- Support static library (libsupport.a) linked into PLG ✅
- setRules() shrunk 48% using addTest() ✅

### In Progress
- runDeferred / full callback chain (ElementplgAct etc.) — next architectural layer
- Incant conceptual bible / wiki

### Next Steps
See TODO.md

### Known Working Tests
```
input: ",678"
rule:  Max
result: matched 4 chars: ,678

input: Tests/Testing.g
result: parsed 90 chars; RuleplgNow fires for Max and Integer
```

---

## Open Design Questions

These need thinking through before coding. Not implementation tasks — design conversations.

1. **runDeferred in new PLG** — PLGitem.runDeferred() doesn't exist yet. Design: PLGitem accumulates deferred (callback, item) pairs during match. runDeferred() walks the list and fires each. Callbacks receive new-model PLGitem and build Element/Alternative objects. ElementplgAct and the full chain must be rewritten from scratch against the new model — the old code used PLGtester and cannot be ported as-is.

2. **Label/rule distinction documentation** — isLabel/isRule flags, contextual semantics, the iron maiden rule (everything inside a label context is isLabel).

3. **Action/method boundary** — when does a C++ method become an incant action? How does the JIT change this boundary?

4. **Deferred actions** — canonical explanation of what defer means, when it fires, how it relates to PLG's runDeferred.

5. **Distributed GroupItem messaging** — design for cross-platform GroupItem communication. Bot project in InProcess/Bot has early experiments.

6. **Claude as GroupItem field type** — isCLAUDE data type alongside isSTRING, isNUMBER, isGROUP etc. Evaluates lazily — calls API, returns GroupItem. The AI is not a tool called from incant, it IS a field in incant.

---

## Glossary

- **HWF** — Hands Waving Furiously. Design mode with reduced constraint on anchor to reality. Valuable. Watch the cliff.
- **POP** — Proof Of Pudding. Prove it works before committing. Anthony's shorthand.
- **WSS** — We Shall See. The pragmatic response to any claim about what Xcode will or won't do.
- **Lootenant WTF** — debugging assistant. Finds culprits. Earns commendations. (American pronunciation.)
- **Bonfire** — retired code. Gone but not mourned.
- **Attic** — commented-out code. Not active, not deleted, findable.
- **Clay** — Claude at claude.ai. Design, reasoning, architecture.
- **Clod** — Claude Code. Execution, file ops, GitHub. Not disparaging. Just shorter.
- **do the needful** — Hinglish. Do what needs doing. Standing instruction to Clod.
- **eRocka** — modern eureka. Reserved for significant breakthroughs.
- **convention of one** — a convention held by exactly one person. Still a convention.
- **DSL** — Domain-Specific Language. A language designed for one specific purpose.
- **YAML** — Yet Another Markup Language. Human-friendly config format. Used for xcodegen project specs.
- **The cha cha** — the three-way workflow: Clay designs, Clod executes, Anthony architects. An elegant dance.
- **Tar baby** — a problem that gets stickier the more you engage with it. Avoid when possible.
- **extern "C" bridge function** — a free function using C linkage that bridges two types. Pattern for cross-type utilities that neither type should own.
- **Clod working states** — Nebulizing, Gallivanting, Zesting, Swirling, Fiddling, Moonwalking, Forging, Bebopping, Topsy turving, Embellishing, Churning, Pouncing, Reticulating, Baking, Puttering, Blanching, Catapulting, Percolating, Tempering, Stewing, Tinkering, Coalescing, Transfiguring, Cooking, Razzmatazzing, Frolicking, Kneading, Fiddle-faddling, Diddling, Cerebrating, Galloping, Forging sigils. All indicate productive activity of varying direction and certainty.
