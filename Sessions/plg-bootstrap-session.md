# PLG Bootstrap Session — 2026-05-04

## Part 1 — Distilled Notes

The session-level record of what shipped, what blocked, and the decisions
behind both. For ongoing project state, see `TODO.md` and `projectBible.md`
in the repo. This file captures *how* we got here, in case the reasoning
is needed later.

### Goals at session start

Sequenced from the morning plan:

1. Check repo state, update TODO.md to reflect reality
2. Rename `plg_rules.g → plgRules.g` (already done as it turned out)
3. Reorganize Tests directories (Tokf/Tests, Parse/Tests) with symlinks to source
4. Diagnose missing `ActionOption` definition
5. Self-host check — feed generated `plg.twk` back through tawk, compile, verify
6. Grammar reorganization — split `plg.g` (structure), `action.g` (actions), `plgRules.g` (shared)

### What shipped

**Tests reorganization** ✅
- `Tokf/Tests/` — kept the `Links/` symlinks (25 of them, all to Tokf source files); demoted up; removed `Externals/`, `Odds/`, `Other/`, `Programs/`, plus loose-file cruft. Dropped two dangling symlinks (`Externals` and `includes`).
- `Parse/Tests/` — kept `Testing.g` and `Testing.twk`; deleted 19 stale regular files; added 14 symlinks to `../{action.g, parts.g, plg.act, plg.g, plg.rtn, plg.twk, PLGitem.twk, PLGlabel.twk, PLGparse.twk, PLGrgx.twk, PLGrule.twk, plgRules.g, PLGtester.twk, Splitter.twk}`.

**Generator switched to `addTest()` form** ✅ — commit `bad6ebc`
- The generator was emitting raw `Element *elem = new Element(); elem->minimum=...; ...` blocks (~6-9 lines each). PLGparse already had `addTest(int kind, String data, String label, int min, int max, String skipSet)` designed specifically to replace this verbose form — the docstring on `addTest` literally calls it the replacement. The hand-written `setRules()` already used it throughout.
- Rewrote `Element.generate()` and `Alternative.generate()`. Added `setBanged()` and `setIgnored()` helpers to PLGparse alongside the existing `setNoSkip()`. Added `name`/`specs`/`setBuffer` field declarations to `external PLGset` in `PLGrevision` (TAWK couldn't resolve `setRef.specs` without the explicit body).
- **Result: generated `plg.twk` shrank 76% — 1432 lines → 336 lines, 26KB → 10KB.**

**`//` separator removal** ✅ — same commit
- Discovered mid-session: `PLGrule.generate()` emits `//\ncurrentRule = getRule("X");` between every rule. CLAUDE.md's TAWK quirks list says "Empty `//` comment lines reset field resolution context" — and that's exactly what bit us when we tried the self-host paste-in: `currentRule = getRule(...)` lost its `parser->` prefix because `use parser` context had been reset. Hand-written `setRules()` had **0** `//` separators (it uses blank lines). Fixed the generator to do the same.

**`action.g` excluded from include chain** ✅ (Option A)
- Diagnosed `ActionOption` situation: not a missing definition. `action.g` defines `ActionOption` correctly with two alternatives, but the parser silently drops three rules (`ActionEnd`, `ActionBody`, `ActionOption`'s body) because of unparseable syntax (modifier-on-terminal forms `'\n'&` `[.|;]&` `';'!&`, plus a `{` modifier of unclear meaning, plus self-recursion).
- The top-of-file banged action callbacks (`ActionOption!`, `ActionRule!` with TAWK bodies) are the **unimplemented Action-blocks feature** per TODO.md. Until that lands, `action.g` cannot be ingested.
- **Critical sub-finding:** the broken `action.g` include was corrupting parse state, silently dropping ~13 rules from `plg.g`'s main grammar body. The "23 rules" milestone in commit `2df2c23` was a mirage — with `action.g` cleanly excluded, **all 36 rules** parse end-to-end.
- Edits: commented out `include(action.g)` in `plg.g` (using `/* */` because `//` isn't a recognized comment in `.g` files — first naïve `//` attempt silently parsed the include anyway); removed the `ActionRule` alternative from `Body`; deleted the `ActionRule` definition from `plgRules.g` (depended on `ActionOption`).

**`ActionOption2` is a red herring** ✅
- Only exists in `setRules.twk` (old hand-bootstrap, no longer compiled) and commented-out shims in `PLG.twk`. No current `.g` file references it. The split-rule pattern is gone.

**TODO.md updated** ✅ — moved 11 shipped Callback-Chain items to Done; reframed Immediate around current sprint.

### What didn't ship — self-host loop attempt

Attempted to close the loop by pasting the new generated `plg.twk` into `PLG.twk`'s `setRules()`, plus 8 manual action wirings as a "TEMPORARY BRIDGE" (since the generator can't yet emit immediate/defer attachments — those depend on the unimplemented Action-blocks feature).

Build succeeded. Run produced **0 rules** parsed.

**Root cause: `plg.g` does not self-describe.** The hand-written meta-grammar in PLG.twk uses a different shape than `plg.g`'s own rules describe:

| | OLD hand-written setRules() | Parsed from plg.g |
|---|---|---|
| `Start` | `Header* '%%' Body+ '%%'? Trailer?` | `Header? Body+` |
| `Header` | `Include` (the rule) | `'%%'` (literal) |
| `Include` | (handled bare `include` form) | only `'include(...)'` form |

`plg.g`'s actual content starts with `include PLGincludes` (no parens, no semicolon) before any `%%`. The OLD setRules was tuned to that file structure; the parsed Start/Header/Include rules from `plg.g` are too narrow to re-read it. Self-host can't close until either (X) `plg.g`'s rules are updated to self-describe accurately, or (Y) `plg.g`'s content is rewritten to conform to its own simpler grammar.

Reverted PLG.twk to pre-splice working state (`git checkout PLG.twk PLG.C PLG.h`). Build still passes; binary still produces 36 rules cleanly. Generator improvements survive.

### Decisions landed

- **Generator emits `addTest()` form**, not raw `Element *elem = new …`. Matches hand-written setRules style; 76% size reduction; format-gap closes for paste-in.
- **`//` separator suppressed** in generated rule headers. Use blank line instead. Same change, big effect — the previous form was silently breaking TAWK's `use parser` field resolution.
- **`action.g` excluded** from `plg.g`'s include chain until Action-blocks feature lands. Removed `ActionRule` ruleref from `Body` and its definition from `plgRules.g` to keep grammar clean.
- **Self-host loop deferred** — closing it requires action-callback wiring (Action-blocks dependency) AND `plg.g` self-description fix (grammar-design call). Not blocking other PLG work.

### Things to remember

- **`//` is not a comment marker in `.g` files.** Use `/* */`. This bit us: a commented-out include line was parsed as a real include and the `action.g` parse failed silently, eating ~13 rules.
- **TAWK `external X` with no body** means TAWK doesn't auto-populate X's fields. Add explicit field declarations between `{ }` for any fields the generator references. Bit us with `setRef.specs` resolution.
- **TAWK `//` lines** reset field-resolution context inside method bodies. Already documented in CLAUDE.md, now also discovered in generator output. Avoid emitting them.
- **Symbols introduced via TAWK methods must also be declared in PLGrevision.** `setBanged()`/`setIgnored()` were added to `PLGparse.twk` but didn't compile until added to PLGparse's external method list in `PLGrevision`.
- **The "X rules parsed" milestone** can be misleading when an upstream include fails partway through. `2df2c23` reported 23 rules; the real number with action.g excluded was 36. Always verify rule count against the source-grammar's expected count.

### Open work

- **Grammar-design call** — `plg.g` rules vs `plg.g` content. Two paths to make `plg.g` self-describe; needs design discussion.
- **Action blocks** — design approved per TODO.md, implementation pending. Required for self-host to close cleanly (so action wiring round-trips).
- **Cosmetic generator quirks** — `kLit` extracts a leading `\n` for `','` (e.g., `elem->litText = "\n,"`); `Set` spec name loses leading space when serialized. Both noted in TODO under "PLG known cosmetic issues."

### Commits

- `bad6ebc` — Generator rewrite complete: addTest format, 76% smaller output; self-host loop diagnosis
- `fb8ef20` — Mirror bible: incant bytecode design Qs decided
- `be7671d` — Mirror bible: no vregs in incant — they're just GroupItem fields

All pushed to `TAnthonyAllen/plg`.

---

## Part 2 — Conversation summary

Summary of the user/Clod exchanges. Compressed to one paragraph per
substantive turn — full context lives in chat history.

---

**Anthony:** Good morning. Today's plan: 6 items (state check, plg_rules
rename, Tests reorg, ActionOption fix, self-host check, grammar reorg).
Read TODO.md and CLAUDE.md before starting. Report current state first.

**Clod:** Repo state clean. plg_rules.g rename already done. Tests dirs
need work. ActionOption is referenced but the rule is in `action.g`
(banged callback at top + proper rule definition) and only partially
parsed by current PLG. ActionOption2 only exists in old hand-bootstrap
and commented-out shims. Recommended order: (1) TODO refresh, (2) Tests
reorg, (3) ActionOption diagnosis, (4) self-host.

**Anthony:** Approved order. Update TODO.md, then Tests reorg, then
ActionOption diagnosis (report before fixing).

**Clod:** TODO.md updated — moved 11 shipped Callback-Chain items to
Done, reframed Immediate. Tokf/Tests reorg: dropped Externals/Odds/
Other/Programs/cruft, demoted Links/ contents up, dropped two dangling
symlinks (Externals + includes). Parse/Tests reorg: kept Testing.g+twk,
deleted 19 stale files, added 14 symlinks to Parse/ source.

**Clod:** ActionOption diagnosis: not missing as a definition. action.g
defines it correctly. But the parser silently drops three rules
(ActionEnd, ActionBody, ActionOption body) because their bodies use
syntax incant can't ingest yet — modifier-on-terminal forms, `{`
modifier of unclear meaning, self-recursion. The banged callbacks at
top of action.g (`ActionOption!`, `ActionRule!` with TAWK bodies) are
the unimplemented Action-blocks feature. Three options: (A) exclude
action.g until Action-blocks land, (B) patch the rules to use only
currently-supported syntax (lossy), (C) fix the parser. Recommended A.

**Anthony:** Option A approved.

**Clod:** Three edits in `Parse/`: commented out `include(action.g)` in
plg.g, removed ActionRule alternative from Body, removed ActionRule
definition from plgRules.g. No Action* refs remain.

**Clod:** Self-host check — regenerated plg.twk on the modified plg.g.
First attempt produced 13 rules (regression from 23). Found two bugs in
my edit: (1) `//` is not a recognized comment marker in `.g` files
(only `/* */` is); the include line was being parsed as a real include
and `action.g`'s partial-parse failure was eating downstream rules.
Fixed with `/* */` form. Second attempt: **36 rules**, all of plg.g's
main grammar body now parses cleanly. The "23 rules end-to-end"
milestone was a mirage — broken action.g include was costing 13 rules
to parse-state corruption.

**Clod:** To complete the self-host loop, the generator needs to
produce setRules() that can be pasted into PLG.twk and recompiled.
Investigation: the generator emits raw `Element *elem = new Element();
elem->minimum=...;` blocks (~6-9 lines each). PLG.twk's existing
hand-written setRules uses `addTest(...)` shorthand (~1 line each) —
that helper was specifically designed to replace the verbose form.
Format gap is the blocker. Recommended: switch generator to addTest
emission. Closes format gap, ~76% size reduction.

**Anthony:** Option B (paste + recompile) approved. Switch generator
first, then attempt paste-and-recompile.

**Clod:** Generator rewrite. Element.generate now emits
`addTest(K, "data", "label", min, max, "skipSet");` for kLit/kChr/kSet/
kAny/kEof/kRuleRef; falls back to raw form for kKeyTable/kCondition.
Alternative.generate emits `currentAlt = new Alternative(); ... ;
currentRule.alternatives += currentAlt;`. Added setBanged/setIgnored
helpers to PLGparse alongside setNoSkip. First build failed: TAWK
couldn't resolve `setRef.specs` because PLGrevision had `external
PLGset` with no body. Added explicit field declarations. Build
succeeded. New generated plg.twk: 336 lines, 76% smaller, all 36
rules preserved.

**Clod:** Spliced new plg.twk content into PLG.twk's setRules() body.
Build failed: `currentRule = getRule(...)` was emitted without `parser->`
prefix. Diagnosis: the `//` separator lines between rules were
resetting TAWK's field-resolution context (CLAUDE.md TAWK quirks
section warned about this). Fixed PLGrule.generate to use bare newline.
Stripped `//` from spliced PLG.twk. Build succeeded after second issue:
setBanged/setIgnored were not in PLGrevision's PLGparse extern method
list — added them.

**Clod:** Self-host run on plg.g produced **0 rules**. Investigation:
the new setRules() doesn't wire any action callbacks. The OLD setRules
had 8 active callback wirings (ForwardDecl→ForwardDeclplgNow, Alt→
AltplgAct, Element→ElementplgAct, ElementType→ElementTypeplgAct,
Include→IncludeplgNow, Rule→RuleplgNow, RuleOption→RuleOptionplgAct,
RuleOptions→RuleOptionsplgAct). Without these the parser doesn't fire
the callbacks that populate fresh rules. Three paths: (A) restore PLG
to pre-splice, (B) hand-patch the 8 wirings as a TEMPORARY BRIDGE,
(C) leave broken. Recommended B.

**Anthony:** Option B. Add the 8 wirings, document them clearly, run
self-host, report.

**Clod:** Added 8 wirings via sed (Edit had race-condition issue with
unread file modifications). Built, ran — still **0 rules**. Different
root cause: plg.g's defined Start/Header/Include rules don't match
plg.g's actual file structure. Parsed Start = `Header? Body+` where
Header=`%%`, but plg.g's content starts with `include PLGincludes`
(no parens) before any `%%`. The OLD hand-written meta-grammar had
Start = `Header* '%%' Body+ '%%'? Trailer?` with Header = Include
(handling the bare-include preamble). Self-host can't close until
either plg.g rules update or plg.g content rewrites. Reverted PLG.twk
via git checkout — generator improvements survive.

**Anthony:** Stop self-host. Commit working changes.

**Clod:** Committed `bad6ebc` — generator rewrite complete (addTest
format, 76% smaller, self-host diagnosis). 11 files, +280/-110 lines.

---

## How to use this file

If you pick PLG bootstrap back up:
1. Read this file first, then check `TODO.md` for current state.
2. The grammar-self-description issue is the next design call. Either
   path (X update plg.g rules, Y update plg.g content) is doable; need
   to pick.
3. Action-blocks design is approved per TODO; implementation will
   close the action-wiring round-trip. Both gates need to clear before
   self-host loop completes cleanly.
