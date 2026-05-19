# Session 9 working-level plan

*Drafted 2026-05-18. Companion to HWF Session 9. Authoritative for the brief sequence until superseded. Designed to be readable cold by fresh-Claude or Clod with only the bible + HWF + this doc.*

---

## What this document is

The bridge between Session 9's design (HWF) and individual Clod briefs. Session 9 settled what to do and why. This doc settles in what order, what each piece touches, and how victory gets measured.

Session 9's victory criterion: plg parses Testing.g and emits Testing.twk that passes muster — setRules() clean, set declarations correct, labeled captures landing in children, spliced .act content compiles through tok.

---

## Architectural calls settled during planning

Items left open in Session 9 and pinned during working-level planning:

**Action method signatures are uniform.** Every action in a new-shape .act takes `(PLGparse state, PLGitem iTEM)`. Mirrors incant's convention.

**Action method naming.** `RuleNameAct` for post-match (`currentRule.defer`), `RuleNameNow` for immediate (`currentRule.immediate`). Alternative-N gets digit infix: `RuleName2Act`. No `plg` suffix. Alphabetical sort clusters by rule name.

**Where .act content lands in generated output.** Above setRules. File shape: `[header includes] → [extern declarations for all action methods] → [.act splice] → [setRules()] → [main/runtime]`. Action methods findable without paging through setRules.

**Externs for action methods.** plg generates them at the top of the output, one per action method present in the included .act. Old plg did this and it worked well; new plg keeps the practice. Tok needs externs visible before any action method body that calls another action method.

**.act file structure.** Splice-verbatim with one exception — a declarations line at the top names which rules have actions in the file. plg reads that line (tiny parse), uses it to generate the externs block at top of output and to know which methods exist. Everything else in the .act is spliced verbatim with no scanning.

**Action wiring is opt-in by presence.** A rule with a matching-named action in .act gets auto-wired. A rule without doesn't, no error, no warning. Author guarantees an action exists for a rule by writing at least a stub conforming to the action signature. Adding actions later: edit-.act, rerun-plg, cuffs and collars match.

**Methods in .act that don't match a rule-action name** are treated as deferred-by-fiat — utility methods callable from other actions, externed at the top with the rest, otherwise unwired.

**Buffer-based splice accumulator.** plg uses a Buffer field on PLGparse to accumulate .act/.rtn splice content. Buffers proved their value as parser-mess infrastructure in incant (flexibility + inspectability).

---

## Brief sequence

### Brief 1 — SetVariableplgNow 'S' branch fix

**File**: `PLG.twk` (the body of `SetVariableplgNow`).

**Problem**: the 'S' (Set keyword) branch passes the set's *name* to PLGset's constructor as if it were the character spec. Spec ends up containing the characters of the name string. The `set` labeled child of the parse tree (containing the StringSet capture) is never read.

**Fix**: read `iTEM.children["set"]` (optional, marked `?` in the grammar). When present, walk into its `text` grandchild for the inline-spec form. Use `state.getSet(name, specs)` — the existing factory that does lookup-create-register correctly — rather than constructing PLGset directly.

**Verification**: run plg against Testing.g. setTable diagnostic output (already in process()) should show one entry — `excludeSet` with spec `A-Za-z:.|`.

*Status*: ✅ **landed and runtime verified** (commit 19f9f68 for the fix, paired with the Testing.g fixture fix in af39a11). Testing.twk's setRules() contains `excludeSet = getSet("excludeSet", ".:ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz|");` — the expanded form of `A-Za-z:.|` (PLGset's constructor resolves ranges into literal characters at registration time; same set, different surface).

**Scope**: 'S' branch only. Other branches (V/R/K/C/D) remain as-is — out of scope for unblocking Testing.g.

**Dependencies**: none. First brief.

---

### Brief 1.5 — Testing.g fixture missing terminating semicolon

**File**: `Tests/Testing.g`.

**Problem**: surfaced during Brief 1 execution. The initial trace appeared to show optional-element handling failing on Header. Closer trace inspection (Clod's second pass) revealed optional-element handling was working correctly — Header returned itemEmpty and the alt continued. The actual failure was downstream: Body matched SetVariable's Set alternative, captured `name=excludeSet` and `set=[A-Za-z:.|]`, then failed because the meta-grammar's SetVariable.alt4 requires a terminating `;` and Testing.g's Set declaration was missing it.

**Fix**: add `;` to the end of `Set excludeSet [A-Za-z:.|]` in Testing.g. One character.

**Verification**: run plg against Testing.g. Expected: parse advances to SetVariable.alt4 success, SetVariableplgNow fires, setTable contains `excludeSet`. Testing.twk's setRules() contains the getSet call.

*Status*: ✅ **closed** (commit af39a11). Testing.g + `;` added; Testing.g and Testing.act force-added past Tests/ gitignore to make Session 9 spec-referenced fixtures available to fresh-Clod.

**Lesson surfaced**: first-pass parse traces can mislead about which level failed. The shape "inner rule failed, then outer alt failed" doesn't establish causation — the outer alt may be failing for its own downstream reason. Consume/cursor data on each element resolves the ambiguity; without it, the hypothesis can chase a non-bug. Fold into bible Lessons at Session 9 graduation.

**Dependencies**: none. Blocked runtime verification of Brief 1; now closed.

---

### Brief 2 — PLGitem getLabel(name)

**File**: `PLGitem.twk`.

**Addition**: a `getLabel(String name)` accessor method. Body is `return children[name];` — a hash lookup returning null on absent key.

**Verification**: compile cleanly. No behavior change to existing parse paths.

*Status*: ✅ **landed and verified** (commit 9fc46aa). Build clean; Testing.g still parses 500 chars, Testing.twk regen unchanged, no regression.

**Implementation note**: tok auto-casts on typed assignment LHS but not on direct return. The body is written as `PLGitem result = children[name]; return result;` rather than `return children[name];` to get the cast right. Matches the existing PLG.C idiom of `(PLGitem*)iTEM->children->get("...")` at call sites.

**Dependencies**: none. Lands when ready, consumed by Briefs 5 and 7.

---

### Brief 3 — IncludeplgNow extension routing

**Files**: `PLG.twk` (the body of `IncludeplgNow`) and `PLGparse.twk` (additions to support the splice mechanism).

**Problem**: current IncludeplgNow treats every included file as plg grammar — Body.match in a loop. Correct for `.g` files; wrong for `.rtn` and `.act` files which should splice verbatim into the eventual generated output.

**Mechanism, end to end:**

1. PLGparse gains a Buffer field for accumulated splice content, plus a list-or-table for tracking action method names harvested from .act declarations lines.
2. IncludeplgNow's extension dispatch routes `.g` to existing path; `.rtn` to splice-verbatim accumulator (no scanning); `.act` to splice-verbatim accumulator *plus* parse the tiny declarations line at the top to harvest action method names.
3. Declarations line recognized: `actions RuleName1, RuleName2, ...;` as the first non-blank line of the .act file. Everything after goes to the accumulator verbatim.
4. generateRules() — extended in Brief 4 — emits externs and splices the accumulator at the right point in output.

**Scope**: routing + accumulator wiring + declarations-line parsing. Output-side emission lands in Brief 4.

**Verification**: run plg against a minimal .g that includes a small .rtn (just a comment) and an .act with a single-action declarations line. Diagnostic output confirms accumulator received splice content and action name list. No-progress detection prevents infinite loops on malformed includes.

**Dependencies**: none.

---

### Brief 4 — Generation pipeline reshape

**File**: `PLGparse.twk` (generateRules() body). *Brief 4's earlier text said PLG.twk — actually PLGparse.twk; corrected here.*

**Changes**:

1. Add externs block emission at top of generated output — one extern per action method name harvested by Brief 3's declarations parse. All externs use the uniform action signature `(PLGparse state, PLGitem iTEM)`; return type per convention (Now → int, Act → void).
2. Add splice point between externs and setRules() — flush the Brief 3 accumulator there.
3. Update file shape to: `[includes] [extern block] [.act splice] [setRules()] [main/runtime]`.

**Verification**: run plg against Testing.g (with Brief 1 and Brief 3 landed). Output Testing.twk contains externs block at top, then spliced action bodies from Testing.act, then setRules, then main. Tok compiles the generated Testing.twk cleanly.

*Status*: ✅ **landed and verified** (commit 29d1cb5). Both cases tested: empty actionNames (current Testing.act) → splice above setRules, no externs block; populated actionNames (temporary prepend of "actions Test, StringSet, StringSet2;") → externs block with correct return types above splice above setRules. Return-type convention working: Now → int, Act → void. Brief 1's setRules excludeSet line still intact.

**Dependencies**: Brief 3.

---

### Brief 5 — Four hook sites in PLGrule::match

**File**: `PLGrule.twk` (the match() method).

**Changes**: add four tok-recognizable goto labels at the named positions:

- `parseAttempt` at entry, before alternatives loop.
- `matchSucceeded` inside alternatives loop when an Alternative returns non-null (alt index in scope here).
- `matched` after successful match with label and atRuleMark info in scope.
- `parseReturn` at final return with `success` boolean in scope.

Labels themselves are inert — no behavior change. They exist so plgDirectives can address them via tok directives.

**Verification**: compile cleanly. No runtime difference from before the change.

*Status*: ✅ **landed and verified** (commit dc5118f). Two findings:

- **The `debug` field already exists** on PLGrule (PLGrule.twk:25), already mirrored in PLGrevision's `external PLGrule` block, already consumed by inline match() debug couts via `debugRulePLG || debug`. Brief 5 option (A) added nothing new; the existing per-rule toggle is `rule.debug`, not `rule.debugged`. Brief 6 consumes `rule.debug`. Mirror principle absorbs the name divergence — incant's `debugged` and plg's `debug` are the same pattern with different names.
- **parseReturn fires fail-only.** match() has two return paths — `return result` on success (after `matched:`) and `return null` on all-alts-failed (where `parseReturn:` was placed). Brief 6's directive design path α: accept the split, `matched` does success-branch debug, `parseReturn` does fail-branch debug. No match() refactor needed.

**Dependencies**: none.

---

### Brief 6 — plgDirectives entries for the four hook sites

**File**: `plgDirectives`.

**Tony's seat**, not Clod's. Format mirrors incant's groupDirectives shape — directive entries with `cout` lines, `indent()` calls, `debugIndent` counter. Each of the four hook sites gets a directive entry that fires when `debugRulePLG || rule.debug`. Path α split per Brief 5's finding: `matched` does success-branch debug, `parseReturn` does fail-branch debug. plg-specific addition: alt index logged at the `matchSucceeded` site.

**Verification**: regen via tok with plgDirectives present produces a PLGrule.C with debug output mirroring incant's format. Without plgDirectives, clean PLGrule.C without debug. Run plg against Testing.g with debug on; output reads tree-style.

*Status*: 🪑 **Tony's seat, after-hours.** Track A closes when this lands.

**Dependencies**: Brief 5.

---

### Brief 7 — First new-shape Testing.act

**File**: `Testing.act` (rewrite).

**Content** in new-shape per the architectural calls:

- `actions Test, StringSet, StringSet2;` declarations line at top.
- `external PLGitem { initializer getLabel; }` block.
- `TestAct` method using label-locals shorthand (`PLGitem name:, set:;` at body top), implementing the same logic as Brief 1's SetVariableplgNow fix.
- `StringSetAct`, `StringSet2Act` methods migrated from the old plg .act content with label-locals shorthand.

**Verification**: plg parses Testing.g, encounters `include(Testing.act)`, splices via Brief 3 mechanism, generates Testing.twk via Brief 4 pipeline. Resulting Testing.twk compiles through tok. Run compiled binary against a tiny test input; Test rule's TestAct fires and updates setTable as expected.

**Dependencies**: Briefs 2, 3, 4.

---

## Victory test for Session 9

Run plg against Testing.g. Expected outcomes:

- setTable contains `excludeSet` with spec `A-Za-z:.|` — Brief 1.
- Output Testing.twk has externs block at top — Brief 4.
- Output Testing.twk has spliced Testing.act content above setRules — Briefs 3 and 4.
- setRules() block clean, set declarations correct — Brief 1.
- Labeled captures land in `iTEM.children["name"]`, `iTEM.children["set"]` — already working per Session 9 bible state.
- Testing.twk compiles through tok cleanly — Briefs 3, 4, 7.
- Running compiled binary, debug output is tree-formatted per incant pattern — Briefs 5, 6.

All seven boxes ticked = Session 9 graduates.

---

## Tracking items (not blocking briefs)

**plg modifier coverage audit.** Plg's `modify` implementation status vs incant's modifier table is unverified. Specific divergence noted: `&` is `isPointer` in incant, `noSkip` in plg (Tony ran out of characters during plg's design). Worth a side-by-side audit at some point — bible Housekeeping entry candidate.

**Action-naming auto-wiring mechanism.** The incant pattern "rule definition scans for matching action method, auto-wires from finding it" is the model adopted for plg. Implementation detail: auto-wiring happens during parse of .g when a rule is being defined; depends on action externs being available because the action method bodies haven't been spliced yet at parse time. The externs block emission in Brief 4 needs to feed something that PLGrule definition can consult during the parse pass. Worth confirming the precise mechanism when Brief 4 is being drafted.

**Other SetVariable branches** (Variable, Rules, KeyWord, Condition, Debug) — partially implemented at best. Not exercised by Testing.g. Revisit when broader .g files (Tawk.g) get tested.

**Tests/ gitignore policy.** Force-added Testing.g and Testing.act as Session 9 spec-referenced fixtures (commit af39a11). Broader audit of Tests/ contents — what else should be tracked vs stay ignored — deferred to Tony's call. Standalone work item.

---

## Order of execution

Strict dependency order:

1. Brief 1 — SetVariableplgNow ✅ landed and runtime verified (commits 19f9f68 + af39a11)
2. Brief 1.5 — Testing.g fixture fix ✅ closed (commit af39a11)
3. Brief 2 — PLGitem getLabel ✅ landed (commit 9fc46aa) + polish (commit 0e4d04a)
4. Brief 3 — IncludeplgNow routing ✅ landed (commits d5cbda6 + e026ade + b9264ce + 82b9bf8)
5. Brief 4 — Generation pipeline reshape ✅ landed (commit 29d1cb5) + polish 1 (commit 233794b) + polish 2 (commit 454fdc3)
6. Brief 5 — Hook sites in PLGrule::match ✅ landed (commit dc5118f)
7. Brief 6 — plgDirectives entries 🪑 **Tony's seat, after-hours**
8. Brief 7 — First new-shape Testing.act ✅ landed (commit c6e5314, verification closed alongside Brief 8)
9. Brief 8 — Class wrapper emission ✅ landed end-to-end (commits 6f45f5c + 06083f0)

Briefs 2 and 5 are independent and parallelizable. Briefs 3-4-7 are the Track B chain. Briefs 5-6 are the Track A pair. Recommended cadence: serial execution, surfacing on-disk state at each brief landing to inform the next draft.
