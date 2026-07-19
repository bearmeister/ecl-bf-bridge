// Organisation: Bullets'n'Bandages
// Author:       Bushy <contact@bushy.dev>
// Version:      v1.0.1
// Modified:     2026-07-18
//
// BNB_ECL_Actions.c - Electric CodeLock action conditions generalised to
// Building Fortifications doors: attach, keypad, lock, change-pass and detach
// all resolve the lock through a BF door target. Optional extras (keypad
// auto-open after attach, screwdriver detach on an open door) are
// settings-gated and OFF by default.

#ifdef BM_CodeLock
modded class ActionAttachBMCodeLock
{
    override protected bool BM_IsSupportedTarget(EntityAI target)
    {
        if (super.BM_IsSupportedTarget(target))
            return true;
        #ifdef BuildingFortifications
        if (target && target.IsInherited(BuildingFortficationsCore))
            return true;
        #endif
        return false;
    }

    override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
    {
        if (!super.ActionCondition(player, target, item))
            return false;
        EntityAI attachTarget = EntityAI.Cast(target.GetObject());
        if (!attachTarget)
            return false;
        #ifdef BuildingFortifications
        // No attach verb until the door can actually take the lock - mirrors
        // the slot-display gate.
        BF_DoorBarricade bnbDoor = BF_DoorBarricade.Cast(attachTarget);
        if (bnbDoor && !bnbDoor.BNB_LockSlotAvailable())
            return false;
        BF_DoubleDoorBarricade bnbDouble = BF_DoubleDoorBarricade.Cast(attachTarget);
        if (bnbDouble && !bnbDouble.BNB_LockSlotAvailable())
            return false;
        #endif
        return true;
    }

    // Optional: a freshly attached codeless lock opens the keypad straight
    // into set-PIN mode.
    override void OnExecuteClient(ActionData action_data)
    {
        super.OnExecuteClient(action_data);
        if (!BNB_EclBridgeSettings.KeypadOpenAfterAttach())
            return;
        EntityAI attachTarget = EntityAI.Cast(action_data.m_Target.GetObject());
        if (!attachTarget)
            return;
        GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(BNB_OpenKeypadAfterAttach, 500, false, action_data.m_Player, attachTarget);
    }

    protected void BNB_OpenKeypadAfterAttach(PlayerBase player, EntityAI attachTarget)
    {
        if (!player || !attachTarget)
            return;
        BM_CodeLock lock = BM_CodeLock.Cast(attachTarget.FindAttachmentBySlotName("Att_CombinationLock"));
        if (!lock || lock.HasCode())
            return;
        player.OpenBMCodeLockUI(lock, true);
    }
}

modded class ActionOpenBMCodeLock
{
    override protected BM_CodeLock ResolveLock(ActionTarget target)
    {
        BM_CodeLock lock = super.ResolveLock(target);
        if (lock)
            return lock;
        #ifdef BuildingFortifications
        EntityAI targetEntity = EntityAI.Cast(target.GetObject());
        if (targetEntity && targetEntity.IsInherited(BuildingFortficationsCore))
            return BM_CodeLock.Cast(targetEntity.FindAttachmentBySlotName("Att_CombinationLock"));
        #endif
        return null;
    }

    override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
    {
        if (super.ActionCondition(player, target, item))
        {
            // Wrong-code cooldown covers the upstream fence path too.
            BM_CodeLock fenceLock = ResolveLock(target);
            if (fenceLock && fenceLock.IsLocked() && fenceLock.BNB_IsCoolingDown())
                return false;
            return true;
        }
        #ifdef BuildingFortifications
        BuildingFortficationsCore door = BuildingFortficationsCore.Cast(target.GetObject());
        if (!door)
            return false;
        BM_CodeLock lock = BM_CodeLock.Cast(door.FindAttachmentBySlotName("Att_CombinationLock"));
        if (!lock)
            return false;
        if (!lock.IsLockAttached())
            return false;
        if (!lock.HasCode())
        {
            m_Text = "Set Code";
            return true;
        }
        if (lock.IsLocked() && !door.IsOpened())
        {
            // Keypad refuses for the optional wrong-code cooldown window.
            if (lock.BNB_IsCoolingDown())
                return false;
            m_Text = "Enter Code";
            return true;
        }
        #endif
        return false;
    }
}

modded class ActionLockBMCodeLock
{
    override protected BM_CodeLock ResolveLock(ActionTarget target)
    {
        BM_CodeLock lock = super.ResolveLock(target);
        if (lock)
            return lock;
        #ifdef BuildingFortifications
        EntityAI targetEntity = EntityAI.Cast(target.GetObject());
        if (targetEntity && targetEntity.IsInherited(BuildingFortficationsCore))
            return BM_CodeLock.Cast(targetEntity.FindAttachmentBySlotName("Att_CombinationLock"));
        #endif
        return null;
    }

    override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
    {
        if (super.ActionCondition(player, target, item))
            return true;
        #ifdef BuildingFortifications
        BuildingFortficationsCore door = BuildingFortficationsCore.Cast(target.GetObject());
        if (!door)
            return false;
        BM_CodeLock lock = BM_CodeLock.Cast(door.FindAttachmentBySlotName("Att_CombinationLock"));
        if (!lock)
            return false;
        if (!lock.HasCode())
            return false;
        if (lock.IsLocked())
            return false;
        if (door.IsOpened())
            return false;
        return true;
        #else
        return false;
        #endif
    }
}

modded class ActionChangePassBMCodeLock
{
    override protected BM_CodeLock ResolveLock(ActionTarget target)
    {
        BM_CodeLock lock = super.ResolveLock(target);
        if (lock)
            return lock;
        #ifdef BuildingFortifications
        EntityAI targetEntity = EntityAI.Cast(target.GetObject());
        if (targetEntity && targetEntity.IsInherited(BuildingFortficationsCore))
            return BM_CodeLock.Cast(targetEntity.FindAttachmentBySlotName("Att_CombinationLock"));
        #endif
        return null;
    }
}

modded class ActionDetachBMCodeLock
{
    override protected BM_CodeLock ResolveLock(ActionTarget target)
    {
        BM_CodeLock lock = super.ResolveLock(target);
        if (lock)
            return lock;
        #ifdef BuildingFortifications
        EntityAI targetEntity = EntityAI.Cast(target.GetObject());
        if (targetEntity && targetEntity.IsInherited(BuildingFortficationsCore))
            return BM_CodeLock.Cast(targetEntity.FindAttachmentBySlotName("Att_CombinationLock"));
        #endif
        return null;
    }

    override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
    {
        bool ok = super.ActionCondition(player, target, item);
        #ifdef BuildingFortifications
        // Optional: screwdriver detach of an unlocked lock on an open door.
        if (!ok && BNB_EclBridgeSettings.ScrewdriverDetachOpenDoor())
        {
            BuildingFortficationsCore door = BuildingFortficationsCore.Cast(target.GetObject());
            BM_CodeLock bfLock = ResolveLock(target);
            if (door && bfLock && item && Screwdriver.Cast(item) && door.IsOpened() && !bfLock.IsLocked())
                ok = true;
        }
        #endif
        return ok;
    }

    // Upstream server exec is fence-typed and returns early for a BF parent -
    // this runs the same detach sequence against the BF door.
    override void OnFinishProgressServer(ActionData action_data)
    {
        // Actor recorded pre-drop so the ground-drop audit line carries a name.
        BM_CodeLock auditLock = ResolveLock(action_data.m_Target);
        if (auditLock && action_data.m_Player)
            auditLock.BNB_RecordDetachActor(action_data.m_Player.GetIdentity());
        super.OnFinishProgressServer(action_data);
        #ifdef BuildingFortifications
        BuildingFortficationsCore door = BuildingFortficationsCore.Cast(action_data.m_Target.GetObject());
        if (!door)
            return;
        BM_CodeLock lock = BM_CodeLock.Cast(door.FindAttachmentBySlotName("Att_CombinationLock"));
        if (!lock)
            return;
        int slotId = InventorySlots.GetSlotIdFromString("Att_CombinationLock");
        door.GetInventory().SetSlotLock(slotId, false);
        lock.PrepareForDetach();
        door.GetInventory().DropEntity(InventoryMode.SERVER, door, lock);
        lock.SetTakeable(true);
        lock.SetSynchDirty();
        #endif
    }
}
#endif
