# TODO.md — PLG/TAWK/Incant Ecosystem
*Read this first every session. Keep it current.*

---

## 🔥 Immediate (current sprint)

### PLG — Self-hosting
- [ ] Bare-include over-matching fix — use plgDirectives for instrumentation (lesson learned: directives first, print statements second)
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
Clod wrote his first real incant (interpret()) — the demon is emerging! 😄
- [ ] Read `utilities` before next incant write
- [ ] Fix `gIF` — first real bytecode emitter task
- [ ] Fix `gExpressioN`
- [ ] Expand testByteCode with more cases

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

- [ ] Incant repo: commit Maps/ deletion (support symlink cleanup)
- [ ] Incant repo: commit XML/Notions/flags modification
- [ ] Add `InProcess/InProcess.xcworkspace` to all CLAUDE.md files
- [ ] plg.g `%%` assumption — document/fix
- [ ] doNotGuard accumulation — Header/CommentPartBoDY/ForwardDecl/Trailer
- [ ] +1000 offset reporting quirk — benign but odd
- [ ] Bible mirror drift — projectBible.md in tawk and incant repos is behind plg/support as of 2026-05-07 (resurrection-reader standard, May 7 polishes, path-bug entry). Mirror and verify per push-then-fetch protocol. Support is current.

---

## ✅ Done (sessions 1-4)

- [x] Four repos created and public
- [x] CLAUDE.md, TODO.md, projectBible.md in all four repos
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
- [x] plg.g parses end-to-end — 37 rules
- [x] Generator rewrite — addTest format, 76% smaller
- [x] banged/noSkip fix — 6 coordinated changes
- [x] noSkip propagation through `}&` modifier
- [x] Comment+CommentBody decomposition
- [x] Grammar source tracked in Parse/Revision/Grammar/
- [x] APFS case collision resolved — Grammar/ subdir
- [x] plg.g self-describes — Start/Header/Include fixed
- [x] TAWK Directives documented
- [x] Xcode project rebuilt via xcodegen/project.yml
- [x] Session docs created and tracked
- [x] Bytecode design decisions settled
- [x] interpret() written in incant ✅ — Clod's first real incant!
- [x] bcOPs registry + C++ handlers (Bytecode.mm)
- [x] Gating hook in GroupRules.mm:786 (bytecodE → interpret())
- [x] All 4 repos updated with bytecode section
