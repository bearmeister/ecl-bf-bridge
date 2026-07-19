// Organisation: Bullets'n'Bandages
// Author:       Bushy <contact@bushy.dev>
// Version:      v1.1.0
// Modified:     2026-07-18
//
// BNB_ECL_KnownCode.c - known-code quick access for the Electric CodeLock on
// Building Fortifications doors. OFF by default: quick_access_known_code in
// $profile:EclBfBridge\settings.json (1 = unlock only, 2 = unlock + open).
// A player who has entered a lock's PIN once knows it; thereafter press-F
// types it for them (keypad beeps). Changing the PIN wipes all knowledge.

#ifdef BM_CodeLock

// Client-side single-entry cache of "does my character know this lock's code"
// (server answers via RPC). Unknown reads as NO so the keypad stays the fallback.
class BNB_ECL_KnownCache
{
    private static BM_CodeLock s_Lock;
    private static int s_Known = -1;
    private static int s_ValidUntilMs = 0;
    private static int s_NextQueryMs = 0;

    // Answers age out after 5s so knowledge changes show within one refresh;
    // the stale answer serves meanwhile so the verb does not flicker.
    static bool IsKnown(PlayerBase player, BM_CodeLock lock)
    {
        if (!player || !lock)
            return false;
        int now = GetGame().GetTime();
        bool cachedForThisLock = (s_Lock == lock && s_Known >= 0);
        if (cachedForThisLock && now < s_ValidUntilMs)
            return s_Known == 1;
        if (now >= s_NextQueryMs)
        {
            s_NextQueryMs = now + 2000;
            if (!cachedForThisLock)
            {
                s_Lock = lock;
                s_Known = -1;
            }
            GetGame().RPCSingleParam(lock, BNB_EclBridgeRPCs.RPC_BNB_ECL_KNOWN_QUERY, null, true);
        }
        if (cachedForThisLock)
            return s_Known == 1;
        return false;
    }

    static void SetKnown(BM_CodeLock lock, bool known)
    {
        s_Lock = lock;
        s_ValidUntilMs = GetGame().GetTime() + 5000;
        if (known)
            s_Known = 1;
        else
            s_Known = 0;
    }
}

// Press-F on a locked, closed door whose code your character already knows:
// mode 1 unlocks (a second press opens); mode 2 unlocks and opens in one go.
class ActionBNBQuickUnlock extends ActionInteractBase
{
    void ActionBNBQuickUnlock()
    {
        m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_INTERACTONCE;
        m_StanceMask = DayZPlayerConstants.STANCEMASK_CROUCH | DayZPlayerConstants.STANCEMASK_ERECT;
        m_Text = "Unlock";
    }

    // The prompt mirrors what the action will actually do under the mode.
    override string GetText()
    {
        if (BNB_EclBridgeSettings.QuickAccessMode() >= 2)
            return "Unlock & Open";
        return "Unlock";
    }

    override void CreateConditionComponents()
    {
        m_ConditionItem = new CCINone;
        m_ConditionTarget = new CCTCursor;
    }

    #ifdef BuildingFortifications
    protected BuildingFortficationsCore BNB_ResolveDoor(ActionTarget target)
    {
        return BuildingFortficationsCore.Cast(target.GetObject());
    }

    protected BM_CodeLock BNB_ResolveLock(BuildingFortficationsCore door)
    {
        if (!door)
            return null;
        return BM_CodeLock.Cast(door.FindAttachmentBySlotName("Att_CombinationLock"));
    }
    #endif

    override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
    {
        #ifdef BuildingFortifications
        if (!BNB_EclBridgeSettings.QuickAccess())
            return false;
        BuildingFortficationsCore door = BNB_ResolveDoor(target);
        BM_CodeLock lock = BNB_ResolveLock(door);
        if (!lock)
            return false;
        if (!lock.IsLocked())
            return false;
        if (lock.BNB_IsCoolingDown())
            return false;
        if (door.IsOpened())
            return false;
        if (!g_Game.IsDedicatedServer())
            return BNB_ECL_KnownCache.IsKnown(player, lock);
        PlayerIdentity id = player.GetIdentity();
        if (!id)
            return false;
        return lock.BNB_KnowsCode(id.GetPlainId());
        #endif
        return false;
    }

    override void OnStartServer(ActionData action_data)
    {
        super.OnStartServer(action_data);
        #ifdef BuildingFortifications
        BuildingFortficationsCore door = BNB_ResolveDoor(action_data.m_Target);
        BM_CodeLock lock = BNB_ResolveLock(door);
        if (!lock)
            return;
        lock.BNB_QuickAccessUnlock(action_data.m_Player.GetIdentity());
        #endif
    }
}

modded class ActionConstructor
{
    override void RegisterActions(TTypenameArray actions)
    {
        super.RegisterActions(actions);
        actions.Insert(ActionBNBQuickUnlock);
    }
}
#endif
