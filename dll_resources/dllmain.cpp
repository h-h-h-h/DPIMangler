#include <Windows.h>
#include <ShellScalingAPI.h>
#include "MinHook.h"

bool initialized = false;

typedef BOOL (WINAPI *SETPROCESSDPIAWARE)(VOID);
typedef HRESULT (WINAPI *SETPROCESSDPIAWARENESS)(PROCESS_DPI_AWARENESS);

// Pointer for calling the original DPI awareness functions.
SETPROCESSDPIAWARE fpSetProcessDPIAware = NULL;
SETPROCESSDPIAWARENESS fpSetProcessDpiAwareness = NULL;

// Replacement for SetProcessDPIAware.
BOOL WINAPI NukeSetProcessDPIAware()
{
    return FALSE;
}

// Replacement for SetProcessDpiAwareness.
HRESULT WINAPI NukeSetProcessDpiAwareness(PROCESS_DPI_AWARENESS value)
{
    return E_ACCESSDENIED;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        if (initialized) {
            break;
        }

        initialized = true;

        //MessageBoxW(NULL, L"Hooking", L"DPIMangler", MB_OK);

        // Initialize MinHook.
        if (MH_Initialize() != MH_OK)
        {
            MessageBoxW(NULL, L"Initialization error", L"DPIMangler", MB_OK);
            return 1;
        }

        // Create a hook for the DPI awareness functions, in disabled state.
        if (MH_CreateHook(&SetProcessDPIAware, &NukeSetProcessDPIAware,
            reinterpret_cast<void**>(&fpSetProcessDPIAware)) != MH_OK ||
            MH_CreateHook(&SetProcessDpiAwareness, &NukeSetProcessDpiAwareness,
            reinterpret_cast<void**>(&fpSetProcessDpiAwareness)) != MH_OK)
        {
            MessageBoxW(NULL, L"Error creating hook", L"DPIMangler", MB_OK);
            return 1;
        }

        // Enable the hook for the DPI awareness functions.
        if (MH_EnableHook(&SetProcessDPIAware) != MH_OK ||
            MH_EnableHook(&SetProcessDpiAwareness) != MH_OK)
        {
            MessageBoxW(NULL, L"Error enabling hook", L"DPIMangler", MB_OK);
            return 1;
        }

        //MessageBoxW(NULL, L"Hooked...", L"DPIManger", MB_OK);

        break;
    case DLL_PROCESS_DETACH:
        // Disable the hook for the DPI awareness functions.
        if (MH_DisableHook(&SetProcessDPIAware) != MH_OK ||
            MH_DisableHook(&SetProcessDpiAwareness) != MH_OK)
        {
            MessageBoxW(NULL, L"Error disabling hook", L"DPIMangler", MB_OK);
            return 1;
        }

        // Uninitialize MinHook.
        if (MH_Uninitialize() != MH_OK)
        {
            MessageBoxW(NULL, L"Error uninitializing minhook", L"DPIMangler", MB_OK);
            return 1;
        }
        break;
    }

	return TRUE;
}
