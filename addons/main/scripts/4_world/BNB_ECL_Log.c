// Organisation: Bullets'n'Bandages
// Author:       Bushy <contact@bushy.dev>
// Version:      v1.0.1
// Modified:     2026-07-18
//
// BNB_ECL_Log.c - opt-in audit logging for lock lifecycle and entry attempts.
// Server-side only; lines go to a per-day file under $profile:EclBfBridge\logs
// and mirror to the vanilla admin log (.ADM, active only with -adminlog).
// PIN values are never logged.

class BNB_EclBridgeLog
{
    protected static const string BASE_DIR = "$profile:EclBfBridge";
    protected static const string LOG_DIR = "$profile:EclBfBridge\\logs";

    static void Lifecycle(string evt, EntityAI lock, EntityAI parent, PlayerIdentity actor)
    {
        if (!BNB_EclBridgeSettings.LogLockEvents())
            return;
        Emit(evt, lock, parent, actor);
    }

    static void Entry(string evt, EntityAI lock, EntityAI parent, PlayerIdentity actor)
    {
        if (!BNB_EclBridgeSettings.LogEntryEvents())
            return;
        Emit(evt, lock, parent, actor);
    }

    // Lifecycle variant taking a pre-formatted actor tag (deferred-drop paths
    // where the identity is gone by the time the event lands).
    static void LifecycleWho(string evt, EntityAI lock, EntityAI parent, string who)
    {
        if (!BNB_EclBridgeSettings.LogLockEvents())
            return;
        EmitWho(evt, lock, parent, who);
    }

    protected static void Emit(string evt, EntityAI lock, EntityAI parent, PlayerIdentity actor)
    {
        string who = "-";
        if (actor)
            who = actor.GetName() + "(" + actor.GetPlainId() + ")";
        EmitWho(evt, lock, parent, who);
    }

    protected static void EmitWho(string evt, EntityAI lock, EntityAI parent, string who)
    {
        if (!GetGame() || !GetGame().IsServer())
            return;
        if (who == "")
            who = "-";
        string lockType = "-";
        vector pos = "0 0 0";
        if (lock)
        {
            lockType = lock.GetType();
            pos = lock.GetPosition();
        }
        string parentType = "-";
        if (parent)
        {
            parentType = parent.GetType();
            pos = parent.GetPosition();
        }
        int px = Math.Round(pos[0]);
        int pz = Math.Round(pos[2]);
        string line = Stamp() + " [EclBfBridge] " + evt + " lock=" + lockType + " door=" + parentType + " pos=" + px.ToString() + " " + pz.ToString() + " player=" + who;
        WriteFile(line);
        GetGame().AdminLog(line);
    }

    protected static void WriteFile(string line)
    {
        if (!FileExist(BASE_DIR))
            MakeDirectory(BASE_DIR);
        if (!FileExist(LOG_DIR))
            MakeDirectory(LOG_DIR);
        int year;
        int month;
        int day;
        GetYearMonthDay(year, month, day);
        string path = LOG_DIR + "\\ECL-BF-Bridge_" + year.ToString() + "-" + Pad2(month) + "-" + Pad2(day) + ".log";
        FileHandle fh = OpenFile(path, FileMode.APPEND);
        if (fh == 0)
            return;
        FPrintln(fh, line);
        CloseFile(fh);
    }

    protected static string Stamp()
    {
        int year;
        int month;
        int day;
        int hour;
        int minute;
        int second;
        GetYearMonthDay(year, month, day);
        GetHourMinuteSecond(hour, minute, second);
        return year.ToString() + "-" + Pad2(month) + "-" + Pad2(day) + " " + Pad2(hour) + ":" + Pad2(minute) + ":" + Pad2(second);
    }

    protected static string Pad2(int v)
    {
        if (v < 10)
            return "0" + v.ToString();
        return v.ToString();
    }
}
