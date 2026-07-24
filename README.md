# ECL-BF-Bridge

**v1.2.6**

Attach and operate the **Electric CodeLock** on **Building Fortifications**
barricade doors (single and double). This is a standalone compatibility
bridge: it contains no code or assets from either mod and requires both as
separate subscriptions.

## What it does out of the box

Out of the box the bridge mirrors upstream Electric CodeLock vanilla-style
DayZ behaviour faithfully. By default a coded door requires the full
sequence on every pass: enter the PIN on the keypad, unlock, open the door,
close it, then lock it again. Everything below is disabled until the server
owner enables it in `profiles/EclBfBridge/settings.json` (created with
defaults on first server start).

## Optional QoL features (OFF by default, require configuration)

| Setting | Effect |
|---|---|
| `quick_access_known_code` | A player who has entered a lock's PIN once can press-F for quick access (keypad beeps out digits as if they were entered). `1` = unlock only (the prompt reads Unlock), `2` = unlock and open in one action (the prompt reads Unlock & Open). Access is per-lock, persisted and wiped when the code changes. |
| `auto_lock_on_close` | A coded lock re-engages automatically when the door closes. |
| `auto_lock_on_code_set` | Setting or re-keying a code on a closed door locks it immediately. |
| `keypad_open_after_attach` | Attaching a codeless lock opens the keypad straight into set-PIN mode. |
| `replace_shock_with_cooldown` | Replaces the wrong-code electric shock with a keypad lockout (`wrong_code_cooldown_ms`, default 5000). |
| `screwdriver_detach_open_door` | A screwdriver detaches an unlocked lock from an open door. |
| `require_fully_built` | The door must be fully built (planked on both faces plus hinged) before the code lock will attach (highly recommended). |
| `mask_code_display` | PIN privacy. By default every entered digit is shown live on the lock's LCD display for anyone watching (and stays readable for up to 30 seconds if entry is abandoned). With this on, no one ever sees the digits: bystanders see solid blocks on the lock, the typing player sees one `*` per keypress on the keypad and the PIN never leaves the typing player's client except in the final submit to the server. Exception: while setting or changing a PIN, the typing player sees the digits they enter (so they can be certain of the code they set); everyone else still sees blocks. |
| `log_lock_events` | Audit logging for lock lifecycle events (attach, detach, code set, admin code set, lock), with player name, SteamID and position. PIN values are never logged. |
| `log_entry_events` | Audit logging for entry attempts (unlock, failed entry, cooldown trigger, attempts refused during a cooldown). A wrong PIN that starts a cooldown logs both the failed entry and the cooldown trigger. |

## Logging and auditing

Logging is off by default: the bridge writes nothing until you turn on
`log_lock_events` or `log_entry_events` above. Those decide whether an
event is recorded; two further settings decide where each line goes, and
they work independently so you can use either, both or neither. PIN values
are never written to either destination.

| Setting | Effect |
|---|---|
| `log_to_admin_log` | Default on. Writes to the vanilla DayZ admin log (`.ADM`), the standard player-action audit surface. The server must run with `-adminlog` or the file is never produced. |
| `log_to_daily_log` | Default off. Opt-in isolated copy: one file per day under `profiles/EclBfBridge/logs/` carrying only this mod's lines, useful for keeping bridge activity separate from the `.ADM` stream. |

## Tested against

- Electric CodeLock: Workshop update of 2026-05-04
- Building Fortifications: Workshop update of 2025-09-01

An upstream update to either mod can change the surfaces this bridge relies
on. If lock behaviour breaks after an upstream update, check the Workshop
page: the combination above is the last one verified on a live server.

## Install

Subscribe on the Steam Workshop (recommended for players and most servers):

https://steamcommunity.com/sharedfiles/filedetails/?id=3766801953

## Build

Built with [HEMTT](https://github.com/BrettMayson/HEMTT). From the mod root:

```
hemtt release
```

Notes:

- The build binds against the two upstream mods (`BM_CodeLock` /
  `BM_CodeLock_Client` and `Building Fortifications`). Their class surfaces
  must be present for the modded classes to resolve.
- Signing uses the `Bushy` authority. A clone does not carry that key, so
  build unsigned for local testing or supply your own authority in
  `.hemtt/project.toml`.

## Status

This repository is a one-way, automatically published mirror of the mod
shipped on the Steam Workshop. It does not accept issues or pull requests.
Fork it if you want to build on it, within the license terms below.

Version history is in [CHANGELOG.md](CHANGELOG.md).

## Credits and acknowledgements

This mod is just the glue: the real work lives in the two mods below. Huge
thanks to their authors for building the mechanics this bridge leans on.

- Breathe and Cookup: Electric CodeLock, the lock this bridge exists for.
- Dumpgrah: Building Fortifications, the doors it locks.

## License

Creative Commons Attribution-NonCommercial 4.0 International (CC BY-NC 4.0).
Modify, repack and fork for non-commercial use with attribution. See
[LICENSE](LICENSE). This mod ships only Bullets'n'Bandages-authored script
layers; the Electric CodeLock and Building Fortifications remain their
authors' work under their own terms.

DAYZ is a registered trademark of Bohemia Interactive a.s. This is an
unofficial modification that is not affiliated or authorized by Bohemia
Interactive a.s.
