// Organisation: Bullets'n'Bandages
// Author:       Bushy <contact@bushy.dev>
// Version:      v1.1.2
// Modified:     2026-07-18
//
// BNB_ECL_Loader.c - loads $profile:EclBfBridge\settings.json on the
// server (creating it with defaults if missing) and pushes a copy to each
// client on connect. QoL features stay OFF until configured.

class BNB_EclBridgeLoader
{
    protected static const string DIR = "$profile:EclBfBridge";
    protected static const string PATH = "$profile:EclBfBridge\\settings.json";
    protected static bool s_Loaded = false;

    // Lazy server-side load; safe to call from any condition path.
    static void EnsureLoaded()
    {
        if (s_Loaded)
            return;
        if (!GetGame() || !GetGame().IsServer())
            return;
        s_Loaded = true;
        if (!FileExist(PATH))
        {
            if (!FileExist(DIR))
                MakeDirectory(DIR);
            JsonFileLoader<BNB_EclBridgeSettingsData>.JsonSaveFile(PATH, BNB_EclBridgeSettings.Data());
            Print("[EclBfBridge] settings.json created with defaults");
            return;
        }
        BNB_EclBridgeSettingsData data = new BNB_EclBridgeSettingsData();
        string err;
        if (!JsonFileLoader<BNB_EclBridgeSettingsData>.LoadFile(PATH, data, err))
        {
            Print("[EclBfBridge] settings.json load FAILED (" + err + ") - using built-in defaults");
            return;
        }
        BNB_EclBridgeSettings.Apply(data);
        // Re-save so keys added in newer versions appear with their defaults.
        JsonFileLoader<BNB_EclBridgeSettingsData>.JsonSaveFile(PATH, data);
        Print("[EclBfBridge] settings loaded");
    }
}

modded class PlayerBase
{
    override void OnConnect()
    {
        super.OnConnect();
        BNB_EclBridgeLoader.EnsureLoaded();
        RPCSingleParam(BNB_EclBridgeRPCs.RPC_BNB_ECL_SETTINGS, new Param1<ref BNB_EclBridgeSettingsData>(BNB_EclBridgeSettings.Data()), true, GetIdentity());
    }

    override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
    {
        super.OnRPC(sender, rpc_type, ctx);
        if (!GetGame().IsServer() && rpc_type == BNB_EclBridgeRPCs.RPC_BNB_ECL_SETTINGS)
        {
            // Read into a live Param with a live payload - robust on every engine.
            Param1<ref BNB_EclBridgeSettingsData> p = new Param1<ref BNB_EclBridgeSettingsData>(new BNB_EclBridgeSettingsData());
            if (ctx.Read(p))
                BNB_EclBridgeSettings.Apply(p.param1);
        }
    }
}
