# TODO.md — PLG/TAWK/Incant Ecosystem
*Read this first every session. Keep it current.*

---

## 🔥 Immediate (current sprint)

### PLG — Self-Hosting
- [ ] Tests directory reorganization (Tokf/Tests, Parse/Tests — symlink to source)
- [ ] Fix missing `ActionOption` / `ActionOption2` definition (diagnosis pending)
- [ ] Self-host check — feed generated `plg.twk` back through tawk, compile, verify
- [ ] Grammar split — `plg.g` (structure), `action.g` (Action blocks), `plgRules.g` (shared rules)

### Incant — Conceptual Bible
- [ ] Clay drafts "What Is Incant?" wiki page from Pages document + session HWF discussion
- [ ] Clod sets up GitHub wiki on incant repo
- [ ] Push wiki page

---

## 📋 Next Up

### PLG
- [ ] Wash & rinse cycle — scan for architectural ghosts, dead code, old patterns
- [ ] Support/Frame audit — long/int conversions, deprecated functions, dead code
- [ ] shrink_setrules.py improvements — track named-set variable assignments properly
- [ ] Xcode workspace (Shape B) — wire plg + support + tawk + incant

### PLG — Action blocks (new top-level grammar construct)
- [ ] Define `Action actionName { TAWK body } ;` syntax — inline TAWK method tied to a rule.
  - PLG generates: `void actionName(PLGparse *state, PLGitem *item) { body }`
  - Update Rule definition to allow optional immediate and deferred action-name references before options.
  - Goal: `.g` files become self-contained — no separate `.act` or `.rtn` files needed.
  - Conceptually parallel to incant field code blocks. Design approved, implementation pending.

### PLG — known issues / quirks
- [ ] **`%%` first-separator is mandatory in Start.** Grammars without any `%%` (rules directly after preamble or no preamble at all) won't parse — Start's first `%%` element is min=1. Worth a sanity check before broader use.
- [ ] **Buffer offset reporting overshoots by ~1000.** `parsed 4939 chars` for a 3939-byte file. Benign so far but odd; might bite when buffer accounting matters (e.g. precise error messages). Likely an offset baseline issue in start/cursor arithmetic.
- [ ] **`doNotGuard` workarounds accumulate.** Header / CommentPartBoDY / ForwardDecl / Trailer all set `doNotGuard = true` because their setGuard() couldn't compute a usable FIRST set. Will compound when generated rules try guard-based fast-rejection. Audit and tighten when revisiting setGuard for the negated-set / kSet `^X` cases.

### TAWK
- [ ] Create TAWK repo Xcode project (project.yml)
- [ ] TAWK autopsy — work through issues table in bible
  - [ ] Empty comment line context reset fix
  - [ ] `@field` context markers replacing implicit resolution
  - [ ] Include search path support (`-I` style)
  - [ ] Include guards generated automatically
  - [ ] Unused field warning (emit warning, keep field)

### Incant
- [ ] JSON rule — find in attic, get running, POP
- [ ] Bot messaging project — assess what's in InProcess/Bot, what's salvageable
- [ ] Distributed GroupItem messaging design
- [ ] JIT — ongoing

---

## 🔭 Longer Term (design conversations needed first)

- [ ] Claude as native GroupItem field type (isCLAUDE) — design in Open Design Questions
- [ ] Incant as distributed virtual OS — GroupItems as deployable units, cross-platform messaging
- [ ] Incant display/layout field — GroupItem that handles typesetting and rendering
- [ ] File system as GroupItems — incant represents the file system natively
- [ ] PLG written in Incant — the ultimate bootstrap
- [ ] Incant self-hosting via JIT — close the loop, shrink the C++ bootstrap layer
- [ ] Xcode-like development environment written in incant

---

## 🗂️ Housekeeping

- [ ] Update bible after each significant session (Clay drafts, Clod pushes to all 4 repos)
- [ ] Incant repo: commit Maps/ deletion (support symlink cleanup)
- [ ] Incant repo: commit XML/Notions/flags modification
- [ ] Bible: flesh out Open Design Questions 2-6 (labels/rules, action/method, defer, messaging, Claude field)
- [ ] GitHub UI note: may fail on dirs with many extensionless files (WorkingOn/, Stash/ etc.) — files are safe, rendering issue only

---

## ✅ Done (recent)

- [x] Four repos created and public (plg, incant, support, tawk)
- [x] CLAUDE.md in all four repos
- [x] projectBible.md mirrored to all four repos
- [x] Support static library (libsupport.a)
- [x] PLGset moved to support repo
- [x] addTest() implemented
- [x] setRules() shrunk 48%
- [x] Guards implemented (recursion, cycle protection, zero-advance-stop)
- [x] .next chain merges (BodyN, AlternativeBlockN etc.)
- [x] Alternative.match optional element fix
- [x] Two-table process() pattern
- [x] RuleplgNow firing — Max + Integer in fresh rules table
- [x] Testing.g parses end-to-end (90/91 bytes)
- [x] Xcode project rebuilt via xcodegen/project.yml
- [x] TAWK iteration trap documented in bible across all repos
- [x] `PLGitem.runDeferred()` — defer/immediate cascade firing
- [x] `defer` field firing wired in `PLGrule::match()`
- [x] `ElementplgAct` rewritten for new model (Element/Alternative)
- [x] `ElementTypeplgAct` — min/max repetition wired
- [x] `RuleOptionsplgAct` / `RuleOptionplgAct` / `AlternativeplgAct` chain — rules parse as SEQUENCE not CHOICE; labels propagate
- [x] Testing.g parse builds full Element structures in fresh rules table (`generateRules()` emits parsed rules, not meta-grammar)
- [x] plg.g parses end-to-end — 23 rules defined, bootstrap loop closed
- [x] Include resolution wired; banged/noSkip propagation restored; cursor restore on alt failure
- [x] `plg_rules.g` → `plgRules.g` rename (no underscores)
