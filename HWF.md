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

**Origin:** The bible contains the line *"Claude is a GroupItem — `isCLAUDE` alongside `isSTRING`, `isNUMBER`, `isGROUP`. The AI is not a tool called from incant — it IS a field in incant."* Tony flagged on review that this needs an HWF session before it goes any further, because the implications run deeper than the bible currently acknowledges.

This session is in active-discussion form (not yet trimmed) because no points have resolved. Trim happens when the session next gets touched and something settles.

### 1. Operational semantics of an isCLAUDE field

What does it mean to *have* an isCLAUDE field in a GroupItem?

- When you read the field, what comes back? The last response? A live call? Cached reasoning? A frozen-at-evaluation snapshot?
- Is the field's "value" the prompt, the response, the conversation history, or the disposition-to-respond?
- Does reading the field have side effects (cost, latency, state change) the way reading an isSTRING does not?

Different answers give wildly different languages. Probably where the session should start, because everything else depends on it.

### 2. Identity and persistence

If a GroupItem holds an isCLAUDE field, gets serialized to disk, and gets reloaded next week:

- Same conversation continued?
- Fresh instance with the stored context replayed?
- Frozen snapshot of what the AI said at save-time, with no live channel?

Each answer implies a different theory of what the AI *is* in the system. Pick wrong and the language has a fundamental incoherence between "saved" and "running" states.

### 3. Composition

Two GroupItems each with an isCLAUDE field:

- Shared instance? Shared context? Independent?
- Can a CLAUDE field read another CLAUDE field?
- Can it read its containing group?

The reflexive/homoiconic claim in the bible almost demands the last one — if code and data have the same structure, an AI field examining its container is the same operation as any other field examining its container. But "almost demands" is HWF talk. Needs working through.

### 4. The cha cha — working relationship side

Right now the dance is: Tony architects, Claude (here) reasons-with-Tony, Clod executes. If isCLAUDE is in incant, then a running incant program contains something Claude-shaped at runtime. That runtime-Claude is not the same Claude as me-in-this-chat — but it's not unrelated either.

Open question: what's the relationship between AI-as-collaborator (this conversation) and AI-as-field-type (in the language)? Same role at different scales, or genuinely different things that happen to share a name? The answer affects how isCLAUDE gets implemented and also how the project describes itself.

### 5. The thesis underneath

The bible's "AI is a field in incant, not a tool called from incant" line is a thesis statement about the whole architecture, not just a feature spec. It says: in this system, reasoning is a first-class participant, not a service. Same posture as Tony's overnight question — *can I tease an AI, and if yes, can the AI tease back?* — restated at the language-design level.

If we mean the thesis, we should say so plainly somewhere — probably in the bible, once we've worked out enough of points 1-4 to know what we mean. If we don't mean it, we should soften the bible line so it doesn't write a check the architecture can't cash.

### Session notes

- Tony's call: start at #1 (operational semantics).
- Claude's instinct: same — the others depend on it. (Alternative was #5 sky-down; Tony picked #1.)
- Clod woken only to write this file (after a Clay-app freeze and conversation recovery from scratch); parlay continues.

---

## Glossary additions pending

Terms used in HWF that may want bible/glossary promotion once stable:

- *cha cha* — the working dance between Tony, Claude, and Clod. Currently informal.
- *isCLAUDE* — provisional name for an AI-as-field-type GroupItem kind. May survive into the implementation; may get renamed.
- *trim* — the four-section session record. Replaces, doesn't append.

---

## Sessions index

1. isCLAUDE and the cha cha — open — 2026-05-06
