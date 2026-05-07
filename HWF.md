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

**Format per session — four sections:**

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

## Conventions

- Session headings: `## Session N — Topic — Date — Status` (status is `open`, `closed`, or `deferred`).
- Sub-points within a session: `###` headings. Discussion as prose, bullets, or dialog — whatever fits. (For active sessions only; trimmed sessions use the four-section format above.)
- Speaker tags (Tony: / Claude: / Clod:) when useful, skip when not.

---

## Session 1 — isCLAUDE and the cha cha — opened 2026-05-06 — open

**Decisions:**
- isCLAUDE spine agreed: a field that looks like any other field to incant, with an attached method that fires on demand, calls Claude, wraps the response in a GroupItem.
- Persistence model: P-2 by default, with continuity carried *outside* the field via files-and-sources, not via in-field accumulation. Pattern mirrors Tony+Claude+Clod's daily resurrection-from-files cha cha.
- Stand-down ritual at project boundaries handles cross-project continuity: heavier than HWF trim, written once at boundary, read on pickup. Lives in the field's `sources` list, not as separate machinery.
- "Deliberate compression at boundaries" is the architectural pattern, applied at multiple scales (HWF trim per session-touch, stand-down per project-set-aside, isCLAUDE sources per fire).

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

## Glossary additions pending

Terms used in HWF that may want bible/glossary promotion once stable:

- *cha cha* — the working dance between Tony, Claude, and Clod. Currently informal.
- *isCLAUDE* — provisional name for an AI-as-field-type GroupItem kind. May survive into the implementation; may get renamed.
- *trim* — the four-section session record. Replaces, doesn't append.

---

## Sessions index

1. isCLAUDE and the cha cha — open — 2026-05-06
