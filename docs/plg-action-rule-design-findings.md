# plg self-hosting + action-rule — findings for the design pass (2026-05-31)

*Written by Clod after a deep recon session. Feeds Clay's plg design pass. The
sequencing decision (Tony): **design + fix plg first, then return to the Tawk
task.** This doc is the raw material for that design pass — especially the
named-options decision, which everything else follows from.*

## How we got here

Started on the Tawk task: make `plg Tawk.g` generate a `Tawk.twk` that `tok`
can compile (it currently dies with `ERROR Inheritance`). Tracing the failure
bottomed out in a deeper truth: **plg is self-hosting and its grammar sources
have drifted from the hand-revised runtime.** So plg gets fixed first.

## The five facts that frame the whole thing

1. **plg is self-hosting.** `plg plg.g` → `PLG.twk` → (`tok`) → `PLG.C` → the plg
   binary. `plg.g` drives it via `insert(plg.rtn)` + `include(plgRules.g)` +
   `include(plg.act)`. The grammar sources live in **`Parse/Grammar/`**
   (git-tracked; their home — do not move): `plg.g`, `plg.rtn`, `plg.act`,
   `action.g`, `parts.g`, `plgRules.g`, `PLGincludes`.

2. **The grammar sources are STALE vs today's `PLG.twk`.** `plg.rtn` is from
   2021; `plg.g`/`plgRules.g` from May 5; the live top-level `PLG.twk` is a
   ~61 KB hand-revised file from today. `Grammar/plg.twk` is only ~12 KB — **not**
   the current generated parser. **Running `plg plg.g` now would regenerate an
   OLD plg and clobber the refactor.** It has not been run. The POP for the
   grammar files is exactly this: `plg plg.g` regenerates today's plg. Getting
   there means reconciling `plg.g`/`plg.rtn`/`plg.act` with the current runtime.

3. **The action rule is deferred — this is the crux.** `Grammar/plg.g:21`:
   ```
   include(plgRules.g)
   /* include(action.g) deferred until Action-blocks feature lands;
      current parser cannot ingest ActionEnd/ActionBody/ActionOption bodies */
   include(plg.act)
   ```
   `action.g` (the *separate rule for actions* Tony wants) already exists but is
   commented out of `plg.g`, because plg can't yet parse its
   `ActionBody`/`ActionEnd`/`ActionOption` constructs. The matching runtime hook
   `ActionRuleplgNow` is likewise disabled (`PLG.twk:774`,
   `//currentRule.immediate = ActionRuleplgNow;`). **Enabling this is the
   foundation fix.**

4. **Named options is the consequential design decision.** plg *numbers* rule
   options (`Body2`, `Body3`, `StatementBody16`) rather than naming them the way
   incant does. The numbering is currently load-bearing for one good reason:
   Tony wants **all related actions to emit in order in the resulting `.C`**, and
   the numbering preserves that ordering. Better names would be better, but the
   ordering guarantee must survive whatever syntax is chosen. Syntax is open
   (e.g. `Block : convert`, `Block(convert)`, …) — this is the first design call;
   it's designed into `plg.g` **and** `action.g` together.

5. **Actions are class methods inside the generated parser.** (Settled with Tony
   for the Tawk task; carries into plg's own self-host.) See the target shape
   below.

## The `.act` file format (input to the action-rule design)

Each `.act` file (e.g. `Tokf/Tawk.act`, `generate.act`, `parts.act`,
`declare.act`, `expression.act`) is a sequence of entries:

```
Block!            <- <RuleName> '!'  (the action header)
.                 <- a '.' alone on its line marks an IMMEDIATE action;
Statement   s;       its ABSENCE marks a DEFERRED action
... body ...
;                 <- a ';' alone on its line terminates the body
```

- `!`-headers are keyed **by rule name** — a `.act` `Name!` must match a rule the
  grammar declares (so the action attaches to it).
- Immediate (`.`-marked) → method `int <Name><Parser>Now(PLGitem iTEM)`.
  Deferred (no `.`) → `void <Name><Parser>Act(PLGitem iTEM)`.
- Counts (current files): 87 immediate + 24 deferred = 111 in the `.act` files.
  The 2021-era template `Tawk.twk` had 172 (145 Now / 27 Act) — the extra ~61
  came from inline-grammar actions and old numbered-option splitting. **Do not
  over-fit to 172**; the grammar has evolved.
- Body terminator is fragile to hand-parse (`\n;\n`); `action.g`'s
  `ActionBody`/`ActionEnd` rules are the principled way to delimit it.

## Target generated structure (what the action rule must produce)

`Tawk.twk` (and, self-hosted, `PLG.twk`) is **one class, no `external {}` block**:

```
include includes                      <- pulls frame/globals + tok.ext's `external Tawk`

class Tawk extends PLGparse
{
    <.rtn splice>                     <- parser-state fields + helper methods, INSIDE the class
    int  BlockTawkNow(PLGitem iTEM)    <- expanded action METHODS, inside the class
    {
        PLGitem start = iTEM.children["start"];   <- one per distinct labelled capture
        PLGitem line  = iTEM.children["line"];       (NO parser-pointer injection —
        ... body verbatim ...                         a method reaches members directly)
        return true;
    }
    ...
    void setRules() { setSkip(); ... }
}
```

- **Action *signatures* go in `tok.ext`'s `external Tawk { }` block**
  (`Include/tok.ext`, before its `}` ~line 764) — via the tok-externals
  mechanism, NOT emitted into the `.twk`. The PLGset/KeyTable field decls are
  already there ("added by PLG when parsing Tawk.g", ~lines 714–739).
- Captures use the **new PLGitem surface**: `iTEM.children["label"]` (or
  `iTEM.getLabel("label")`). `runDeferred` now takes a state arg; the parser
  static is **`Tok::testParser`** (`Tokf/Tok.twk`).

## What landed this session (working tree — NOT all to be committed)

**Keepers** (hand-written runtime classes — they belong where they are):
- `Parse/PLGparse.twk` — `generateRules` restructured (`include includes` →
  `class … {` → `.rtn` splice inside class → `writeActions` loop → `setRules` →
  `}`; external block removed); new `attachActions(String)` parses `.act` and
  binds bodies to rules.
- `Parse/PLGrule.twk` — `writeActions(parserName, output)` + `writeCaptures`
  implemented (new surface, no parser pointer, dedup labels across alternatives).
- `Include/PLGrevision` — `void attachActions(String content);` added to the
  `external PLGparse` block.

**Stopgap — DO NOT COMMIT** (gets replaced by the proper fix in `plg.g`/`plg.act`):
- `Parse/PLG.twk` — `IncludeplgNow`'s `.act` branch rerouted to
  `attachActions`. `PLG.twk` is *generated* from `plg.g`; this hand-edit is
  throwaway. (It also isn't fully working: regen still showed raw `Block!` in the
  output / 0 expanded methods — the `.act` is reaching the splice by a path not
  yet pinned down. Don't chase it; the action-rule redesign supersedes it.)

Verified `ERROR Inheritance` clears once the `.rtn` field decls move inside the
class (Step 2) — confirmed by a tok run; the remaining errors were all the raw
`.act` content, which the action rule eliminates.

## Build + gotchas

- **Build plg in Debug only** (Release `support` dep can't find `PLGparse.h`):
  `tok PLGparse.twk plgDirectives` (+ each changed `.twk`), then
  `xcodebuild -project plg.xcodeproj -target plg -configuration Debug`.
  `~/bin/plg` → `Parse/build/Debug/plg`. Always pass `plgDirectives` to `tok`.
- **Tawk task harness**: `plg Tawk.g` in `Tokf/Tests/` (its `Tawk.g`/`.act`/`.rtn`
  are symlinks to `Tokf/`); output is `Tokf/Tests/Tawk.twk`; target template is
  the parent `Tokf/Tawk.twk` (89a3abc baseline — toks; do NOT use HEAD `ef2730d`).
- **Case-collision**: macOS is case-insensitive — `Grammar/plg.twk` and top-level
  `PLG.twk` are the *same path*. (Moot now that grammar files stay in `Grammar/`.)
- Xcode `plg.xcodeproj` links to the `Grammar/` `.g`/`.act`/`.rtn` files are
  **broken** — Tony can't open them in Xcode. Repairing those references is a
  side task (the files themselves are fine in `Parse/Grammar/`).

## Design-pass running order (Tony → Clay)

1. **Named-options syntax** — most consequential; preserve ordered `.C` emission.
2. **`action.g` enablement** — what `ActionBody`/`ActionEnd`/`ActionOption` look
   like now and what the parser needs to ingest them; wire `ActionRuleplgNow`.
3. **Reconcile** `plg.g`/`plg.rtn`/`plg.act` with today's `PLG.twk` so the grammar
   describes the current runtime (the self-host POP: `plg plg.g` regenerates plg).
4. **Then return to the Tawk task** with the foundation solid.
