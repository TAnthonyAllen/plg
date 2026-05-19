# Session 9 — plg debug + actions, incant-mirrored — opened 2026-05-18 — graduated 2026-05-19

*Graduated trim. Both tracks closed end-to-end. Testing.g → Testing.twk → Testing.C → Testing.o pipeline works; debug machinery layers cleanly on top via tok directives on named hook sites in PLGrule::match and Alternative::match.*

---

**Decisions (planning phase, 2026-05-18 morning):**

- Parse debug machinery moves out of PLGrule.twk source into plgDirectives, mirroring incant's pattern: named hook sites in the parse method, format hung on them via directive entries. Hook sites are tok-recognizable goto labels in PLGrule::match, addressable by directives the same way incant's `parse` hooks are addressable.
- Four hook sites in PLGrule::match: `parseAttempt` at entry before alternatives loop; `matchSucceeded` inside alternatives loop when an Alternative returns non-null (alt index in scope, plg-specific value-add); `matched` after successful match with label and atRuleMark info in scope; `parseReturn` at final return.
- plg keeps its existing debug flag names `debugRulePLG`, `debugGuardPLG`, `doNotGuard`. The `-PLG` suffix protects against namespace collision if plg and incant ever coexist in a single binary. The mirror principle applies to *patterns*, not to *globally-visible names*.
- Action code adopts incant's model: one input PLGitem passed to the action, labeled children declared as locals via trailing-colon shorthand at the top of the action body.
- PLGitem grows a `getLabel(name)` accessor — `return children[name];`. Simpler than incant's `getLabelGroup` because plg's PLGitem has no GROUP/Method/Rule wrapper distinctions.
- .act files preserved, repurposed from "parsed by plg" to "spliced verbatim by plg" — same pipeline as .rtn. Author-writes model: .act author declares labels explicitly via `PLGitem Foo:, Bar:;` shorthand at the top of each action. plg does not parse the .act content; it concatenates.
- `external PLGitem { initializer getLabel; }` block lives at the top of each .act file. Same principle as incant's `getLabelGroup` initializer at the top of GroupRules.twk: initializer externals live with their consumers because tok currently supports only one initializer per scope.
- IncludeplgNow routes by extension: `.g` files sub-parsed via the stashed Body rule (existing path); `.rtn` and `.act` files spliced verbatim into a buffer on PLGparse, with `.act` parsing a small declarations line at the top to harvest action method names for extern emission.
- The previously bible-approved inline-action-blocks syntax (`Action actionName { TAWK body } ;`) is parked as a considered-not-chosen alternative.

**Decisions (execution phase, 2026-05-18 during Clod work):**

- Action method signatures are uniform: every action takes `(PLGparse state, PLGitem iTEM)`. Mirrors incant's convention.
- Action method naming: `RuleNameAct` for post-match (`currentRule.defer`), `RuleNameNow` for immediate (`currentRule.immediate`). Alternative-N gets digit infix: `RuleName2Act`. No `plg` suffix. Alphabetical sort clusters by rule name.
- Action method bodies stay extern, top-level, outside the class wrapper. Mirrors incant's working pattern, mirrors old plg's pattern. NOT class members.
- Class wrapper shape: `class <BaseName> extends PLGparse { void setRules() { ... } }`. BaseName from input filename stem (Testing.g → Testing). Only setRules inside the class; includes, externs, .act splice all outside.
- main() lives in PLGmain.twk, separate file, linked in at build time. Modified separately to accept grammar filename.
- Declarations line at top of .act names *method names directly*, not rule names. plg emits one extern per declared name, return type derived from suffix (Now → int, Act → void).
- File shape of generated .twk: `[includes] [external block with action externs] [.act splice — top-level definitions] [class <BaseName> extends PLGparse { setRules() }]`. No main inside.
- Output naming: plg writes `<base>.twk` into CWD. The `.regen.twk` convention was retired overnight 2026-05-18. Future feature candidate: explicit output-directory directive in the .g file.

**Decisions (closeout phase, 2026-05-19 — Track A directives polish):**

- The `succeeded` field on PLGrule is a debugger breakpoint anchor, not parser state. The directive bodies that write `succeeded = true/false` exist so a debugger can pause at the named moment. Removing them would remove the anchor — keep as-is.
- Alternative directives gate on `state.debugRulePLG` only (not `item.deferRule.debug`). Alternative has no per-rule debug context to consult safely (item may be null on the failure path; item.deferRule may be null even on success). Per-rule selectivity already lives at PLGrule level; Alternative traces are always nested inside a PLGrule trace, so the global switch is enough.
- Alternative directive bodies reference correctly-scoped objects: `elementFail` reads `elem.label` (Element object, always in scope); `elementSuccess` reads `item.label` (matched PLGitem, in scope and non-null on this path).
- `setGuard endSetGuard before` directive parked inactive (typo on activation keyword + no matching label). Re-activated when guard debugging becomes a need.

**Definitions earned:**

- **Hook site** — a named position inside a method, written as a tok-recognizable goto label, that directives can address by name to inject debug or behavior code. Format/behavior lives in directives; the method file stays clean.
- **Labels-as-locals shorthand** — `Type Foo:, Bar:;` declaration form where the trailing colon after each name marks "this is a labeled child of the input, generate the getter for it." Tok emits the per-name accessor call. Wired into plg via `external PLGitem { initializer getLabel; }`.
- **Tok one-initializer-per-scope constraint** — tok currently doesn't support multiple initializers in scope at the same time. Initializer externals can't be co-located in widely-included files; they live with their consumers.
- **Mirror principle** — when plg and incant share an idea, mirror the *pattern* freely; carry project-specific suffixes on any name visible at global or shared scope. File-local and method-local names can match across projects without cost. *Examples: `debug` (plg) vs `debugged` (incant) — same purpose, different names, no friction.*
- **Author-writes (vs generator-augments)** for .act label declarations — author-writes is the chosen path: .act author writes `PLGitem Foo:, Bar:;` explicitly. Generator-augments (plg lightly parsing .act to prepend declarations automatically) parked as an unused alternative.
- **Trap pattern (bible #12)** — hand-maintained external mirror in PLGrevision diverges silently from PLGparse/PLGitem/PLGrule class definitions when authors forget to update the mirror. Failure surfaces loudly at consumer parse, not at source. Fired and caught this session (Brief 2 omission, surfaced at Brief 7 two briefs later).
- **Empty-vs-populated verification gap** — verification under an empty case (e.g., empty actionNames) can pass while the populated case fails. The populated case is the real test; empty case is at best a sanity check. Brief 4 missed the includes and class-wrapper deliverables; Brief 7's populated case surfaced both.
- **Debugger breakpoint anchor** — a member variable (e.g., `PLGrule.succeeded`) written by debug directives at named moments, used purely as a stable spot to set a debugger breakpoint. The writes don't drive logic; they let the debugger pause when a particular branch fires. Worth a name because the pattern surfaces in directive review — Clay's first instinct was to treat the writes as dead code.

**Open questions:**

- *none.* Session closes complete.

**Lessons / corrections:**

- Initial Clay framing put the PLGitem initializer external in a shared support/Include externals file. Wrong: tok's one-initializer-per-scope constraint means widely-included files can't carry initializer declarations. Fix surfaced by Tony noting the historical reason for getLabelGroup's location.
- Initial Clay default of "rename plg's flags to match incant" was the wrong instinct. Cross-project name alignment applies to patterns and file-scoped names, not globally-visible names. The `-PLG` suffix on `debugRulePLG` is doing real work; preserve it.
- The `debug`/`debugged` finding (Brief 5) — Session 9 design proposed adding `rule.debugged`; plg already had `rule.debug`. Check existing source before designing additions. Lesson: when designing a class addition, grep the class definition first.
- The Brief 4-surfaces-by-Brief-7 pattern — verification gaps that only surface under populated cases. The producer-side test can pass while the consumer-side test fails. The consumer of a deliverable is often the real verifier of that deliverable's completeness.
- The two-day trap-pattern cycle (bible #12 documented 2026-05-17, fired and caught 2026-05-18) — doc-debt earning its keep at unprecedented speed. Real-instance pinning at fire time (not later) compounds the documentation's value for future-readers.
- First-pass parse traces can mislead about which level failed. The Brief 1.5 "optional Header bug" hypothesis was Clay's read of an initial trace; second-pass trace showed optional handling working correctly and the failure was a missing semicolon in Testing.g. Get consume/cursor data before drawing causation conclusions.
- Brief-writing should include tooling mechanics when the brief touches Clod-first machinery. Today's Brief 6 wake-up said "restore plgDirectives" without explaining that `tok File.twk plgDirectives` (positional second arg) is how directives get baked in. Clod figured it out by hoohaa. The mechanics live in Tony's head; if Clay doesn't surface them, Clod rediscovers them. Future: when a brief touches tooling Clod hasn't used before, the brief either includes the mechanics or points at a working example.
- Initial Clay read of directive review found "dead code" in PLGrule.succeeded writes. Wrong — succeeded is a debugger breakpoint anchor, not parser state. Tony surfaced the actual idiom; Clay's "simplify by removing succeeded" recommendation got withdrawn. Lesson: when something looks like dead code, ask why it's there before recommending removal. Code with no logical purpose may still have an operational purpose (debugging, profiling, breakpoint anchoring).
- Staged verification pattern earned its keep on the directive POP. Three steps: (1) baseline with directives parked, (2) directives present but debug off, (3) directives active. Each step isolates one source of failure: drift since last commit (1), directive infrastructure changing behavior when off (2), debug machinery itself (3). Cleaner diagnosis than a single end-to-end test would give.
- One-.twk-at-a-time directive review with Q1 (placement) before Q2 (content) kept the review tractable. Reviewing both files' directives at once produced too much text for Tony to work through; serializing per-file and per-question made each review beat focused.

**Texture worth preserving:**

The trigger was Tony's note that existing plg debug output was overwhelming and he wanted to mirror incant's pattern. That request alone could have been a small directive-wiring task. Pulling the thread surfaced the parallel question of where plg action code lives, which pulled the further thread of incant's labels-as-locals shorthand and its initializer-external wiring. Three apparently separate pieces (debug format, action code shape, label getter machinery) turned out to share the same underlying pattern: *tok directives matching named sites in twk source, with format and behavior living in directive entries rather than source files.*

The plan that fell out has the symmetry of "plg gets the incant idiom in the two places it currently doesn't have it." Debug machinery and action code converged on incant's model with limited new infrastructure because the underlying tok-directive mechanism was already in anger in plg.

The .act-as-splice resolution is worth marking on its own. Two options were on the table: inline action blocks (new grammar work in plg) and side-file (some flavor of preserving .act). Tony's framing — "preserve .act, change it to be spliced like .rtn" — collapsed the dichotomy. Three goods reached by reframing the question instead of solving the original version: plg needs no new grammar work, .act files stay where action code naturally wants to live, Tawk.act migration is mechanical. When a design choice presents as A-vs-B, check whether there's a reframing that delivers both.

**Execution day (2026-05-18) — Track B closure at scale.** 13 commits across Parse and support repos: Brief 1 (SetVariableplgNow fix) → Brief 1.5 (Testing.g semicolon + fixtures tracked) → Brief 2 (PLGitem.getLabel + polish) → Brief 3 (IncludeplgNow routing + PLGrevision tracked) → Brief 4 (generation pipeline + two polishes) → Brief 7 (Testing.act new-shape) → Brief 8 (class wrapper). Plus Brief 5 (hook sites, dc5118f) closing Track A halfway. Each brief landed with its own commit; findings landed as separate commits when they surfaced mid-flight. The bible #12 trap pattern, documented the previous day, fired during Brief 7 and was caught fast because documented — two-day cycle from "wrote down the trap" to "trap fires, gets diagnosed fast."

The "out to the woodshed" pattern: Tony coined this morning's planning rhythm — bring Clay's plan to the woodshed, torque it down with hard-won fluency, produce a working-level plan with brief sequence + dependencies + victory criterion before Clod wakes up. The plan did this in roughly an hour of cha cha; Clod's execution rode that planning all day. The woodshed move is the design discipline before execution starts; once Clod is awake, the woodshed has done its work.

The cha cha worked at scale because friction in both directions was productive. Tony pushed back on Clay (squirrel-chasing on link errors, wrap-around-axle on the actions-list problem, the "tok complains loudly" correction to a too-alarming framing of the trap). Clay pushed back on Tony (held the "actions stay extern" call open until Tony confirmed it was deliberate). Clod surfaced findings rather than failures; Clay matched the tone from the design side. None of the three seats were redundant — Tony brought architectural fluency and in-the-moment instinct, Clay brought structural framing and .md-archival memory (Tony doesn't read .md files; that's Clay's seat), Clod brought execution rigor and findings discipline. Three seats, three roles, one cha cha.

Clod's belt-and-suspenders earned explicit mention. Throughout the session: verification ran both empty AND populated cases, smoke tests after small changes, three commits where one would do (preserving narrative durability), state-of-disk reports listing what didn't change alongside what did, and the "Stak has no []" observation that became its own tracking item. Confident enough to commit without asking; disciplined enough to know when asking matters.

**Closeout day (2026-05-19) — Track A polish.** Smaller arc than Track B but earned its own lessons. Tony did Brief 6 (plgDirectives entries) after-hours; tok ran clean on the .twk files. Morning review found one real bug (Alternative directives gating on a null-deref path) plus an inactive directive (setGuard) and some cosmetic items. Clay flagged the bug, Tony fixed via paste-and-go. Three-step staged verification (baseline → directives-off → POP-with-debug-on) caught nothing wrong — Testing.twk byte-identical across all three steps, debug trace tree-indented and readable. Two commits: pre-existing regen drift cleanup + Brief 6.

The three findings Clod surfaced at Step 3's completion — tok directives are positional (not auto-discovered), #PLG and #PLGparse directive blocks unbaked (out of Brief 6 scope), no CLI toggle for debugRulePLG (used a one-line PLG.C patch + revert for Step 3) — all landed as TODO items rather than mid-session work. Findings discipline held: Clod surfaced rather than chased.

**Final commit chain across the session (2026-05-18 + 2026-05-19), in order:** 19f9f68, af39a11, 9fc46aa, d5cbda6, e026ade, b9264ce, 82b9bf8, 233794b, 454fdc3, 0e4d04a, 0baf44b, c6e5314, 6f45f5c, 06083f0, dc5118f, 07b4ba3, a03764a. Plus mirror commits 64f3d1b (incant), 9696538 (tawk), 598dc1e (support), d166123 (Parse) for the end-of-2026-05-18 sync. 21 commits total across four repos for Session 9.
