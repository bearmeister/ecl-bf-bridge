// Organisation: Bullets'n'Bandages
// Author:       Bushy <contact@bushy.dev>
// Version:      v1.2.2
// Modified:     2026-07-23
//
// BNB_ECL_Settings.c - server-owner settings for the ECL BF Bridge.
// Every feature flag defaults to 0 (OFF): out of the box the bridge only adds
// Building Fortifications door support to the Electric CodeLock, mirroring
// upstream behaviour. The two sink selectors are the exception - the admin-log
// sink defaults to 1, since it is the standard audit surface and is inert
// unless a master log flag is also on. Configure in
// $profile:EclBfBridge\settings.json (created with defaults on first boot).

// JSON data carrier - member names bind 1:1 to the settings file keys.
class BNB_EclBridgeSettingsData
{
    // Press-F access for players who have entered this lock's PIN before:
    // 0 = off, 1 = unlock only, 2 = unlock + open in one action.
    int quick_access_known_code = 0;
    // A coded lock re-engages automatically when the door closes.
    int auto_lock_on_close = 0;
    // Setting or re-keying a code on a closed door locks it immediately.
    int auto_lock_on_code_set = 0;
    // Attaching a codeless lock opens the keypad straight into set-PIN mode.
    int keypad_open_after_attach = 0;
    // Replace the upstream wrong-code electric shock with a keypad cooldown.
    int replace_shock_with_cooldown = 0;
    int wrong_code_cooldown_ms = 5000;
    // Allow screwdriver detach of an unlocked lock on an open door.
    int screwdriver_detach_open_door = 0;
    // Hide typed PIN digits from other players: the lock's display shows
    // blocks and the real code never leaves the server or the typing client.
    int mask_code_display = 0;
    // Require a fully planked + hinged door before a lock can attach;
    // OFF follows BF's native hinge-only gating.
    int require_fully_built = 0;
    // Audit logging master switches - whether an event is logged at all.
    // PIN values are never logged.
    int log_lock_events = 0;
    int log_entry_events = 0;
    // Sink selectors - where a logged event goes.
    // Vanilla admin log (.ADM), active only with -adminlog.
    int log_to_admin_log = 1;
    // Per-day file under $profile:EclBfBridge\logs\.
    int log_to_daily_log = 0;
}

// Read API. Server fills from disk via the 4_World loader; clients receive a
// copy over RPC on connect and hold the safe defaults until it arrives.
class BNB_EclBridgeSettings
{
    protected static ref BNB_EclBridgeSettingsData s_Data = new BNB_EclBridgeSettingsData();

    static BNB_EclBridgeSettingsData Data()
    {
        return s_Data;
    }

    static void Apply(BNB_EclBridgeSettingsData data)
    {
        if (data)
            s_Data = data;
    }

    static bool QuickAccess()
    {
        return s_Data.quick_access_known_code != 0;
    }

    static int QuickAccessMode()
    {
        return s_Data.quick_access_known_code;
    }

    static bool AutoLockOnClose()
    {
        return s_Data.auto_lock_on_close != 0;
    }

    static bool AutoLockOnCodeSet()
    {
        return s_Data.auto_lock_on_code_set != 0;
    }

    static bool KeypadOpenAfterAttach()
    {
        return s_Data.keypad_open_after_attach != 0;
    }

    static bool ReplaceShockWithCooldown()
    {
        return s_Data.replace_shock_with_cooldown != 0;
    }

    static int WrongCodeCooldownMs()
    {
        if (s_Data.wrong_code_cooldown_ms < 500)
            return 500;
        return s_Data.wrong_code_cooldown_ms;
    }

    static bool ScrewdriverDetachOpenDoor()
    {
        return s_Data.screwdriver_detach_open_door != 0;
    }

    static bool RequireFullyBuilt()
    {
        return s_Data.require_fully_built != 0;
    }

    static bool MaskCodeDisplay()
    {
        return s_Data.mask_code_display != 0;
    }

    static bool LogLockEvents()
    {
        return s_Data.log_lock_events != 0;
    }

    static bool LogEntryEvents()
    {
        return s_Data.log_entry_events != 0;
    }

    static bool LogToAdminLog()
    {
        return s_Data.log_to_admin_log != 0;
    }

    static bool LogToDailyLog()
    {
        return s_Data.log_to_daily_log != 0;
    }
}
