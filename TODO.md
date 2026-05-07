# TODO.md — PLG/TAWK/Incant Ecosystem
*Read this first every session. Keep it current.*

---

## 🔥 Immediate (current sprint)

### PLG — Self-hosting
- [ ] Action blocks feature — `Action actionName { TAWK body } ;` generates `void actionName(PLGparse *state, PLGitem *item) { body }`. Required for self-host loop closure without TEMPORARY BRIDGE.
- [ ] Grammar reorganization — plg.g (structure only), action.g (Action blocks), plgRules.g (shared rules)

### Incant — testByteCode POP
- [ ] Add `Bytecode.mm` to incantGUI Xcode target (manual: drag into project navigator, check target)
- [ ] Emitter rewrite: `gIF` — generate bytecodE attribute instead of old C++ source output
- [ ] Emitter rewrite: `gExpressioN` — same
- [ ] Verify `gBlocK`, `gFOR`, `gWhilE`, `gDO` — emitting bytecodes or old-style C++ strings?
- [ ] Run testByteCode end-to-end — POP: `if righty > 0; maximus = righty * 2;` → `maximus = 26`

### Incant — Build
- [ ] incantGUI build cleanup — PLGset moved to support repo, update project references
- [ ] PLGsetParse API drift — old .twk references stale API (addTest moved from PLGset to PLGparse). Porting job, needs Anthony input on what stays vs retires.
- [ ] Workspace: `InProcess/InProcess.xcworkspace` — umbrella workspace. Use this.

---

## 📋 Next Up

### PLG
- [ ] Wash & rinse cycle — architectural ghosts, dead code, old patterns
- [ ] Support/Frame audit — long/int conversions, deprecated functions
- [ ] Xcode workspace (Shape B) — plg + support + tawk + incant properly wired

### TAWK
- [ ] TAWK autopsy — TGV approaching 🚄
  - [ ] Empty comment line context reset fix
  - [ ] `@field` context markers
  - [ ] Include search path support
  - [ ] Include guards generated automatically
  - [ ] Unused field warning

### Incant
- [ ] `gPrinT` — implement (stub)
- [ ] `gXpress` — implement (stub)
- [ ] `gDeclare` — verify correct
- [ ] `genPrint` in Generate.rtn — replace with bytecode equivalent
- [ ] `runCall` handler — implement when first test needs it
- [ ] JSON rule — find in attic, POP
- [ ] Bot messaging project — assess InProcess/Bot
- [ ] Distributed GroupItem messaging design

### Sub-goal: Get Clod Fluent in Incant
Clod wrote his first real incant (interpret()) — the demon is emerging.
- [ ] Read `utilities` before next incant write
- [ ] Expand testByteCode with more cases (after gIF/gExpressioN land — see Immediate)

---

## 🔭 Longer Term (HPDL)

- [ ] Claude as native GroupItem field type (`isCLAUDE`)
- [ ] Incant as distributed virtual OS
- [ ] Go-style channel messaging — Ken Thompson approved theft
- [ ] ZFS-flavored storage — copy-on-write as GroupItem operations
- [ ] Incant display/layout field
- [ ] File system as GroupItems
- [ ] PLG written in Incant
- [ ] Incant self-hosting via JIT
- [ ] Xcode-like development environment written in incant

---

## 🗂️ Housekeeping

- [ ] Add `InProcess/InProcess.xcworkspace` to all CLAUDE.md files
- [ ] plg.g `%%` assumption — document/fix
- [ ] doNotGuard accumulation — Header/CommentPartBoDY/ForwardDecl/Trailer
- [ ] +1000 offset reporting quirk — benign but odd

---

## ✅ Done

### Recent (2026-05)
- [x] PLG `process()` CWD-relative path contract — drop sourceDir-from-filename, basename-only outFile (commit da51193, day 2)
- [x] Bible May 7 polishes — resurrection-reader standard, day-3 inverse-failure lesson, path-bug entry, eRocka removed, TOK xcode references dropped
- [x] Bible mirror sweep across all four repos (plg → support → tawk → incant) — same MD5 across all four
- [x] Incant repo cleanup commit (b5375e8) — Maps/ deletions + symlink, BackupXML/ removed, XML/.DS_Store untracked, XML/junk removed, Tests/ added to .gitignore, XML/Notions/flags updated
- [x] HWF.md trim ritual added; Session 1 lessons captured day-3 recursion + inverse failure
- [x] Verification protocol added — fetch from GitHub raw with cache-busting (commit-pinned URL or `?v=` query) after every push

### Earlier
- [x] Four repos created and public (plg, incant, support, tawk)
- [x] CLAUDE.md, TODO.md, projectBible.md present in all four repos (each repo's content is repo-focused, bible content mirrored)
- [x] "What Is Incant?" wiki page
- [x] Support static library (libsupport.a)
- [x] PLGset moved to support repo
- [x] foundIn dependency cycle resolved
- [x] addTest() implemented, setRules() 48% smaller
- [x] Guards implemented — setGuard() howitzer fix (8 cases, null vs empty)
- [x] .next chain merges, Alternative.match optional element fix
- [x] Two-table process() pattern
- [x] Full callback chain: RuleplgNow, AlternativeplgAct, ElementplgAct, ElementTypeplgAct
- [x] PLGitem.runDeferred() implemented
- [x] Testing.g parses end-to-end, labels work
- [x] plg.g parses end-to-end — 39 rules
- [x] Tawk.g parses end-to-end — 200 rules captured, 177 populated
- [x] Generator rewrite — addTest format, 76% smaller
- [x] banged/noSkip fix — 6 coordinated changes
- [x] noSkip propagation through `}&` modifier
- [x] Comment+CommentBody decomposition
- [x] Pre-parse comment stripping — stripComments() in PLGparse
- [x] Grammar source tracked in Parse/Revision/Grammar/
- [x] APFS case collision resolved — Grammar/ subdir
- [x] plg.g self-describes — Start/Header/Include fixed
- [x] Bare-include over-matching investigated — STALE FRAMING, was never actually broken (directives trace proved Body was working correctly)
- [x] TAWK Directives documented and used in anger
- [x] Xcode project rebuilt via xcodegen/project.yml
- [x] Session docs created and tracked
- [x] Bytecode design decisions settled
- [x] interpret() written in incant — Clod's first real incant
- [x] bcOPs registry + C++ handlers (Bytecode.mm)
- [x] Gating hook in GroupRules.mm:786 (bytecodE → interpret())
- [x] All 4 repos updated with bytecode section
