# Changelog

Notable changes to ECL-BF-Bridge. Versions here follow the Steam Workshop
releases. Source history in this repository starts at v1.2.3; not every Workshop
release is tagged here, and earlier entries are recorded for reference.

## v1.2.6

- Log destinations are now independently selectable. Two settings decide
  where audited events are written: `log_to_admin_log` (default on) sends
  lines to the vanilla DayZ admin log (`.ADM`), and `log_to_daily_log`
  (default off) writes an isolated per-day copy under
  `profiles/EclBfBridge/logs/` carrying only this mod's lines. Use either,
  both or neither. The `log_lock_events` and `log_entry_events` switches
  still decide whether an event is logged at all and stay off by default.
- The admin panel Open and Close Gate actions now operate on Building
  Fortifications doors instead of doing nothing.
- Fix: an admin Open on a Building Fortifications element with no hinges no
  longer leaves it stuck open and refusing every later code-lock attach.

## v1.2.5

- A licence file is bundled with the mod: CC BY-NC 4.0 terms, the DayZ
  trademark notice required of unofficial modifications and a statement that
  Electric CodeLock and Building Fortifications remain separate mods under
  their own authors' terms.

## v1.2.4

- Player names are sanitised before being written to the log: newlines,
  carriage returns and tabs stripped, length capped at 96 characters, so a
  crafted profile name cannot forge a log line.
- Fix: pending timers are cancelled when a lock is detached or its parent is
  destroyed, so a queued cooldown clear can no longer fire against a lock that
  has since been re-attached.
- Fix: detaching refuses to drop a lock that is still locked.
- Save safety: an unknown or newer persistence stream version is skipped rather
  than misread.

## v1.2.3

- Workshop page only: added a link to the GitHub source. No code change.

## v1.2.2

- Fix: the settings payload is delivered correctly over the network.
- Fix: guard against restoring state before initialisation completes.
- Logging: door-to-door attach and detach are audited, and the pending detach
  record is always cleared.

## v1.2.1

- Fix: client-side settings are applied correctly.
- Logging: attach and detach are audited across every inventory path, not just
  hand-to-door, so dragging a lock is now recorded.
- Logging: locks dropped to the ground are attributed to the player who
  detached them.
- Logging: new cooldown-blocked event, and unlock failures follow the upstream
  parent-support check.
- With `mask_code_display` on, the player setting a PIN now sees the real
  digits while setting it, so they can confirm the code. Bystanders still see
  blocks, and code entry still shows asterisks.

## v1.2.0

- Renamed the setting `quick_unlock_known_code` to `quick_access_known_code`
  and changed it from on/off to a mode: `0` off, `1` unlock only, `2` unlock
  and open in one action. Existing settings files are migrated by dropping the
  old key on load.
- Mode `2` opens the door after unlocking it.
- The action text follows the mode, reading "Unlock" or "Unlock & Open".

## v1.1.1

- Fix: v1.1.0 crash-looped the server on load because of a reserved keyword in
  the new logging code. v1.1.1 is the first working audit-logging build; skip
  v1.1.0.

## v1.1.0

- New opt-in audit logging, both options off by default: `log_lock_events`
  (attach, detach, code set, admin code set, lock) and `log_entry_events`
  (unlock, failed unlock, cooldown).
- Logs are written to a per-day file under `$profile:EclBfBridge\logs\` and
  mirrored to the server admin log when the server runs with `-adminlog`.
  Entries carry player name, ID and position. PIN values are never logged.
- Settings are re-saved after load, so new options from an update appear
  automatically on existing installs.
- Known defect: this build crash-loops on load. Fixed in v1.1.1.

## v1.0.7

- `require_fully_built` default reverted to `0`, so every option is off by
  default and behaviour matches upstream out of the box. Turning it on is
  still recommended.

## v1.0.6

- Fix: in masked mode, the admin change-code buffer was not reset between uses.

## v1.0.5

- With `mask_code_display` on, the player typing sees one asterisk per
  keypress. Bystanders still see solid blocks on the lock.

## v1.0.4

- Internal file renaming only. No behaviour change.

## v1.0.3

- Removed the startup self-heal for stranded lock slots added in v1.0.2. The
  stranded state can only be created by pre-v1.0.2 code, so no live server can
  reach it.

## v1.0.2

- Fix (headline): re-attaching a lock to a Building Fortifications door was
  refused. Any path that removed a lock left the door's attachment slot
  stranded as locked-but-empty, blocking every later attach. The slot is now
  released whenever a lock leaves.
- New `mask_code_display` option (default off). Upstream broadcasts each typed
  digit to every nearby client and renders it on the lock, readable for around
  30 seconds if entry is abandoned. With masking on, bystanders see solid
  blocks and the real code travels only from the typing client to the server.
- `require_fully_built` default changed to `1`.

## v1.0.1

- Renamed the mod to `ECL-BF-Bridge` so the DayZ launcher resolves it from the
  server mod list, plus Workshop page wording.

## v1.0.0

- First release. Electric CodeLock can be attached to and operated on Building
  Fortifications barricade doors, single and double: attach, set a code, enter
  the code, lock and detach. A locked door hides the native open action so the
  keypad is the only way in. Dismantling or destroying the gate drops the
  attached lock intact.
- All quality-of-life behaviour is gated behind
  `$profile:EclBfBridge\settings.json`, every option off by default, so out of
  the box the mod behaves exactly like upstream.
