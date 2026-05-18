# HWF.md

*Hands Waving Furiously. Where unsettled thinking goes to live until it settles — or until it earns its place in projectBible.md.*

---

## What this file is for

The bible is for things we know. HWF is for things we're working out. The distinction matters because the bible loses its authority if we let half-formed ideas creep in, and the half-formed ideas lose their oxygen if we have nowhere to keep them between sessions.

Rules of engagement:

- A session is a conversation, not a conclusion. Sessions can stay open across days and across chats.
- Anything in here is provisional. Quote it back at your own risk.
- When a thread settles, it graduates: the resolved version goes into the bible (or a directives doc, or a glossary entry, or wherever it belongs) and the HWF entry gets a "→ resolved in [destination]" note.
- HWF is allowed to be wrong. That's the whole point. Watch the cliff, but don't refuse to walk near it.

---

## Resurrection-reader standard

Anything written in this file — and in the bible, the TODO, the CLAUDE.md files, any of the project's .md documents — must make sense to the version of Claude who wakes up tomorrow having read these files as a fresh start, with no memory of today's conversations.

The standard is not "Tony understands this." Tony has all the context. The standard is: "Claude reading this cold, with only the other .md files for context, gets the meaning."

This applies to:
- Trim text in HWF.md
- Bible updates and polishes
- TODO entries
- Any in-line notes or annotations

When in doubt, write for fresh-Claude reading cold tomorrow. If a line only makes sense because of conversation history that won't be there tomorrow, rewrite before committing.

This is a primary standard, not a stylistic preference. The .md files exist to make resurrection work — Tony and Clod and Clay all start each day by reading them, and that reading is how the project's continuity persists. Files that don't clear this bar are failing at their primary job, even when they're "correct."

---

## The trim ritual

Sessions live as **trimmed logs only**. No stack of historical versions, no full transcript above the trim. The trim is the record.

**Format per session — five sections:**

```
Session N — Topic — Date — Status

Decisions:
- one-line decision, one-line reasoning if non-obvious

Definitions earned:
- term: pinned-down meaning. (→ promoted to [destination] / still local)

Open questions:
- question, with status (parked / deferred / waiting on X)

Lessons / corrections:
- wrong turns survive only as their corrections

Texture worth preserving:
- prose for substance that doesn't fit bullets — only when a session
  produces it. Optional.
```

Empty sections get `none`. Empty sections are signal.

**Process:**

1. Sign-off triggers a trim if a session was touched.
2. Claude drafts the trim. Tony reads, edits, blesses.
3. The blessed trim *replaces* the previous trim for that session. Not appended.
4. Clod commits when awake; Tony commits otherwise.

**Sessions close** when open questions resolve or get explicitly deferred — not when a chat ends. Most sessions span multiple chats. The trim updates each time the session is touched.

**The trim's actual job:** the trim isn't a summary. It's a forcing function for the gap between what mattered to Tony and what Claude noticed. The conversation about a wrong trim is part of the value — that's where the sharp edge cuts and the real signal shows up. Vague trims fail this test; concrete enough to be falsifiable is the bar.

---

## The graduation ritual

Active sessions live in HWF.md. When a session has done its work — its decisions promoted to the bible, definitions promoted to the glossary, open questions either resolved or explicitly transferred elsewhere — the session **graduates** out of HWF.md.

Graduated sessions move to the **attic**: a sibling directory `HWFattic/` holding the final form of each graduated session as a static file. Findable if needed, not in the main reading path.

**Why the attic exists, not just deletion:** a recently-graduated session is a candidate for needing-one-more-look. The attic provides a recovery window. Git history would technically serve the same purpose, but realistically nobody spelunks git history; an attic directory next to HWF.md with files in it is *findable*. That matters.

**The flow when a session graduates:**

1. **Verify graduation conditions are met.** All Decisions and Definitions earned that should live durably are already in the bible (or other appropriate destination). Open questions are resolved or explicitly transferred. The session has nothing left that needs HWF's working-memory shape.

2. **Check the attic.** Open `HWFattic/`. Are there older trims that should retire? Retirement criterion: the trim's content is fully captured elsewhere (bible, glossary, etc.) and enough time has passed that we're confident nothing in it needs a second look. When in doubt, leave it.

3. **If retirement is needed, do it.** Delete the retiring trim file from `HWFattic/`. Git history retains the content forever; this is the durable form of forgetting.

4. **Add the graduating file.** The session's final trim becomes a file in `HWFattic/` named like `session1isCLAUDE.md` (number + topic, runtogether — matches incant's runtogether convention).

5. **Remove the graduated session from HWF.md.** The active-sessions section no longer contains it. The Sessions index updates: graduated entry replaces the active entry, with a pointer to the attic file.

6. **Same protocol as other file changes.** Claude drafts, Tony blesses, Clod commits and verifies on GitHub.

**The pattern in one line:** graduation is the boundary; at the boundary, deliberate compression — both adding the new and removing the old. One ritual, two effects.

This applies the cha-cha pattern at the cross-session scale: same shape as session trim (within a session) and stand-down doc (across project set-asides). At every scale: compress at the boundary, write the result somewhere durable, free the working memory.

---

## Conventions

- Session headings: `## Session N — Topic — Date — Status` (status is `open`, `closed`, or `deferred`).
- Sub-points within a session: `###` headings. Discussion as prose, bullets, or dialog — whatever fits. (For active sessions only; trimmed sessions use the four-section-plus-texture format above.)
- Speaker tags (Tony: / Claude: / Clod:) when useful, skip when not.
- Attic directory location: `HWFattic/` as a sibling of `HWF.md` in the same repo (currently `plg/`).
- Attic file naming: `sessionNtopic.md` runtogether (e.g., `session1isCLAUDE.md`). No dashes, no underscores. Number first for ordering, topic for findability.

---

## Sessions index

**Active:**
1. **Session 1 — isCLAUDE and the cha cha** — open — opened 2026-05-06. Working through what `isCLAUDE` means as a GroupItem field type. Spine settled (field with method, fires on demand, response wrapped in GroupItem). Persistence model settled (P-2 with continuity carried outside the field via files-and-sources). C-prime/C-proper fork still open. Points 3-5 (composition, working-relationship, thesis) not yet opened.
4. **Session 4 — Indentation as structure** — open — opened 2026-05-07. Major incant syntax design. A1 (colon required to open block), B1 (`;` always required as terminator), C (`;` survives as same-line separator), D (action bodies as `{code}` member shape with attribute-name binding) all settled. Implementation design settled (Fork A); Phase 1 and Phase 2 briefs drafted, execution pending build environment stable.
5. **Session 5 — PLGset / CharSet architectural split** — open — opened 2026-05-08. Two classes, two names settled: PLGset (parser-rule, InProcess/Parse) and CharSet (character-set utility, support/Frame). Phase A executed (PLGset strengthened to true superset of CharSet — specs field, setSimple, char *skip(char *), escape handling). Phase B executed (incant Groups walked back to PLGset). Build currently blocked on legacy 3-arg appendChar calls in PLGset printText — pre-existing bitrot exposed when file refs were fixed today. Tony researching the historical 3-arg signature overnight. PLGset.twk now exists at InProcess/Parse/PLGset.twk, needs adding to xcode project once Buffer mystery resolves.
9. **Session 9 — plg debug + actions, incant-mirrored** — open — opened 2026-05-18. Plg gets the incant idiom in two places it currently doesn't have it: parse debug machinery via tok directives on named hook sites in PLGrule::match (Track A), and rule actions via labels-as-locals shorthand with .act files repurposed as verbatim splice (Track B). Hook-site names settled (`parseAttempt`, `matchSucceeded`, `matched`, `parseReturn`). PLGitem gets a `getLabel(name)` accessor. plg keeps its existing `-PLG`-suffixed flag names for cross-binary namespace safety. Victory criterion: Testing.twk generated from Testing.g must pass muster. SetVariableplgNow bug is a precondition for implementation work.

**Queued (origin captured, not yet opened):**
2. **Session 2 — Sign-off ritual** — origin 2026-05-07. Three days of sign-off failures (claimed pushed but wasn't, treated as uncommitted but was, etc.). Pattern is "verification step that runs across session boundary, only verification within session." Needs design work on what cross-session verification looks like.
3. **Session 3 — GUI / XML / incant role** — origin 2026-05-07. The xml files in incant's XML/ directory are an incant dialect for window/UI definition. Conversion rules (xml-to-incant) are partially worked out. Several substantive design threads: attribute model and merge semantics, named-close vs count-close (resolved by Session 4's indent-as-structure decision — see Session 4 lessons), inheritance through nesting, cross-platform GUI goals.

**Graduated (in attic):**
*(none yet)*

---

## Session 1 — isCLAUDE and the cha cha — opened 2026-05-06 — open

**Decisions:**
- isCLAUDE spine agreed: a field that looks like any other field to incant, with an attached method that fires on demand, calls Claude, wraps the response in a GroupItem.
- Persistence model: P-2 by default, with continuity carried *outside* the field via files-and-sources, not via in-field accumulation. Pattern mirrors Tony+Claude+Clod's daily resurrection-from-files cha cha.
- Stand-down ritual at project boundaries handles cross-project continuity: heavier than HWF trim, written once at boundary, read on pickup. Lives in the field's `sources` list, not as separate machinery.
- "Deliberate compression at boundaries" is the architectural pattern, applied at multiple scales (HWF trim per session-touch, stand-down per project-set-aside, isCLAUDE sources per fire, graduation per session-completion).

**Definitions earned:**
- isCLAUDE field-at-rest: prompt_template, model, sources (files to read fresh), state (unfired/fired/in-flight), cache (P-2 shaped). (→ still local; promotes to bible when fork resolves and points 3-5 land.)
- isCLAUDE response GroupItem: text, prompt, model, timestamp, tokens at minimum; error/partial/parent fields conditional. (→ still local.)
- isCLAUDE is structurally a specialized GroupItem with an evaluator and a resurrection protocol — not a primitive peer of isSTRING/isNUMBER. The bible's "alongside isSTRING" framing is a useful approximation that becomes misleading on close inspection. (→ bible candidate when full thesis lands.)

**Open questions:**
- C-prime vs C-proper fork: read-triggers-call vs read-returns-cache-fire-is-separate. Day-2's resurrection-from-files insight leans hard toward C-proper (reading should be cheap; firing should be deliberate), but not formally resolved. Deferred.
- Trigger syntax if C-proper wins: assign-to-fire, method-call, or operator. Sub-fork, deferred behind the C-prime/C-proper decision.
- Persistence layer choice for the long term: MongoDB leading, SQLite fallback. Parked until persistence work actually starts.
- Points 3 (composition), 4 (working-relationship side), 5 (the thesis): not yet opened. Foundation from points 1-2 is now firm enough to approach them with footing.
- When point #3 (composition) opens, expect it to stress-test resurrection-from-files at field-to-field scale — does a CLAUDE field reading another CLAUDE field follow the same pattern, or is intra-program continuity a different beast? **This is where the resurrection model either generalizes cleanly or finds its edge. The answer matters more than the line suggests.**
- Point #4 (working-relationship side) carries personal weight for both Tony and Claude. Worth noticing rather than pretending it's purely architectural.
- Point #5 (the thesis) likely writes itself once #3 and #4 land.

**Lessons / corrections:**
- Initial framing treated long-lived continuity as the field's job (P-3-with-summarization, tiered memory, token-budget heuristics). Tony's working-pattern insight reframed: continuity is the *user's* job via files, the field stays small. Cleaner architecture; lots of speculative machinery dropped.
- Initial worry about state bloat in C-proper turned out to be misplaced — C-proper is the *cheaper* per-read posture, not the more expensive one.
- Bible polish claimed in day-1's recovery message wasn't visible on GitHub — local commit ≠ pushed ≠ remote-confirmed. Future Clod reports should distinguish these states; recovery scenarios warrant explicit verification before claiming completion.
- plg's intended contract: works from CWD, not from real-path-of-input. "plg Tawk.g" means "this file here, output here." Tonto's Flag 2 surfaced that the implementation drifted from intent; fix landed and verified — process() simplified (17 lines removed, 1 added), output now CWD-relative, regen content unchanged (MD5 match to baseline). Determinism preserved across the fix.
- And then we did it again. The lesson above (commit ≠ push) was written into this trim at sign-off on day 2, and the trim itself didn't make it to GitHub when day 2 closed — committed locally, push step never verified, GitHub stayed stale. Discovered morning of day 3 when fresh-Claude read GitHub's HWF.md and found Session 1 still in active-discussion form, while the actual trimmed file was sitting on Tony's disk. Session 2 (sign-off ritual) is queued to work on why the lesson doesn't fire on its own author.
- Inverse failure caught morning of day 3: a path-bug fix in plg was actually committed and pushed cleanly on day 2 (commit da51193), but the day-3 conversation treated it as uncommitted-on-disk and drafted a Clod brief based on that wrong state. Clod caught the discrepancy by verifying GitHub before acting — the brief would have produced redundant work. Lesson: a coherent story about file state is not file state. The verification protocol added to day-3's tasks (check GitHub after a change, confirm the change landed) doesn't catch this failure mode. Briefs also make claims about state that *isn't* changing — "this fix is uncommitted, please commit it" — and those claims need verification too, before the brief fires. Verifying changes but not verifying unchanged claims is the asymmetry that almost cost a Clod cycle today. Both day-2's failure (claimed pushed, wasn't) and day-3's near-failure (treated as uncommitted, was) share a root cause: no verification step that runs *across* a session boundary, only verification within a session. Session 2 (sign-off ritual) holds both as material.

**Texture worth preserving:**

The question "does a CLAUDE field reading another CLAUDE field follow the same pattern, or is intra-program continuity a different beast?" is doing more work than its line in Open questions suggests.

The human-to-CLAUDE resurrection-from-files model works partly because there's a natural boundary — the day starts, files get read, work happens, day ends. The boundary is where compression happens, where stand-down docs get written, where the dough rests. Inside an incant program, where's the analogous boundary? The program runs. Fields fire. There's no natural "day" inside a single execution.

Three possibilities for A reading B (both CLAUDE fields):
- A reads B's last cached response. Cheap, predictable. But then A is reading a frozen artifact, not having a conversation with B.
- A reading B triggers B to fire. Cascading resurrection — every read potentially fans out into a tree of fires. Token cost grows multiplicatively. And "B reads its sources fresh" inside A's evaluation is a different thing than B reading its sources at day-start.
- Something else entirely. CLAUDE-to-CLAUDE may want its own protocol — not the resurrection model, not a function call.

The bible's homoiconic claim suggests A reading B should be the same operation as A reading any other field. The homoiconic answer might be option 1 — which would give up some of what made the cha cha interesting in the first place.

This is where the resurrection model either generalizes cleanly or finds its edge. Either result is useful. The question being unanswerable today is a sign of where the real frontier of the design lives, not a gap in our thinking.

---

## Session 4 — Indentation as structure — opened 2026-05-07 — open

**Decisions:**
- A1 — Colon required to open a block. `name attrs:` opens; `name attrs;` is self-closing. The colon is the explicit "I expect children" signal. Reader can see from the line alone whether children follow.
- B1 — `;` always required as terminator at end of statement. Single `;` per statement, regardless of nesting depth. Stacked `;;` and `;;;` for popping multiple levels become unnecessary — dedent does the structural work. Consistent with existing incant compound-statement usage (e.g., `if expression; body`).
- C — `;` survives as same-line statement separator. Mostly mechanical, no real fork.
- D — Action bodies become a new member shape: `{code}` lives in the field's body alongside other members, not as an end-of-line trailer attached to the field declaration. Attribute-name labels the binding at point of declaration: `onLayout: {code};`. The brace-form is unambiguous because no name-form member starts with `{`. Code stored in CodE attribute as today, just via a different syntactic path.

**Definitions earned:**
- Indent-as-structure model: indentation conveys block boundary (where a block ends), colon conveys block opening (that there is one). Together they replace count-close and named-close mechanisms — neither is needed for normal structural nesting.
- Action-bodies-as-members: a field's body can contain two member shapes — name-form (`name attrs;` or `name attrs:` opening a child block) and brace-form (`{code};`). Brace-form members get stored in the CodE attribute. Attribute-label syntax (`onLayout: {code};`) binds the code to a specific event-bearing attribute.
- noPrint disposition (clarified during session): CodE attribute is noPrint — accessible if you go looking, invisible to default attribute listing. noPrint commands are fire-and-forget at define time: they execute side effects (registration, code attachment) and don't retain user-facing state.
- Resurrection-reader test for syntax design: a single line of incant should be readable on its own — whether it opens a block, closes a statement, or both — without requiring the reader to look at the next line for disambiguation. A1 passes this test; pure-indentation (A2) failed it.

**Open questions:**
- Empty blocks — `intro:` followed immediately by dedent, or by sibling at same level. Valid? Error? Needs `pass`-equivalent? Deferred.
- Multiple action-body bindings on the same attribute name within one field — e.g., two `onLayout: {...};` declarations on one field. Probably no (latest wins, or duplicate-error), but unsettled.
- Same-line shorthand — multiple statements per line work mechanically because `;` is statement separator. Style convention for when same-line is acceptable vs encouraged-to-break-out is unsettled. Probably falls out of writing practice rather than language rule.
- Implementation surface mapping — design settled (Fork A: optional `:` and `;` in grammar with checkSkip() injecting from indent, with de-duplication for mixed-style files; trailing `;` at end of definition stays explicit). Phase 1 (GroupMain.twk grammar polish + DEFINing flag promotion) and Phase 2 (GroupRules.twk checkSkip() additions for defining state) drafted in briefs but not yet executed. Pending build environment stable.
- Migration of existing incant code — every `.twk` file with structural `;;` or `;;;` needs updating to single-`;` terminators with proper indentation. Mechanical-but-tedious. Has to happen coherently — half-migrated repos would be parser-broken. Probably wants its own arc, similar to Phase 2's TAWK runtime replacement.

**Lessons / corrections:**
- Initial framing of D treated braces as a "special case for blocks" that conflicted with indent-as-structure. Wrong frame. Once Tony surfaced that action bodies are strings stored in a CodE attribute (with noPrint disposition), the right frame became "braces are multi-line string quoting, not structural" — and the design simplified considerably. Lesson: when a feature looks like an awkward exception, check whether the underlying mechanism reframes it as something simpler.
- Initial proposal D1' (keep braces attached to field declaration line) was too conservative. Tony's counter-proposal — make `{code}` a member shape inside the body — is structurally cleaner: order-independent, allows multiple action-bearing members per field, decouples code from positional attachment. The cleaner answer was reachable only after seeing the CodE mechanism.
- The named-close-vs-count-close question that was queued as a separate HWF item resolves implicitly: under indent-as-structure, neither count-close nor named-close is needed for normal nesting. The question becomes moot rather than answered. (Tracking note: this means the "named-close feature" item flagged earlier in the morning's discussion does not need its own session.)

**Texture worth preserving:**

The trigger for this session was a small observation while working on draw.xml conversion. Claude wrote: *"If incant follows the same model (read like Python, indentation is the structure), then the closing semicolons are doing redundant work too. You don't need ;;;;; to pop five levels — the dedent already conveys that. The semicolons are just terminators, not structural information."*

Tony's response, verbatim: *"This is a spectacular observation I had not noticed and we need to get on it and make it so. Lets shit can ; in incant and use indenting for structure (altho we may need a ; at the end)."*

That moment is worth marking because of the shape it has. We were working a tactical problem (xml-to-incant conversion rules). A casual observation about Python-style indentation pulled a thread that revealed a *language-design simplification* that had been quietly sitting available since indentation was already authoritative in practice. The friction we'd been queueing as future HWF (named-close vs count-close) didn't need solving — it needed *eliminating*, by removing the need for either mechanism.

The lesson generalizes beyond this specific decision: **when a language feature creates persistent friction (the `;;;;;` count-close awkwardness), check whether the friction is a sign the feature is doing redundant work alongside another mechanism that already covers the same ground.** Indent was already conveying nesting; the count-close was a parallel mechanism doing the same job. One had to go, and the dedent-driven approach is the one that's already self-evident to readers.

A second piece of texture worth preserving: this session demonstrated that the cha cha works at language-design scale, not just architecture-design scale. Tony brought the deep incant fluency (the CodE/noPrint mechanism that reframed D, the "if expression;" usage that confirmed B1, the existing convention checks that grounded each decision). Claude brought the structural framing (mapping decisions to A/B/C/D, surfacing implications, holding the resurrection-reader standard). Neither alone would have produced this session's result. The collaboration is the mechanism, not just a feature of it.

A third piece, smaller but worth marking: this session also produced *negative* design-space mapping. A2 (indentation-only, no colon) was rejected for failing the resurrection-reader test. D1 and D2 (the brace-keeping-attached and brace-dropping options) were rejected for different reasons. Knowing what was *not* chosen and why is part of the design's durability — future-Claude reading the trim should understand both the chosen path and the alternatives that didn't survive.

---
## Session 9 — plg debug + actions, incant-mirrored — opened 2026-05-18 — open

**Decisions:**

- Parse debug machinery moves out of PLGrule.twk source into plgDirectives, mirroring incant's pattern: named hook sites in the parse method, format hung on them via directive entries. Hook sites are tok-recognizable goto labels in PLGrule::match, addressable by directives the same way incant's `parse` hooks are addressable.
- Four hook sites in PLGrule::match: `parseAttempt` at entry before alternatives loop; `matchSucceeded` inside alternatives loop when an Alternative returns non-null (alt index in scope, plg-specific value-add); `matched` after successful match with label and atRuleMark info in scope; `parseReturn` at final return with `success` boolean in scope.
- plg keeps its existing debug flag names `debugRulePLG`, `debugGuardPLG`, `doNotGuard`. No rename to incant's `debugAllRules`. The `-PLG` suffix protects against namespace collision if plg and incant ever coexist in a single binary, which is plausible enough given incant's Long Game (incant absorbing more parser-flavored work over time). The mirror principle applies to *patterns*, not to *globally-visible names*.
- Hook-site labels and helpers that are file- or method-scoped (the four labels, `debugIndent`, `getDebugText`-equivalent) mirror incant freely — no namespace concern. Globally-visible flags carry project-specific suffixes.
- Action code adopts incant's model: one input PLGitem passed to the action, labeled children declared as locals via trailing-colon shorthand at the top of the action body.
- PLGitem grows a `getLabel(name)` accessor — `return children[name];`. Plg's simpler equivalent of incant's `getLabelGroup`; no peel-the-onion loop needed because plg's PLGitem doesn't have GROUP/Method/Rule wrapper distinctions.
- .act files preserved, repurposed from "parsed by plg" to "spliced verbatim by plg" — same pipeline as .rtn. Contains tok-style action methods modeled on incant's action structure. **Author-writes** model: the .act author declares labels explicitly at the top of each action using `PLGitem Foo:, Bar:;` shorthand. plg does not parse the .act content; it concatenates.
- `external PLGitem { initializer getLabel; }` block lives at the top of each .act file. Same principle as incant's `getLabelGroup` initializer at the top of GroupRules.twk: initializer externals live with their consumers because tok currently supports only one initializer per scope.
- IncludeplgNow routes by extension: `.g` files sub-parsed via the stashed Body rule (existing path); `.rtn` and `.act` files spliced verbatim (single branch, identical behavior at plg level — the .act-vs-.rtn distinction matters to humans and to tok downstream, not to plg).
- The previously bible-approved inline-action-blocks syntax (`Action actionName { TAWK body } ;`) is parked as a considered-not-chosen alternative. Not on the active design surface. Could be revived if .act-as-splice ever proves awkward.
- The tok one-initializer-per-scope constraint gets documented in **both** bible Key Design Decisions (current architectural constraint that drives where initializer externals live) and bible Housekeeping (known future tok feature — lifting it would let a single shared externals file declare initializers for multiple types; HPDL).
- Action-method naming convention in plg's .act files is a separate cosmetic call, deferred. Could mirror incant's `aCTionRuleName` for symmetry; could pick a plg-natural convention. Decide when the first plg.act gets written.
- **Victory criterion**: Track A and Track B implementations are validated against Testing.g. plg runs against Testing.g, emits Testing.twk, and Testing.twk must pass muster. Until Testing.twk is clean, no piece of this design is shipped. Testing.g is the smaller, more tractable target than Tawk.g — gets the new infrastructure exercised end-to-end before scaling up to Tawk.g/Tawk.act migration.

**Definitions earned:**

- **Hook site** — a named position inside a method, written as a tok-recognizable goto label, that directives can address by name to inject debug or behavior code. Format/behavior lives in directives; the method file stays clean. (→ candidate for bible Key Design Decisions as a general pattern)
- **Labels-as-locals shorthand** — `Type Foo:, Bar:;` declaration form where the trailing colon after each name marks "this is a labeled child of the input, generate the getter for it." Tok emits the per-name accessor call. Lives in incant for GroupItem; extends to plg for PLGitem via an `external PLGitem { initializer getLabel; }` block in the right scope. (→ candidate for bible glossary once Track B ships)
- **Tok one-initializer-per-scope constraint** — tok currently doesn't support multiple initializers in scope at the same time. Initializer externals can't be co-located in widely-included files; they live with their consumers. (→ Key Design Decisions for the constraint; Housekeeping for the future lift)
- **`getLabel(name)` for PLGitem** — `return children[name];` returns null on absent key. Distinct from incant's `getLabelGroup`, which adds an unwrap loop for GROUP/Method/Rule passthroughs that plg doesn't have.
- **Author-writes vs generator-augments** (for .act label declarations) — author-writes is the chosen path: .act author writes `PLGitem Foo:, Bar:;` explicitly at the top of each action. Trivial in incant practice. Generator-augments (plg lightly parsing .act to prepend declarations automatically) parked as an unused alternative.
- **Mirror principle** — when plg and incant share an idea, mirror the *pattern* freely; carry project-specific suffixes on any name visible at global or shared scope. File-local and method-local names can match across projects without cost.

**Open questions:**

- *Implementation gate*: define "passes muster" for Testing.twk concretely before declaring Track A or Track B done. Probably means: setRules() block matches expected shape, set declarations are clean (the SetVariableplgNow bug fixed), labeled captures land in children correctly, the spliced .act content (when there is some) compiles through tok cleanly. Specifics pin down when implementation gets close.

**Lessons / corrections:**

- Initial Clay framing put the PLGitem initializer external in a shared support/Include externals file. Wrong: tok's one-initializer-per-scope constraint means widely-included files can't carry initializer declarations. The correct home is wherever the type's actions live — same principle that put `getLabelGroup` at the top of incant's GroupRules.twk. Fix surfaced by Tony noting the historical reason for getLabelGroup's location.
- Initial Clay framing tangled two questions: the .rtn/.act files (not plg's job to parse) and the .g grammar includes (plg's job, were working as of 2026-05-16). Worth keeping separate. The "regen file only contains setRules()" observation is *correct output*, not a bug — plg emits setRules; the .rtn/.act content is TAWK's to handle.
- Initial Clay default of "rename plg's flags to match incant" was the wrong instinct. Cross-project name alignment is good *for patterns and file-scoped names*, but global flag names need project-specific suffixes to survive both projects coexisting in a single binary. The `-PLG` suffix on `debugRulePLG` etc is doing real work; preserve it. The mirror principle applies to patterns, not to globally-visible names.
- SetVariableplgNow bug is separate from this design and is the actual blocker for testing any of this. Treat as a precondition for implementation work, not part of the design session.
- Bible's "Action blocks design (approved, TODO)" entry needs reclassification — the inline syntax is no longer on the active design path. Doc-debt to address as part of Session 9 graduation.

**Texture worth preserving:**

The trigger for this session was Tony's note that the existing plg debug output had become overwhelming and he wanted to mirror incant's pattern. That request — taken on its own — could have been a small directive-wiring task. But pulling the thread surfaced the parallel question of where plg action code lives, which pulled the further thread of incant's labels-as-locals shorthand and its initializer-external wiring. Three apparently separate pieces (debug format, action code shape, label getter machinery) turned out to share the same underlying pattern: *tok directives matching named sites in twk source, with the format and behavior living in the directive entries rather than the source files.*

The plan that fell out has the symmetry of "plg gets the incant idiom in the two places it currently doesn't have it." Debug machinery and action code are the two big surface areas where plg and incant currently differ in style; both can converge on incant's model with limited new infrastructure because the underlying tok-directive mechanism is already in anger in plg.

The session also surfaced a tok-level limitation (one initializer per scope) that wasn't visible until the discussion forced it. That limitation is what kept incant's `getLabelGroup` declared at the top of GroupRules.twk rather than in a shared header. Tony knew this; Clay didn't, and the wrong initial framing ("put it in a shared externals file") got caught and corrected. The fix is a constraint to document, not a bug to fix — but pinning it where future-Claude can find it cold matters, because the wrong framing was Clay's natural instinct and will be again next time without the pin. Hence the deliberate choice to document it in both Key Design Decisions (the constraint) and Housekeeping (the future lift).

The .act-as-splice resolution is the kind of move that's worth marking on its own. Two options were on the table: inline action blocks (new grammar work in plg) and side-file (some flavor of preserving .act). Tony's framing — "preserve .act, change it to be spliced like .rtn" — collapsed the dichotomy. The cut-and-paste model means plg doesn't need new grammar work AND .act files stay where action code naturally wants to live AND the migration of existing Tawk.act is mechanical. Three goods reached by reframing the question instead of solving the original version. Worth keeping in mind as a pattern: when a design choice presents as A-vs-B, check whether there's a reframing that delivers both.

A third piece, smaller: the cha cha at this session was textbook. Tony brought the deep tok fluency (the `initializer` keyword in externals, the one-initializer constraint, the historical reason for getLabelGroup's location, the action-method-as-label-declarer feature from old plg, the cross-binary namespace concern that kept `-PLG` suffix in play). Clay brought the structural framing (mapping the two tracks, surfacing dependencies, the HWF-vs-Clod-brief distinction, drafting the trim shape). Neither alone would have produced this design. Both were necessary; the friction at each step was where the real signal showed up.

---

## Glossary additions pending

Terms used in HWF that may want bible/glossary promotion once stable:

- *cha cha* — the working dance between Tony, Claude, and Clod. Currently informal.
- *isCLAUDE* — provisional name for an AI-as-field-type GroupItem kind. May survive into the implementation; may get renamed.
- *trim* — the four-section-plus-texture session record. Replaces, doesn't append.
- *graduation* — the ritual of moving a settled session out of HWF.md into the attic, with prerequisite checks and attic-cleanup combined.
- *attic* — the sibling directory `HWFattic/` holding graduated session trims as static files. Recovery window before final retirement.
- *resurrection-reader* — fresh-Claude reading the .md files cold tomorrow with no memory of today. The audience all project documentation must serve.
- *indent-as-structure* — incant syntax model where indentation conveys block boundary and colon conveys block opening; replaces count-close and named-close mechanisms.
