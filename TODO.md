# TODO.md — PLG/TAWK/Incant Ecosystem
*Read this first every session. Keep it current.*

---

## 🔥 Immediate (current sprint)

### PLG — Callback Chain
- [ ] Design and implement `PLGitem.runDeferred()` — accumulates (callback, item) pairs during match, fires after
- [ ] Wire `defer` field firing in `PLGrule::match()` (paired with `immediate`)
- [ ] Rewrite `ElementplgAct` for new model (Element/Alternative, not PLGtester)
- [ ] Rewrite `RuleOptionsplgAct`, `RuleOptionplgAct`, `AlternativeplgAct` chain
- [ ] Verify: Testing.g parse builds complete Max + Integer rule structures (not just names)
- [ ] Expand to plg.g — full bootstrap

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
