// Organisation: Bullets'n'Bandages
// Author:       Bushy <contact@bushy.dev>
// Version:      v1.3.2
// Modified:     2026-07-18
//
// BNB_ECL_BFSupport.c - Electric CodeLock support for Building Fortifications
// doors. Single modded BM_CodeLock block: extends the lock's supported-parent
// set to BF doors, persists which players know the current PIN (entered it
// once), hooks the code lifecycle, and carries the optional wrong-code keypad
// cooldown, PIN display masking and audit logging. QoL behaviour is
// settings-gated and OFF by default; the known-PIN list is persisted
// unconditionally so saves keep one stable layout.

#ifdef BM_CodeLock
modded class BM_CodeLock
{
    // Stream v1 carried a battery bool between the sentinel and the guid list;
    // v2 dropped it. Loader accepts both.
    protected const int BNB_ECL_STREAM_VERSION = 2;
    protected const int BNB_ECL_KNOWN_CAP = 10;
    protected const int BNB_ECL_QUICK_BEEPS = 6;
    protected const int BNB_ECL_BEEP_GAP_MS = 200;

    // A quick-unlock beep sequence is in flight; refuse re-triggers meanwhile.
    protected bool m_BNB_QuickUnlockPending = false;

    // Plain SteamIDs of players who have entered the CURRENT code.
    protected ref array<string> m_BNB_KnownGuids = new array<string>;

    // Wrong-code cooldown: keypad + quick-unlock refuse while this is set.
    protected bool m_BNB_CoolingDown = false;

    void BM_CodeLock()
    {
        RegisterNetSyncVariableBool("m_BNB_CoolingDown");
    }

    bool BNB_IsCoolingDown()
    {
        return m_BNB_CoolingDown;
    }

    protected void BNB_StartCodeLockCooldown()
    {
        m_BNB_CoolingDown = true;
        SetSynchDirty();
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(BNB_EndCodeLockCooldown, BNB_EclBridgeSettings.WrongCodeCooldownMs(), false);
    }

    protected void BNB_EndCodeLockCooldown()
    {
        m_BNB_CoolingDown = false;
        SetSynchDirty();
    }

    bool BNB_KnowsCode(string guid)
    {
        if (guid == "")
            return false;
        return m_BNB_KnownGuids.Find(guid) > -1;
    }

    // Bounded FIFO: the oldest entry drops once the cap is reached.
    protected void BNB_RememberCode(string guid)
    {
        if (guid == "")
            return;
        if (m_BNB_KnownGuids.Find(guid) > -1)
            return;
        m_BNB_KnownGuids.Insert(guid);
        if (m_BNB_KnownGuids.Count() > BNB_ECL_KNOWN_CAP)
            m_BNB_KnownGuids.RemoveOrdered(0);
    }

    protected void BNB_ForgetCodes()
    {
        m_BNB_KnownGuids.Clear();
    }

    // Quick access beeps once per PIN digit (broadcast), then rides the native
    // try-code chain so the upstream success RPC, sounds and audit hooks fire.
    void BNB_QuickAccessUnlock(PlayerIdentity sender)
    {
        if (!GetGame().IsServer())
            return;
        if (!BNB_EclBridgeSettings.QuickAccess())
            return;
        if (!sender)
            return;
        if (m_BNB_CoolingDown || m_BNB_QuickUnlockPending)
            return;
        if (!IsLocked())
            return;
        if (!BNB_KnowsCode(sender.GetPlainId()))
            return;
        m_BNB_QuickUnlockPending = true;
        BNB_QuickUnlockBeepTick(sender, 0);
    }

    // One beep per digit; the unlock lands one gap after the last beep. State
    // re-checked at fire time (a wrong-code cooldown mid-sequence aborts).
    protected void BNB_QuickUnlockBeepTick(PlayerIdentity sender, int step)
    {
        if (step < BNB_ECL_QUICK_BEEPS)
        {
            GetGame().RPCSingleParam(this, BNB_EclBridgeRPCs.RPC_BNB_ECL_BEEP, new Param1<bool>(true), true, null);
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(BNB_QuickUnlockBeepTick, BNB_ECL_BEEP_GAP_MS, false, sender, step + 1);
            return;
        }
        m_BNB_QuickUnlockPending = false;
        // Knowledge re-checked at fire time: a mid-sequence re-key wipes the
        // guid list and must abort the pending unlock.
        if (IsLocked() && !m_BNB_CoolingDown && sender && BNB_KnowsCode(sender.GetPlainId()))
        {
            HandleTryCode(sender, m_PassCode);
            if (BNB_EclBridgeSettings.QuickAccessMode() >= 2 && !IsLocked())
                BNB_QuickOpenDoor();
        }
    }

    // Mode 2: the unlocked door swings open in the same action.
    protected void BNB_QuickOpenDoor()
    {
        #ifdef BuildingFortifications
        BuildingFortficationsCore door = BuildingFortficationsCore.Cast(GetHierarchyParent());
        if (door && !door.IsOpened() && door.CanOpenFence())
            door.OpenFence();
        #endif
    }

    // mask_code_display: only masked digits reach the synced display vars or
    // the display-sync broadcast; the confirm RPCs alone carry the real code.
    protected string BNB_MaskTyped(string code)
    {
        string masked = "";
        for (int mi = 0; mi < code.Length(); mi++)
        {
            masked += "0";
        }
        return masked;
    }

    override void SetTypedCode(string code)
    {
        if (BNB_EclBridgeSettings.MaskCodeDisplay())
            code = BNB_MaskTyped(code);
        super.SetTypedCode(code);
    }

    override protected void SyncTypedDisplay(string code)
    {
        if (BNB_EclBridgeSettings.MaskCodeDisplay())
            code = BNB_MaskTyped(code);
        super.SyncTypedDisplay(code);
    }

    // Masked world display renders solid blocks instead of digit glyphs.
    override protected string GetDigitTextureByValue(int digit)
    {
        if (BNB_EclBridgeSettings.MaskCodeDisplay())
            return "#(argb,8,8,3)color(0,0,0,1,CA)";
        return super.GetDigitTextureByValue(digit);
    }

    // Optional replacement of the upstream 2-strike electric shock with a
    // keypad cooldown; upstream shock behaviour is the default.
    override protected bool RegisterFailedAttempt(PlayerIdentity sender)
    {
        if (!BNB_EclBridgeSettings.ReplaceShockWithCooldown())
            return super.RegisterFailedAttempt(sender);
        return false;
    }

    // Attach/detach audit rides the location change: any move onto/off a base
    // element logs; a door-to-door drag logs the detach AND the attach.
    override void OnItemLocationChanged(EntityAI old_owner, EntityAI new_owner)
    {
        super.OnItemLocationChanged(old_owner, new_owner);
        if (!GetGame().IsServer())
            return;
        // Persistence restore re-fires this pre-init; only live moves log.
        if (!IsInitialized())
            return;
        if (old_owner && old_owner.IsInherited(BaseBuildingBase))
        {
            // Consumed unconditionally so a stale record never misattributes.
            string recordedActor = BNB_ConsumeDetachActor();
            PlayerIdentity detachActor = BNB_IdentityOf(new_owner);
            if (detachActor)
            {
                BNB_EclBridgeLog.Lifecycle("detach", this, old_owner, detachActor);
            }
            else
            {
                BNB_EclBridgeLog.LifecycleWho("detach", this, old_owner, recordedActor);
            }
        }
        if (new_owner && new_owner.IsInherited(BaseBuildingBase))
            BNB_EclBridgeLog.Lifecycle("attach", this, new_owner, BNB_IdentityOf(old_owner));
    }

    protected PlayerIdentity BNB_IdentityOf(EntityAI ent)
    {
        PlayerBase pb = PlayerBase.Cast(ent);
        if (pb)
            return pb.GetIdentity();
        return null;
    }

    protected string m_BNB_DetachActorWho = "";
    protected float m_BNB_DetachActorTime = 0;

    // The detach action records its actor just before the drop lands.
    void BNB_RecordDetachActor(PlayerIdentity actor)
    {
        if (!actor)
            return;
        m_BNB_DetachActorWho = actor.GetName() + "(" + actor.GetPlainId() + ")";
        m_BNB_DetachActorTime = GetGame().GetTickTime();
    }

    protected string BNB_ConsumeDetachActor()
    {
        string who = m_BNB_DetachActorWho;
        float when = m_BNB_DetachActorTime;
        m_BNB_DetachActorWho = "";
        if (who == "")
            return "-";
        if (GetGame().GetTickTime() - when > 10.0)
            return "-";
        return who;
    }

    override void LockCode(PlayerIdentity sender = null)
    {
        bool bnbWasLocked = IsLocked();
        super.LockCode(sender);
        if (GetGame().IsServer() && !bnbWasLocked && IsLocked())
            BNB_EclBridgeLog.Lifecycle("lock", this, GetHierarchyParent(), sender);
    }

    override protected bool BM_SV_IsSupportedAttachmentParent(EntityAI parent)
    {
        if (super.BM_SV_IsSupportedAttachmentParent(parent))
            return true;
        #ifdef BuildingFortifications
        if (parent && parent.IsInherited(BuildingFortficationsCore))
            return true;
        #endif
        return false;
    }

    override protected bool BM_IsGateOpenedForParent(EntityAI parent)
    {
        #ifdef BuildingFortifications
        BuildingFortficationsCore door = BuildingFortficationsCore.Cast(parent);
        if (door)
            return door.IsOpened();
        #endif
        return super.BM_IsGateOpenedForParent(parent);
    }

    override void OnStoreSave(ParamsWriteContext ctx)
    {
        super.OnStoreSave(ctx);
        ctx.Write(BNB_ECL_STREAM_VERSION);
        ctx.Write(m_BNB_KnownGuids.Count());
        for (int i = 0; i < m_BNB_KnownGuids.Count(); i++)
        {
            ctx.Write(m_BNB_KnownGuids.Get(i));
        }
    }

    // Trailing append behind a version sentinel; a save without it (upstream
    // layout) loads with defaults rather than failing the whole entity.
    override bool OnStoreLoad(ParamsReadContext ctx, int version)
    {
        if (!super.OnStoreLoad(ctx, version))
            return false;
        int bnbVer;
        if (!ctx.Read(bnbVer))
            return true;
        if (bnbVer == 1)
        {
            bool legacyBattery;
            if (!ctx.Read(legacyBattery))
                return true;
        }
        int count;
        if (!ctx.Read(count))
            return true;
        if (count < 0 || count > BNB_ECL_KNOWN_CAP)
            return true;
        m_BNB_KnownGuids.Clear();
        for (int i = 0; i < count; i++)
        {
            string guid;
            if (!ctx.Read(guid))
                return true;
            m_BNB_KnownGuids.Insert(guid);
        }
        return true;
    }

    // A successful PIN entry proves knowledge of the current code; a wrong
    // PIN shuts the keypad down for the cooldown when that feature is on.
    override protected void HandleTryCode(PlayerIdentity sender, string code)
    {
        if (GetGame().IsServer() && m_BNB_CoolingDown)
        {
            // An attempt landing mid-cooldown still gets the denied feedback.
            if (sender)
                SendSimpleRPC(BM_CODELOCK_RPC_FAIL, sender);
            BNB_EclBridgeLog.Entry("cooldown_blocked", this, GetHierarchyParent(), sender);
            return;
        }
        bool wasLocked = IsLocked();
        bool bnbWrongCode = GetGame().IsServer() && BNB_EclBridgeSettings.ReplaceShockWithCooldown() && sender && m_HasCode && m_IsLocked && IsValidCode(code) && code != m_PassCode && BM_SV_IsSupportedAttachmentParent(GetHierarchyParent());
        // Audit predicate: a wrong PIN on a locked lock, mirroring upstream's
        // parent-support guard so ignored attempts on stranded locks stay out.
        bool bnbFailedEntry = GetGame().IsServer() && sender && m_HasCode && m_IsLocked && IsValidCode(code) && code != m_PassCode && BM_SV_IsSupportedAttachmentParent(GetHierarchyParent());
        super.HandleTryCode(sender, code);
        if (wasLocked && !IsLocked())
        {
            BNB_OnUnlockSuccess(sender);
            BNB_EclBridgeLog.Entry("unlock", this, GetHierarchyParent(), sender);
        }
        if (bnbFailedEntry)
            BNB_EclBridgeLog.Entry("unlock_fail", this, GetHierarchyParent(), sender);
        if (bnbWrongCode)
        {
            BNB_StartCodeLockCooldown();
            BNB_EclBridgeLog.Entry("cooldown", this, GetHierarchyParent(), sender);
        }
    }

    protected void BNB_OnUnlockSuccess(PlayerIdentity sender)
    {
        if (!sender)
            return;
        if (!BNB_EclBridgeSettings.QuickAccess())
            return;
        BNB_RememberCode(sender.GetPlainId());
        // Push the known-bit so the quick-unlock verb appears without waiting
        // for the client cache to age out.
        GetGame().RPCSingleParam(this, BNB_EclBridgeRPCs.RPC_BNB_ECL_KNOWN_RESP, new Param1<bool>(true), true, sender);
    }

    // On an actual code change everyone forgets the old code except the setter.
    override protected void HandleCreateCode(PlayerIdentity sender, string code)
    {
        string oldCode = m_PassCode;
        super.HandleCreateCode(sender, code);
        if (m_PassCode != oldCode)
        {
            BNB_EclBridgeLog.Lifecycle("code_set", this, GetHierarchyParent(), sender);
            BNB_ForgetCodes();
            if (sender && BNB_EclBridgeSettings.QuickAccess())
            {
                BNB_RememberCode(sender.GetPlainId());
                GetGame().RPCSingleParam(this, BNB_EclBridgeRPCs.RPC_BNB_ECL_KNOWN_RESP, new Param1<bool>(true), true, sender);
            }
            BNB_AutoLockOnBFDoor();
        }
    }

    // The ECL admin tool re-keys via its own path - same forget-on-change rules.
    override protected void HandleAdminSetCode(PlayerIdentity sender, string code)
    {
        string oldAdminCode = m_PassCode;
        super.HandleAdminSetCode(sender, code);
        if (m_PassCode != oldAdminCode)
        {
            BNB_EclBridgeLog.Lifecycle("admin_code_set", this, GetHierarchyParent(), sender);
            BNB_ForgetCodes();
            if (sender && BNB_EclBridgeSettings.QuickAccess())
            {
                BNB_RememberCode(sender.GetPlainId());
                GetGame().RPCSingleParam(this, BNB_EclBridgeRPCs.RPC_BNB_ECL_KNOWN_RESP, new Param1<bool>(true), true, sender);
            }
            BNB_AutoLockOnBFDoor();
        }
    }

    // Optional: a fresh code on a closed BF door locks immediately, so setting
    // a PIN and walking away never leaves the door open to anyone.
    protected void BNB_AutoLockOnBFDoor()
    {
        if (!GetGame().IsServer())
            return;
        if (!BNB_EclBridgeSettings.AutoLockOnCodeSet())
            return;
        #ifdef BuildingFortifications
        EntityAI bfParent = GetHierarchyParent();
        if (!bfParent || !bfParent.IsInherited(BuildingFortficationsCore))
            return;
        if (m_HasCode && !IsLocked() && !BM_IsGateOpenedForParent(bfParent))
            LockCode(null);
        #endif
    }

    override void PrepareForDetach()
    {
        BNB_ForgetCodes();
        super.PrepareForDetach();
    }

    override void OnParentDestroyed()
    {
        if (GetGame().IsServer())
            BNB_ForgetCodes();
        super.OnParentDestroyed();
    }

    // Known-code query: client asks, server answers to that client only.
    override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
    {
        super.OnRPC(sender, rpc_type, ctx);

        if (rpc_type == BNB_EclBridgeRPCs.RPC_BNB_ECL_KNOWN_QUERY && GetGame().IsServer() && sender)
        {
            bool known = BNB_KnowsCode(sender.GetPlainId());
            GetGame().RPCSingleParam(this, BNB_EclBridgeRPCs.RPC_BNB_ECL_KNOWN_RESP, new Param1<bool>(known), true, sender);
            return;
        }

        if (rpc_type == BNB_EclBridgeRPCs.RPC_BNB_ECL_BEEP && !GetGame().IsDedicatedServer())
        {
            PlayRandomButtonSound();
            return;
        }

        if (rpc_type == BNB_EclBridgeRPCs.RPC_BNB_ECL_KNOWN_RESP && !GetGame().IsDedicatedServer())
        {
            Param1<bool> pKnown = new Param1<bool>(false);
            if (!ctx.Read(pKnown))
                return;
            BNB_ECL_KnownCache.SetKnown(this, pKnown.param1);
        }
    }
}
#endif
