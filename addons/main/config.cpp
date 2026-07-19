// Organisation: Bullets'n'Bandages
// Author:       Bushy <contact@bushy.dev>
// Version:      v1.0.3
// Modified:     2026-07-19
//
// config.cpp - ECL BF Bridge: Electric CodeLock support on Building
// Fortifications doors, with optional server-configurable QoL features
// (QoL OFF by default - see $profile:EclBfBridge\settings.json).

class CfgPatches
{
    class BNB_EclBfBridge
    {
        units[] = {};
        weapons[] = {};
        requiredVersion = 0.1;
        // Load after both upstreams so the modded-class layers land on top.
        requiredAddons[] = { "DZ_Data", "DZ_Scripts", "BM_CodeLock_Client", "BM_CodeLock", "BuildingFortifications" };
        version = "1.2.3";
    };
};

class CfgMods
{
    class BNB_EclBfBridge
    {
        dir = "BNBEclBfBridge";
        name = "ECL-BF-Bridge";
        author = "Bushy";
        type = "mod";
        hidePicture = 0;
        hideName = 0;
        tooltip = "Electric CodeLock on Building Fortifications doors";
        overview = "Bridge mod: attach and operate the Electric CodeLock on Building Fortifications barricade doors. Optional QoL features are OFF by default and enabled via $profile:EclBfBridge\\settings.json.";
        dependencies[] = { "Game", "World", "Mission" };
        class defs
        {
            class gameScriptModule
            {
                value = "";
                files[] = { "BNBEclBfBridge/scripts/3_game" };
            };
            class worldScriptModule
            {
                value = "";
                files[] = { "BNBEclBfBridge/scripts/4_world" };
            };
            class missionScriptModule
            {
                value = "";
                files[] = { "BNBEclBfBridge/scripts/5_mission" };
            };
        };
    };
};
