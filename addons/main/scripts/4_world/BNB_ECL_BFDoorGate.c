// Organisation: Bullets'n'Bandages
// Author:       Bushy <contact@bushy.dev>
// Version:      v1.0.4
// Modified:     2026-07-18
//
// BNB_ECL_BFDoorGate.c - Building Fortifications door integration for the
// Electric CodeLock: door lock-state reads the BM lock, door-side keypad verbs
// attach here, the lock slot needs a closed door (optionally fully built), and
// the attached lock drops intact when the gate part is dismantled or
// destroyed. The slot lock is released whenever the lock leaves the door.
// Auto-lock on close and the strict fully-built gate are settings-gated
// and OFF by default.

#ifdef BuildingFortifications
#ifdef BM_CodeLock
modded class BF_DoorBarricade
{
    BM_CodeLock BNB_GetBMCodeLock()
    {
        return BM_CodeLock.Cast(FindAttachmentBySlotName("Att_CombinationLock"));
    }

    override bool IsLocked()
    {
        BM_CodeLock bmLock = BNB_GetBMCodeLock();
        if (bmLock && bmLock.IsLocked())
            return true;
        return super.IsLocked();
    }

    override void OnPartDismantledServer(notnull Man player, string part_name, int action_id)
    {
        // Free the BM lock BEFORE the BF gate handling runs (its own gate branch
        // calls the fence-typed UnlockServer, which is unsafe on BF parents).
        ConstructionPart dismantledPart = GetConstruction().GetConstructionPart(part_name);
        if (dismantledPart && dismantledPart.IsGate())
        {
            BM_CodeLock bmLock = BNB_GetBMCodeLock();
            if (bmLock)
            {
                PlayerBase actorPb = PlayerBase.Cast(player);
                if (actorPb)
                    bmLock.BNB_RecordDetachActor(actorPb.GetIdentity());
                bmLock.OnParentDestroyed();
                GetInventory().DropEntity(InventoryMode.SERVER, this, bmLock);
                bmLock.PlaceOnSurface();
            }
        }
        super.OnPartDismantledServer(player, part_name, action_id);
    }

    // Gate DESTRUCTION drops the lock too - BF's own destroy path resolves the
    // lock with a fence-typed cast that misses BM and would strand it attached.
    override void OnPartDestroyedServer(Man player, string part_name, int action_id, bool destroyed_by_connected_part = false)
    {
        ConstructionPart destroyedPart = GetConstruction().GetConstructionPart(part_name);
        if (destroyedPart && destroyedPart.IsGate())
        {
            BM_CodeLock bmLock = BNB_GetBMCodeLock();
            if (bmLock)
            {
                PlayerBase actorPb = PlayerBase.Cast(player);
                if (actorPb)
                    bmLock.BNB_RecordDetachActor(actorPb.GetIdentity());
                bmLock.OnParentDestroyed();
                GetInventory().DropEntity(InventoryMode.SERVER, this, bmLock);
                bmLock.PlaceOnSurface();
            }
        }
        super.OnPartDestroyedServer(player, part_name, action_id, destroyed_by_connected_part);
    }

    // ECL clears the slot lock only for Fence-type parents, so any lock-exit
    // path on a BF door would strand the slot locked and refuse re-attach.
    override void EEItemDetached(EntityAI item, string slot_name)
    {
        super.EEItemDetached(item, slot_name);
        if (GetGame().IsServer() && item && item.IsKindOf("CombinationLock"))
            GetInventory().SetSlotLock(InventorySlots.GetSlotIdFromString("Att_CombinationLock"), false);
    }

    // A fresh lock needs a closed door with an empty slot; with
    // require_fully_built=1 the door must also be fully planked + hinged.
    bool BNB_LockSlotAvailable()
    {
        if (IsOpened())
            return false;
        if (FindAttachmentBySlotName("Att_CombinationLock"))
            return false;
        return BNB_LockAttachPermitted();
    }

    // Server-authoritative belt covering non-action attach paths (inventory
    // drag): open door always refuses; strict gate adds the fully-built set.
    bool BNB_LockAttachPermitted()
    {
        if (IsOpened())
            return false;
        if (!BNB_EclBridgeSettings.RequireFullyBuilt())
            return true;
        Construction c = GetConstruction();
        if (!c) return false;
        return c.IsPartConstructed("c_outer_lower_planks") && c.IsPartConstructed("c_outer_upper_planks") && c.IsPartConstructed("f_inner_lower_planks") && c.IsPartConstructed("f_inner_upper_planks") && c.IsPartConstructed("i_hindges");
    }

    override bool CanReceiveAttachment(EntityAI attachment, int slotId)
    {
        if (attachment && attachment.IsKindOf("CombinationLock") && !BNB_LockAttachPermitted())
            return false;
        return super.CanReceiveAttachment(attachment, slotId);
    }

    // Hinges alone give the engine an openable gate with no visible door -
    // with the strict gate enabled, the open verb waits for a frame.
    bool BNB_AnyFrameBuilt()
    {
        Construction c = GetConstruction();
        if (!c)
            return false;
        return c.IsPartConstructed("b_lower_frame") || c.IsPartConstructed("b_upper_frame");
    }

    override bool CanDisplayAttachmentSlot(string slot_name)
    {
        if (!super.CanDisplayAttachmentSlot(slot_name))
            return false;
        if (slot_name == "Att_CombinationLock")
        {
            // A lock already attached stays visible/manageable on a closed door.
            if (!IsOpened() && FindAttachmentBySlotName("Att_CombinationLock"))
                return true;
            return BNB_LockSlotAvailable();
        }
        return true;
    }

    // The door is the raycast target - keypad + quick-unlock verbs attach here.
    // Quick-unlock first: the default interact verb when the code is known.
    override void SetActions()
    {
        super.SetActions();
        AddAction(ActionBNBQuickUnlock);
        AddAction(ActionAttachBMCodeLock);
        AddAction(ActionOpenBMCodeLock);
        AddAction(ActionLockBMCodeLock);
        AddAction(ActionChangePassBMCodeLock);
        AddAction(ActionDetachBMCodeLock);
    }

    // Optional: closing the door engages a coded lock automatically.
    override void CloseFence()
    {
        super.CloseFence();
        if (!GetGame().IsServer())
            return;
        if (!BNB_EclBridgeSettings.AutoLockOnClose())
            return;
        BM_CodeLock bmLock = BNB_GetBMCodeLock();
        if (bmLock && bmLock.HasCode() && !bmLock.IsLocked())
            bmLock.LockCode(null);
    }
}

modded class BF_DoubleDoorBarricade
{
    BM_CodeLock BNB_GetBMCodeLock()
    {
        return BM_CodeLock.Cast(FindAttachmentBySlotName("Att_CombinationLock"));
    }

    override bool IsLocked()
    {
        BM_CodeLock bmLock = BNB_GetBMCodeLock();
        if (bmLock && bmLock.IsLocked())
            return true;
        return super.IsLocked();
    }

    override void OnPartDismantledServer(notnull Man player, string part_name, int action_id)
    {
        // Same lock-drop-before-super as the single door.
        ConstructionPart dismantledPart = GetConstruction().GetConstructionPart(part_name);
        if (dismantledPart && dismantledPart.IsGate())
        {
            BM_CodeLock bmLock = BNB_GetBMCodeLock();
            if (bmLock)
            {
                PlayerBase actorPb = PlayerBase.Cast(player);
                if (actorPb)
                    bmLock.BNB_RecordDetachActor(actorPb.GetIdentity());
                bmLock.OnParentDestroyed();
                GetInventory().DropEntity(InventoryMode.SERVER, this, bmLock);
                bmLock.PlaceOnSurface();
            }
        }
        super.OnPartDismantledServer(player, part_name, action_id);
    }

    // Same stranded-lock guard as the single door for gate destruction.
    override void OnPartDestroyedServer(Man player, string part_name, int action_id, bool destroyed_by_connected_part = false)
    {
        ConstructionPart destroyedPart = GetConstruction().GetConstructionPart(part_name);
        if (destroyedPart && destroyedPart.IsGate())
        {
            BM_CodeLock bmLock = BNB_GetBMCodeLock();
            if (bmLock)
            {
                PlayerBase actorPb = PlayerBase.Cast(player);
                if (actorPb)
                    bmLock.BNB_RecordDetachActor(actorPb.GetIdentity());
                bmLock.OnParentDestroyed();
                GetInventory().DropEntity(InventoryMode.SERVER, this, bmLock);
                bmLock.PlaceOnSurface();
            }
        }
        super.OnPartDestroyedServer(player, part_name, action_id, destroyed_by_connected_part);
    }

    // Same stranded-slot belt as the single door.
    override void EEItemDetached(EntityAI item, string slot_name)
    {
        super.EEItemDetached(item, slot_name);
        if (GetGame().IsServer() && item && item.IsKindOf("CombinationLock"))
            GetInventory().SetSlotLock(InventorySlots.GetSlotIdFromString("Att_CombinationLock"), false);
    }

    // Same fresh-attach predicate as the single door (double-door part names).
    bool BNB_LockSlotAvailable()
    {
        if (IsOpened())
            return false;
        if (FindAttachmentBySlotName("Att_CombinationLock"))
            return false;
        return BNB_LockAttachPermitted();
    }

    // Same server-authoritative belt as the single door (double-door parts).
    bool BNB_LockAttachPermitted()
    {
        if (IsOpened())
            return false;
        if (!BNB_EclBridgeSettings.RequireFullyBuilt())
            return true;
        Construction c = GetConstruction();
        if (!c) return false;
        return c.IsPartConstructed("c_left_planks") && c.IsPartConstructed("c_right_planks") && c.IsPartConstructed("f_left_inner_planks") && c.IsPartConstructed("f_right_inner_planks") && c.IsPartConstructed("h_hindges");
    }

    override bool CanReceiveAttachment(EntityAI attachment, int slotId)
    {
        if (attachment && attachment.IsKindOf("CombinationLock") && !BNB_LockAttachPermitted())
            return false;
        return super.CanReceiveAttachment(attachment, slotId);
    }

    // Same hinges-only open guard as the single door (double-door part names).
    bool BNB_AnyFrameBuilt()
    {
        Construction c = GetConstruction();
        if (!c)
            return false;
        return c.IsPartConstructed("b_left_frames") || c.IsPartConstructed("b_right_frames");
    }

    override bool CanDisplayAttachmentSlot(string slot_name)
    {
        if (!super.CanDisplayAttachmentSlot(slot_name))
            return false;
        if (slot_name == "Att_CombinationLock")
        {
            // A lock already attached stays visible/manageable on a closed door.
            if (!IsOpened() && FindAttachmentBySlotName("Att_CombinationLock"))
                return true;
            return BNB_LockSlotAvailable();
        }
        return true;
    }

    override void SetActions()
    {
        super.SetActions();
        AddAction(ActionBNBQuickUnlock);
        AddAction(ActionAttachBMCodeLock);
        AddAction(ActionOpenBMCodeLock);
        AddAction(ActionLockBMCodeLock);
        AddAction(ActionChangePassBMCodeLock);
        AddAction(ActionDetachBMCodeLock);
    }

    // Optional: closing the door engages a coded lock automatically.
    override void CloseFence()
    {
        super.CloseFence();
        if (!GetGame().IsServer())
            return;
        if (!BNB_EclBridgeSettings.AutoLockOnClose())
            return;
        BM_CodeLock bmLock = BNB_GetBMCodeLock();
        if (bmLock && bmLock.HasCode() && !bmLock.IsLocked())
            bmLock.LockCode(null);
    }
}

// Hide the native open verb on a locked door (keypad verbs take over) and on
// a hinges-only door under the strict gate (nothing visible to open).
modded class ActionOpenBuildingCore
{
    override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
    {
        if (!super.ActionCondition(player, target, item))
            return false;
        BF_DoorBarricade door = BF_DoorBarricade.Cast(target.GetObject());
        if (door && door.IsLocked())
            return false;
        if (door && BNB_EclBridgeSettings.RequireFullyBuilt() && !door.BNB_AnyFrameBuilt())
            return false;
        BF_DoubleDoorBarricade doubleDoor = BF_DoubleDoorBarricade.Cast(target.GetObject());
        if (doubleDoor && doubleDoor.IsLocked())
            return false;
        if (doubleDoor && BNB_EclBridgeSettings.RequireFullyBuilt() && !doubleDoor.BNB_AnyFrameBuilt())
            return false;
        return true;
    }
}
#endif
#endif
