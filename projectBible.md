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

## Language Classifications

| Project | Type | Key Properties |
|---------|------|----------------|
| PLG | DSL / pattern matcher | Declarative, stateless, fast, embeddable |
| TAWK | Transpiler | Source-to-source, metaprogramming, syntactic sugar for C++ |
| Incant | General purpose | Reflexive, homoiconic, stack-aware, generates to C++ |

**Reflexive** — the language can examine and describe itself.  
**Homoiconic** — code and data have the same structure. A GroupItem field IS the rule that describes it.  
**Transpiler** — compiles to source code (C++) rather than machine code.  
**Declarative** — describes what to match, not how to match it.

---

## Architecture

### PLG Core Classes (new architecture)
- **PLGparse** — base parser. Buffer-based input, cursor/eof, rules/setTable hashes, divertInput/revertInput stack
- **PLGrule** — one grammar rule. Has alternatives (DoubleLinkList), guardSet, action callbacks (defer/immediate/fail)
- **Alternative** — one option within a rule. Has elements (DoubleLinkList), guardSet
- **Element** — one match unit. Kind (kLit/kChr/kSet/kAny/kEof/kRuleRef/kKeyTable/kCondition/kVariable/kUpTo/kBalanced), min/max, setRef/litText/ruleRef etc.
- **PLGitem** — match result. Buffer reference + offset (not raw pointer), itemLength, toString()
- **PLG** — extends PLGparse. Contains setRules() and all plg-specific grammar/actions

### Key Design Decisions
1. **Buffer-based input** — all input lives in Buffer objects. divertInput pushes current buffer, installs new one. No raw pointer arithmetic. No writable string problems.
2. **Safe iteration** — use `for link = list.first; link; link = link->next` pattern everywhere. Never use the default `next()` iterator in recursive contexts — it uses shared `entry` state that gets clobbered.
3. **Composition over inheritance** — PLG contains a PLGparse field rather than extending it (workaround for C++ incomplete type issues with tok-generated headers)
4. **toString() only** — PLGitem no longer null-terminates in place. toString() returns a malloc'd copy. string()/unString() retained for backward compatibility with support code.
5. **GC friendly** — tape allocator retired. New/delete or GC handles memory.
6. **Single exit point** — match methods use result variable + goto end pattern for debuggability.

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

---

## The Bootstrapping Problem

PLG is supposed to generate setRules() but needs setRules() to parse plg.g to generate setRules().  
Current solution: old PLG binary generates plg.twk → Python script translates to new format → hand-paste into PLG.twk.  
Script: `python3 translate_setrules.py plg.twk new_setRules.twk`  
Long term: new PLG parses plg.g and generates its own setRules(). Self-hosting.

---

## TAWK Known Issues (autopsy table)

1. **Empty comment lines** (`//`) reset field resolution context — causes `new` to fail type inference and field references to be lost. Fix: context tracking should be comment-blind.
2. **Implicit field resolution** is complex machinery. Incant solves this elegantly with `lastREF`/`@context`. TAWK refactor: replace implicit resolution with explicit `@field` context markers.
3. **No include guards** generated in .h files. Must add manually for inherited classes.
4. **`new` type inference** inconsistent — `alt = new` sometimes fails to resolve to `new Alternative()`. Workaround: explicit `alt = new Alternative()`.
5. **Boolean field declaration** `tok generates individual bool fields; should use boolean lists.`
6. **No #include in .h files** — inherited classes need parent .h included but tok never generates includes in .h files. Must add manually after every re-tok.
7. **Unused field warning** — tok silently drops unused declared fields instead of warning. Fix: emit warning, keep field.
8. **`kSet` macro conflict** — `kSet(button)` is a test macro, not an assignable value. Kind assignment requires numeric literal. Fix: separate test macros from enum values.

---

## Incant / PLG Convergence

- `match()` — atomic recognition. Did this pattern occur here?
- `parse()` — semantic processing. What does it mean that it occurred?
- In PLG these are separate. In Incant they collapse because the data structure IS the grammar IS the result.
- **The bridge**: PLGitem is a lightweight match result. GroupItem wraps PLGitem when semantics are needed. Promotion is lazy — only when incant needs to reason about the result.
- **Input diversion**: same concept in both. PLG: push/pop Buffer. Incant: push/pop GroupItem containing buffer. Same contract, appropriate implementation for each.
- **The north star**: PLG written in Incant, parsing itself. Not today. But the architecture points there.

---

## generateRules() / setRules() relationship

`generateRules()` is PLG's primitive bytecode compiler — it promotes interpreted grammar structures to compiled C++.  
Incant's bytecode work is the same idea at higher abstraction.  
`parse()` orchestrates. `match()` is atomic. `generateRules()` compiles.

---

## Working Relationship

**Anthony** — architect, domain expert, final authority on design decisions.  
**Claude (here)** — design, reasoning, architecture, HWF navigation, pair programming.  
**Clod (Claude Code)** — execution, file operations, GitHub maintenance, running code.  

**Standing permissions**: Clod may change any code in source directories without asking. Ask before GitHub pushes.  
**HWF protocol**: When design goes near a cliff, flag it. The face plants teach as much as the successes.  
**Convention of one**: tawk not tok in source (Tony's awk). Conventions are conventions even with one member.

---

## Current State (last updated: session 2)

### Working
- PLGparse, PLGrule, Alternative, Element, PLGitem compiling and running
- Buffer-based input
- matchSet, matchLit, matchRuleRef working
- applyRepetition working
- Safe DoubleLinkList iteration pattern established
- `matched 4 chars: ,678` — Max rule matching correctly

### In Progress  
- PLG class (extends PLGparse via composition) — compiling, setRules() loaded
- Feeding testing.g through parser instead of hardcoded setRules()
- Bootstrap sequence

### Next Steps
1. Get PLG parsing testing.g end to end
2. Expand to plg.g — bootstrap new PLG
3. GitHub repos for plg/tawk/incant
4. Project bible into each repo
5. TAWK on the autopsy table (after PLG bootstraps)
6. Incant JIT (longer term)

### Known Working Test
```
input: ",678"
rule:  Max
result: matched 4 chars: ,678
```

---

## Glossary

- **HWF** — Hands Waving Furiously. Design mode with reduced constraint on anchor to reality. Valuable. Watch the cliff.
- **Leftenant WTF** — debugging assistant. Finds culprits. Earns commendations.
- **Bonfire** — retired code. Gone but not mourned.
- **Attic** — commented-out code. Not active, not deleted, findable.
- **Clod** — Claude Code. Not disparaging. Just shorter.
- **do the needful** — Hinglish. Do what needs doing. Standing instruction to Clod.
- **eRocka** — modern eureka. Reserved for significant breakthroughs.
- **convention of one** — a convention held by exactly one person. Still a convention.
