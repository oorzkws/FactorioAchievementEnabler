#include "DllLoader.h"
#include <forward_list>

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	switch (fdwReason) {
	case DLL_PROCESS_ATTACH:
		hInstance = hinstDLL;

		DisableThreadLibraryCalls(hinstDLL);
#if defined(_DEBUG) || defined(__ENABLE_CONSOLE)
		OpenConsole();
#endif

#ifndef __NO_EXP
		FixLibraryImport();
#endif
		StartPatch();
		break;

	case DLL_PROCESS_DETACH:
		ExitLibrary();
		break;
	}
	return TRUE;
}

void OpenConsole()
{
	// Re-enable printf functions.
	FILE *stream;
	if (!AttachConsole(-1))
		AllocConsole();
	freopen_s(&stream, "CONOUT$", "w+", stdout);

	dprintf("Fixing library import...\n");
}

void FixLibraryImport() {
	wchar_t szTargetDll[MAX_PATH] = { 0 };
	wchar_t szThisDllPath[MAX_PATH] = { 0 };

	GetSystemDirectory(szTargetDll, MAX_PATH);
	GetModuleFileName(hInstance, szThisDllPath, MAX_PATH);
	auto szDllName = wcsrchr(szThisDllPath, L'\\');
	wcscat_s(szTargetDll, MAX_PATH, szDllName++);

	hDll = LoadLibrary(szTargetDll);

	// If error, exit.
	if (!hDll) {
#ifdef _DEBUG
		wchar_t szErrorMsg[MAX_PATH] = { 0 };
		wsprintf(szErrorMsg, L"Failed to load dll:\n%s", szTargetDll);
		dwprintf(L"%s\n==============================================\n", szErrorMsg);
#endif
		ExitProcess(1);
		// ReSharper disable once CppUnreachableCode
		return;
	}

	int i = 0;
	while (szDllName[i])
	{
		szDllName[i] = tolower(szDllName[i]);
		i++;
	}

	dwprintf(L"Fixing import for %s...\n", szDllName);

#pragma region Restore API
#ifdef __EXP_VERSION
	if (wcscmp(szDllName, L"version.dll") == 0)
	{
		FixSingleApi("GetFileVersionInfoA", uAddr(API_EXPORT::GetFileVersionInfoA));
		FixSingleApi("GetFileVersionInfoByHandle", uAddr(API_EXPORT::GetFileVersionInfoByHandle));
		FixSingleApi("GetFileVersionInfoExW", uAddr(API_EXPORT::GetFileVersionInfoExW));
		FixSingleApi("GetFileVersionInfoSizeA", uAddr(API_EXPORT::GetFileVersionInfoSizeA));
		FixSingleApi("GetFileVersionInfoSizeExW", uAddr(API_EXPORT::GetFileVersionInfoSizeExW));
		FixSingleApi("GetFileVersionInfoSizeW", uAddr(API_EXPORT::GetFileVersionInfoSizeW));
		FixSingleApi("GetFileVersionInfoW", uAddr(API_EXPORT::GetFileVersionInfoW));
		FixSingleApi("VerFindFileA", uAddr(API_EXPORT::VerFindFileA));
		FixSingleApi("VerFindFileW", uAddr(API_EXPORT::VerFindFileW));
		FixSingleApi("VerInstallFileA", uAddr(API_EXPORT::VerInstallFileA));
		FixSingleApi("VerInstallFileW", uAddr(API_EXPORT::VerInstallFileW));
		FixSingleApi("VerLanguageNameA", uAddr(API_EXPORT::VerLanguageNameA));
		FixSingleApi("VerLanguageNameW", uAddr(API_EXPORT::VerLanguageNameW));
		FixSingleApi("VerQueryValueA", uAddr(API_EXPORT::VerQueryValueA));
		FixSingleApi("VerQueryValueW", uAddr(API_EXPORT::VerQueryValueW));
		FixSingleApi("VerQueryValueIndexA", uAddr(API_EXPORT::VerQueryValueIndexA));
		FixSingleApi("VerQueryValueIndexW", uAddr(API_EXPORT::VerQueryValueIndexW));
	}
#endif

#ifdef __EXP_LPK
	if (wcscmp(szDllName, L"lpk.dll") == 0)
	{
		FixSingleApi("LpkDllInitialize", uAddr(API_EXPORT::LpkDllInitialize));
		FixSingleApi("LpkDrawTextEx", uAddr(API_EXPORT::LpkDrawTextEx));

		memcpy(API_EXPORT::LpkEditControl,
			GetProcAddress(hDll, "LpkEditControl"),
			sizeof(API_EXPORT::LpkEditControl)
		);

		FixSingleApi("LpkExtTextOut", uAddr(API_EXPORT::LpkExtTextOut));
		FixSingleApi("LpkGetCharacterPlacement", uAddr(API_EXPORT::LpkGetCharacterPlacement));
		FixSingleApi("LpkGetTextExtentExPoint", uAddr(API_EXPORT::LpkGetTextExtentExPoint));
		FixSingleApi("LpkInitialize", uAddr(API_EXPORT::LpkInitialize));
		FixSingleApi("LpkPSMTextOut", uAddr(API_EXPORT::LpkPSMTextOut));
		FixSingleApi("LpkTabbedTextOut", uAddr(API_EXPORT::LpkTabbedTextOut));
		FixSingleApi("LpkUseGDIWidthCache", uAddr(API_EXPORT::LpkUseGDIWidthCache));
		FixSingleApi("ftsWordBreak", uAddr(API_EXPORT::ftsWordBreak));
	}
#endif

#ifdef __EXP_MSIMG32
	FixSingleApi("AlphaBlend", uAddr(API_EXPORT::AlphaBlend));
	FixSingleApi("DllInitialize", uAddr(API_EXPORT::DllInitialize));
	FixSingleApi("GradientFill", uAddr(API_EXPORT::GradientFill));
	FixSingleApi("TransparentBlt", uAddr(API_EXPORT::TransparentBlt));
	FixSingleApi("vSetDdrawflag", uAddr(API_EXPORT::vSetDdrawflag));
#endif

#ifdef __EXP_WINMM
	if (wcscmp(szDllName, L"winmm.dll") == 0)
	{
#ifdef PlaySound
#ifndef __BAK__PlaySound
#define __BAK__PlaySound PlaySound
#endif
#undef PlaySound
#endif
		FixSingleApi("mciExecute", uAddr(API_EXPORT::mciExecute));
		FixSingleApi("CloseDriver", uAddr(API_EXPORT::CloseDriver));
		FixSingleApi("DefDriverProc", uAddr(API_EXPORT::DefDriverProc));
		FixSingleApi("DriverCallback", uAddr(API_EXPORT::DriverCallback));
		FixSingleApi("DrvGetModuleHandle", uAddr(API_EXPORT::DrvGetModuleHandle));
		FixSingleApi("GetDriverModuleHandle", uAddr(API_EXPORT::GetDriverModuleHandle));
		FixSingleApi("OpenDriver", uAddr(API_EXPORT::OpenDriver));
		FixSingleApi("PlaySound", uAddr(API_EXPORT::PlaySound));
		FixSingleApi("PlaySoundA", uAddr(API_EXPORT::PlaySoundA));
		FixSingleApi("PlaySoundW", uAddr(API_EXPORT::PlaySoundW));
		FixSingleApi("SendDriverMessage", uAddr(API_EXPORT::SendDriverMessage));
		FixSingleApi("WOWAppExit", uAddr(API_EXPORT::WOWAppExit));
		FixSingleApi("auxGetDevCapsA", uAddr(API_EXPORT::auxGetDevCapsA));
		FixSingleApi("auxGetDevCapsW", uAddr(API_EXPORT::auxGetDevCapsW));
		FixSingleApi("auxGetNumDevs", uAddr(API_EXPORT::auxGetNumDevs));
		FixSingleApi("auxGetVolume", uAddr(API_EXPORT::auxGetVolume));
		FixSingleApi("auxOutMessage", uAddr(API_EXPORT::auxOutMessage));
		FixSingleApi("auxSetVolume", uAddr(API_EXPORT::auxSetVolume));
		FixSingleApi("joyConfigChanged", uAddr(API_EXPORT::joyConfigChanged));
		FixSingleApi("joyGetDevCapsA", uAddr(API_EXPORT::joyGetDevCapsA));
		FixSingleApi("joyGetDevCapsW", uAddr(API_EXPORT::joyGetDevCapsW));
		FixSingleApi("joyGetNumDevs", uAddr(API_EXPORT::joyGetNumDevs));
		FixSingleApi("joyGetPos", uAddr(API_EXPORT::joyGetPos));
		FixSingleApi("joyGetPosEx", uAddr(API_EXPORT::joyGetPosEx));
		FixSingleApi("joyGetThreshold", uAddr(API_EXPORT::joyGetThreshold));
		FixSingleApi("joyReleaseCapture", uAddr(API_EXPORT::joyReleaseCapture));
		FixSingleApi("joySetCapture", uAddr(API_EXPORT::joySetCapture));
		FixSingleApi("joySetThreshold", uAddr(API_EXPORT::joySetThreshold));
		FixSingleApi("mciDriverNotify", uAddr(API_EXPORT::mciDriverNotify));
		FixSingleApi("mciDriverYield", uAddr(API_EXPORT::mciDriverYield));
		FixSingleApi("mciFreeCommandResource", uAddr(API_EXPORT::mciFreeCommandResource));
		FixSingleApi("mciGetCreatorTask", uAddr(API_EXPORT::mciGetCreatorTask));
		FixSingleApi("mciGetDeviceIDA", uAddr(API_EXPORT::mciGetDeviceIDA));
		FixSingleApi("mciGetDeviceIDFromElementIDA", uAddr(API_EXPORT::mciGetDeviceIDFromElementIDA));
		FixSingleApi("mciGetDeviceIDFromElementIDW", uAddr(API_EXPORT::mciGetDeviceIDFromElementIDW));
		FixSingleApi("mciGetDeviceIDW", uAddr(API_EXPORT::mciGetDeviceIDW));
		FixSingleApi("mciGetDriverData", uAddr(API_EXPORT::mciGetDriverData));
		FixSingleApi("mciGetErrorStringA", uAddr(API_EXPORT::mciGetErrorStringA));
		FixSingleApi("mciGetErrorStringW", uAddr(API_EXPORT::mciGetErrorStringW));
		FixSingleApi("mciGetYieldProc", uAddr(API_EXPORT::mciGetYieldProc));
		FixSingleApi("mciLoadCommandResource", uAddr(API_EXPORT::mciLoadCommandResource));
		FixSingleApi("mciSendCommandA", uAddr(API_EXPORT::mciSendCommandA));
		FixSingleApi("mciSendCommandW", uAddr(API_EXPORT::mciSendCommandW));
		FixSingleApi("mciSendStringA", uAddr(API_EXPORT::mciSendStringA));
		FixSingleApi("mciSendStringW", uAddr(API_EXPORT::mciSendStringW));
		FixSingleApi("mciSetDriverData", uAddr(API_EXPORT::mciSetDriverData));
		FixSingleApi("mciSetYieldProc", uAddr(API_EXPORT::mciSetYieldProc));
		FixSingleApi("midiConnect", uAddr(API_EXPORT::midiConnect));
		FixSingleApi("midiDisconnect", uAddr(API_EXPORT::midiDisconnect));
		FixSingleApi("midiInAddBuffer", uAddr(API_EXPORT::midiInAddBuffer));
		FixSingleApi("midiInClose", uAddr(API_EXPORT::midiInClose));
		FixSingleApi("midiInGetDevCapsA", uAddr(API_EXPORT::midiInGetDevCapsA));
		FixSingleApi("midiInGetDevCapsW", uAddr(API_EXPORT::midiInGetDevCapsW));
		FixSingleApi("midiInGetErrorTextA", uAddr(API_EXPORT::midiInGetErrorTextA));
		FixSingleApi("midiInGetErrorTextW", uAddr(API_EXPORT::midiInGetErrorTextW));
		FixSingleApi("midiInGetID", uAddr(API_EXPORT::midiInGetID));
		FixSingleApi("midiInGetNumDevs", uAddr(API_EXPORT::midiInGetNumDevs));
		FixSingleApi("midiInMessage", uAddr(API_EXPORT::midiInMessage));
		FixSingleApi("midiInOpen", uAddr(API_EXPORT::midiInOpen));
		FixSingleApi("midiInPrepareHeader", uAddr(API_EXPORT::midiInPrepareHeader));
		FixSingleApi("midiInReset", uAddr(API_EXPORT::midiInReset));
		FixSingleApi("midiInStart", uAddr(API_EXPORT::midiInStart));
		FixSingleApi("midiInStop", uAddr(API_EXPORT::midiInStop));
		FixSingleApi("midiInUnprepareHeader", uAddr(API_EXPORT::midiInUnprepareHeader));
		FixSingleApi("midiOutCacheDrumPatches", uAddr(API_EXPORT::midiOutCacheDrumPatches));
		FixSingleApi("midiOutCachePatches", uAddr(API_EXPORT::midiOutCachePatches));
		FixSingleApi("midiOutClose", uAddr(API_EXPORT::midiOutClose));
		FixSingleApi("midiOutGetDevCapsA", uAddr(API_EXPORT::midiOutGetDevCapsA));
		FixSingleApi("midiOutGetDevCapsW", uAddr(API_EXPORT::midiOutGetDevCapsW));
		FixSingleApi("midiOutGetErrorTextA", uAddr(API_EXPORT::midiOutGetErrorTextA));
		FixSingleApi("midiOutGetErrorTextW", uAddr(API_EXPORT::midiOutGetErrorTextW));
		FixSingleApi("midiOutGetID", uAddr(API_EXPORT::midiOutGetID));
		FixSingleApi("midiOutGetNumDevs", uAddr(API_EXPORT::midiOutGetNumDevs));
		FixSingleApi("midiOutGetVolume", uAddr(API_EXPORT::midiOutGetVolume));
		FixSingleApi("midiOutLongMsg", uAddr(API_EXPORT::midiOutLongMsg));
		FixSingleApi("midiOutMessage", uAddr(API_EXPORT::midiOutMessage));
		FixSingleApi("midiOutOpen", uAddr(API_EXPORT::midiOutOpen));
		FixSingleApi("midiOutPrepareHeader", uAddr(API_EXPORT::midiOutPrepareHeader));
		FixSingleApi("midiOutReset", uAddr(API_EXPORT::midiOutReset));
		FixSingleApi("midiOutSetVolume", uAddr(API_EXPORT::midiOutSetVolume));
		FixSingleApi("midiOutShortMsg", uAddr(API_EXPORT::midiOutShortMsg));
		FixSingleApi("midiOutUnprepareHeader", uAddr(API_EXPORT::midiOutUnprepareHeader));
		FixSingleApi("midiStreamClose", uAddr(API_EXPORT::midiStreamClose));
		FixSingleApi("midiStreamOpen", uAddr(API_EXPORT::midiStreamOpen));
		FixSingleApi("midiStreamOut", uAddr(API_EXPORT::midiStreamOut));
		FixSingleApi("midiStreamPause", uAddr(API_EXPORT::midiStreamPause));
		FixSingleApi("midiStreamPosition", uAddr(API_EXPORT::midiStreamPosition));
		FixSingleApi("midiStreamProperty", uAddr(API_EXPORT::midiStreamProperty));
		FixSingleApi("midiStreamRestart", uAddr(API_EXPORT::midiStreamRestart));
		FixSingleApi("midiStreamStop", uAddr(API_EXPORT::midiStreamStop));
		FixSingleApi("mixerClose", uAddr(API_EXPORT::mixerClose));
		FixSingleApi("mixerGetControlDetailsA", uAddr(API_EXPORT::mixerGetControlDetailsA));
		FixSingleApi("mixerGetControlDetailsW", uAddr(API_EXPORT::mixerGetControlDetailsW));
		FixSingleApi("mixerGetDevCapsA", uAddr(API_EXPORT::mixerGetDevCapsA));
		FixSingleApi("mixerGetDevCapsW", uAddr(API_EXPORT::mixerGetDevCapsW));
		FixSingleApi("mixerGetID", uAddr(API_EXPORT::mixerGetID));
		FixSingleApi("mixerGetLineControlsA", uAddr(API_EXPORT::mixerGetLineControlsA));
		FixSingleApi("mixerGetLineControlsW", uAddr(API_EXPORT::mixerGetLineControlsW));
		FixSingleApi("mixerGetLineInfoA", uAddr(API_EXPORT::mixerGetLineInfoA));
		FixSingleApi("mixerGetLineInfoW", uAddr(API_EXPORT::mixerGetLineInfoW));
		FixSingleApi("mixerGetNumDevs", uAddr(API_EXPORT::mixerGetNumDevs));
		FixSingleApi("mixerMessage", uAddr(API_EXPORT::mixerMessage));
		FixSingleApi("mixerOpen", uAddr(API_EXPORT::mixerOpen));
		FixSingleApi("mixerSetControlDetails", uAddr(API_EXPORT::mixerSetControlDetails));
		FixSingleApi("mmDrvInstall", uAddr(API_EXPORT::mmDrvInstall));
		FixSingleApi("mmGetCurrentTask", uAddr(API_EXPORT::mmGetCurrentTask));
		FixSingleApi("mmTaskBlock", uAddr(API_EXPORT::mmTaskBlock));
		FixSingleApi("mmTaskCreate", uAddr(API_EXPORT::mmTaskCreate));
		FixSingleApi("mmTaskSignal", uAddr(API_EXPORT::mmTaskSignal));
		FixSingleApi("mmTaskYield", uAddr(API_EXPORT::mmTaskYield));
		FixSingleApi("mmioAdvance", uAddr(API_EXPORT::mmioAdvance));
		FixSingleApi("mmioAscend", uAddr(API_EXPORT::mmioAscend));
		FixSingleApi("mmioClose", uAddr(API_EXPORT::mmioClose));
		FixSingleApi("mmioCreateChunk", uAddr(API_EXPORT::mmioCreateChunk));
		FixSingleApi("mmioDescend", uAddr(API_EXPORT::mmioDescend));
		FixSingleApi("mmioFlush", uAddr(API_EXPORT::mmioFlush));
		FixSingleApi("mmioGetInfo", uAddr(API_EXPORT::mmioGetInfo));
		FixSingleApi("mmioInstallIOProcA", uAddr(API_EXPORT::mmioInstallIOProcA));
		FixSingleApi("mmioInstallIOProcW", uAddr(API_EXPORT::mmioInstallIOProcW));
		FixSingleApi("mmioOpenA", uAddr(API_EXPORT::mmioOpenA));
		FixSingleApi("mmioOpenW", uAddr(API_EXPORT::mmioOpenW));
		FixSingleApi("mmioRead", uAddr(API_EXPORT::mmioRead));
		FixSingleApi("mmioRenameA", uAddr(API_EXPORT::mmioRenameA));
		FixSingleApi("mmioRenameW", uAddr(API_EXPORT::mmioRenameW));
		FixSingleApi("mmioSeek", uAddr(API_EXPORT::mmioSeek));
		FixSingleApi("mmioSendMessage", uAddr(API_EXPORT::mmioSendMessage));
		FixSingleApi("mmioSetBuffer", uAddr(API_EXPORT::mmioSetBuffer));
		FixSingleApi("mmioSetInfo", uAddr(API_EXPORT::mmioSetInfo));
		FixSingleApi("mmioStringToFOURCCA", uAddr(API_EXPORT::mmioStringToFOURCCA));
		FixSingleApi("mmioStringToFOURCCW", uAddr(API_EXPORT::mmioStringToFOURCCW));
		FixSingleApi("mmioWrite", uAddr(API_EXPORT::mmioWrite));
		FixSingleApi("mmsystemGetVersion", uAddr(API_EXPORT::mmsystemGetVersion));
		FixSingleApi("sndPlaySoundA", uAddr(API_EXPORT::sndPlaySoundA));
		FixSingleApi("sndPlaySoundW", uAddr(API_EXPORT::sndPlaySoundW));
		FixSingleApi("timeBeginPeriod", uAddr(API_EXPORT::timeBeginPeriod));
		FixSingleApi("timeEndPeriod", uAddr(API_EXPORT::timeEndPeriod));
		FixSingleApi("timeGetDevCaps", uAddr(API_EXPORT::timeGetDevCaps));
		FixSingleApi("timeGetSystemTime", uAddr(API_EXPORT::timeGetSystemTime));
		FixSingleApi("timeGetTime", uAddr(API_EXPORT::timeGetTime));
		FixSingleApi("timeKillEvent", uAddr(API_EXPORT::timeKillEvent));
		FixSingleApi("timeSetEvent", uAddr(API_EXPORT::timeSetEvent));
		FixSingleApi("waveInAddBuffer", uAddr(API_EXPORT::waveInAddBuffer));
		FixSingleApi("waveInClose", uAddr(API_EXPORT::waveInClose));
		FixSingleApi("waveInGetDevCapsA", uAddr(API_EXPORT::waveInGetDevCapsA));
		FixSingleApi("waveInGetDevCapsW", uAddr(API_EXPORT::waveInGetDevCapsW));
		FixSingleApi("waveInGetErrorTextA", uAddr(API_EXPORT::waveInGetErrorTextA));
		FixSingleApi("waveInGetErrorTextW", uAddr(API_EXPORT::waveInGetErrorTextW));
		FixSingleApi("waveInGetID", uAddr(API_EXPORT::waveInGetID));
		FixSingleApi("waveInGetNumDevs", uAddr(API_EXPORT::waveInGetNumDevs));
		FixSingleApi("waveInGetPosition", uAddr(API_EXPORT::waveInGetPosition));
		FixSingleApi("waveInMessage", uAddr(API_EXPORT::waveInMessage));
		FixSingleApi("waveInOpen", uAddr(API_EXPORT::waveInOpen));
		FixSingleApi("waveInPrepareHeader", uAddr(API_EXPORT::waveInPrepareHeader));
		FixSingleApi("waveInReset", uAddr(API_EXPORT::waveInReset));
		FixSingleApi("waveInStart", uAddr(API_EXPORT::waveInStart));
		FixSingleApi("waveInStop", uAddr(API_EXPORT::waveInStop));
		FixSingleApi("waveInUnprepareHeader", uAddr(API_EXPORT::waveInUnprepareHeader));
		FixSingleApi("waveOutBreakLoop", uAddr(API_EXPORT::waveOutBreakLoop));
		FixSingleApi("waveOutClose", uAddr(API_EXPORT::waveOutClose));
		FixSingleApi("waveOutGetDevCapsA", uAddr(API_EXPORT::waveOutGetDevCapsA));
		FixSingleApi("waveOutGetDevCapsW", uAddr(API_EXPORT::waveOutGetDevCapsW));
		FixSingleApi("waveOutGetErrorTextA", uAddr(API_EXPORT::waveOutGetErrorTextA));
		FixSingleApi("waveOutGetErrorTextW", uAddr(API_EXPORT::waveOutGetErrorTextW));
		FixSingleApi("waveOutGetID", uAddr(API_EXPORT::waveOutGetID));
		FixSingleApi("waveOutGetNumDevs", uAddr(API_EXPORT::waveOutGetNumDevs));
		FixSingleApi("waveOutGetPitch", uAddr(API_EXPORT::waveOutGetPitch));
		FixSingleApi("waveOutGetPlaybackRate", uAddr(API_EXPORT::waveOutGetPlaybackRate));
		FixSingleApi("waveOutGetPosition", uAddr(API_EXPORT::waveOutGetPosition));
		FixSingleApi("waveOutGetVolume", uAddr(API_EXPORT::waveOutGetVolume));
		FixSingleApi("waveOutMessage", uAddr(API_EXPORT::waveOutMessage));
		FixSingleApi("waveOutOpen", uAddr(API_EXPORT::waveOutOpen));
		FixSingleApi("waveOutPause", uAddr(API_EXPORT::waveOutPause));
		FixSingleApi("waveOutPrepareHeader", uAddr(API_EXPORT::waveOutPrepareHeader));
		FixSingleApi("waveOutReset", uAddr(API_EXPORT::waveOutReset));
		FixSingleApi("waveOutRestart", uAddr(API_EXPORT::waveOutRestart));
		FixSingleApi("waveOutSetPitch", uAddr(API_EXPORT::waveOutSetPitch));
		FixSingleApi("waveOutSetPlaybackRate", uAddr(API_EXPORT::waveOutSetPlaybackRate));
		FixSingleApi("waveOutSetVolume", uAddr(API_EXPORT::waveOutSetVolume));
		FixSingleApi("waveOutUnprepareHeader", uAddr(API_EXPORT::waveOutUnprepareHeader));
		FixSingleApi("waveOutWrite", uAddr(API_EXPORT::waveOutWrite));
#ifdef __BAK__PlaySound
#define PlaySound __BAK__PlaySound
#undef __BAK__PlaySound
#endif
	}
#endif
#pragma endregion
}

void StartPatch()
{
	dprintf("StartPatch called...\n");

#ifdef __XP_PATCH_IN_THREAD
	HMODULE hBase = GetModuleHandle(nullptr);
	if (IsWindowsVistaOrGreater())
	{
		do_patch(hBase);
	}
	else
	{
		CreateThread(nullptr, 0,
			LPTHREAD_START_ROUTINE(do_patch), hBase, 0, nullptr);
	}
#else
	do_patch(GetModuleHandle(nullptr));
#endif

	dprintf("All done.\n==============================================\n");
}

void ExitLibrary() {
	FreeLibrary(hDll);
}


void FixSingleApi(LPCSTR lpProcName, uAddr fnEntry)
{
	// In x64 bit system, 
	// When compile as DEBUG, it will generate code like jmp xxxx
	// Which is not long enough to hold a 64-bit address.

	// Where as in RELEASE mode, it will just pass the *real* address
	// to this function.
#if defined(_WIN64) && defined(_DEBUG)
	fnEntry = ReadRelativeAddress(fnEntry + 1);
#endif

	FARPROC fnDest = GetProcAddress(hDll, lpProcName);
	if (fnEntry && fnDest)
	{
		DWORD oldProtect;
#ifdef _WIN64
#define SHELL_LENGTH 14
#else
#define SHELL_LENGTH 5
#endif
		VirtualProtect(reinterpret_cast<void *>(fnEntry), SHELL_LENGTH, PAGE_EXECUTE_READWRITE, &oldProtect);

#ifdef _WIN64

#if TRUE
		// TODO: Use less bytes.
		/*
		00007FFC7DADAA61 | 68 78 56 34 12                       | push LOW32
		00007FFC7DADAA66 | C7 44 24 04 78 56 34 12              | mov dword ptr ss:[rsp+4], HI32
		00007FFC7DADAA6E | C3                                   | ret
		*/

		ubyte szPayload[] = {
			0x68, 0, 0, 0, 0,
			0xC7, 0x44, 0x24, 0x04, 0, 0, 0, 0,
			0xC3
		};

		memcpy_s(LPVOID(fnEntry), sizeof(szPayload), szPayload, sizeof(szPayload));
		*LPDWORD(fnEntry + 1) = DWORD(uAddr(fnDest));
		*LPDWORD(fnEntry + 9) = DWORD(uAddr(fnDest) >> 32);

#else
		// 00007FFD7709E251 | 48 B8 56 34 12 90 78 56 34 12 | mov rax, 1234567890123456 |
		// 00007FFD7709E25B | FF E0 | jmp rax |

		WriteUShortRaw(fnEntry, 0xB848);
		*reinterpret_cast<FARPROC *>(fnEntry + 2) = fnDest;

		// jmp rax
		WriteUShortRaw(fnEntry + 10, 0xE0FF);
#endif



#else
		// 773E3D66 | E9 0D 19 F6 9A | jmp 12345678 |
		WriteByteRaw(fnEntry, 0xE9);
		WriteRelativeAddress(fnEntry + 1, uAddr(fnDest));
#endif
		VirtualProtect(reinterpret_cast<void *>(fnEntry), SHELL_LENGTH, oldProtect, &oldProtect);
	}
	else
	{
		//dprintf("Couldn't find export function %s: fnDest is null.\n", lpProcName);
	}
}

#pragma region API Export
namespace API_EXPORT {
	inline void PLACE_HOLDER(DWORD n)
	{
		SleepEx(n, TRUE);
	}

#ifdef __EXP_VERSION
	// Create some padding for the function.
	dll_export GetFileVersionInfoA(void) { PLACE_HOLDER(1); }
	dll_export GetFileVersionInfoByHandle(void) { PLACE_HOLDER(2); }
	dll_export GetFileVersionInfoExW(void) { PLACE_HOLDER(3); }
	dll_export GetFileVersionInfoSizeA(void) { PLACE_HOLDER(4); }
	dll_export GetFileVersionInfoSizeExW(void) { PLACE_HOLDER(5); }
	dll_export GetFileVersionInfoSizeW(void) { PLACE_HOLDER(6); }
	dll_export GetFileVersionInfoW(void) { PLACE_HOLDER(7); }
	dll_export VerFindFileA(void) { PLACE_HOLDER(8); }
	dll_export VerFindFileW(void) { PLACE_HOLDER(9); }
	dll_export VerInstallFileA(void) { PLACE_HOLDER(10); }
	dll_export VerInstallFileW(void) { PLACE_HOLDER(11); }
	dll_export VerLanguageNameA(void) { PLACE_HOLDER(12); }
	dll_export VerLanguageNameW(void) { PLACE_HOLDER(13); }
	dll_export VerQueryValueA(void) { PLACE_HOLDER(14); }
	dll_export VerQueryValueW(void) { PLACE_HOLDER(15); }
	dll_export VerQueryValueIndexA(void) { PLACE_HOLDER(16); }
	dll_export VerQueryValueIndexW(void) { PLACE_HOLDER(17); };
#endif

#ifdef __EXP_LPK
	dll_export LpkDllInitialize(void) { PLACE_HOLDER(19); };
	dll_export LpkDrawTextEx(void) { PLACE_HOLDER(20); }
	var_export VOID(*LpkEditControl[14])() = { 0 };
	dll_export LpkExtTextOut(void) { PLACE_HOLDER(22); }
	dll_export LpkGetCharacterPlacement(void) { PLACE_HOLDER(23); }
	dll_export LpkGetTextExtentExPoint(void) { PLACE_HOLDER(24); }
	dll_export LpkInitialize(void) { PLACE_HOLDER(25); }
	dll_export LpkPSMTextOut(void) { PLACE_HOLDER(26); }
	dll_export LpkTabbedTextOut(void) { PLACE_HOLDER(27); }
	dll_export LpkUseGDIWidthCache(void) { PLACE_HOLDER(28); }
	dll_export ftsWordBreak(void) { PLACE_HOLDER(29); }
#endif

#ifdef __EXP_MSIMG32
	dll_export AlphaBlend(void) { PLACE_HOLDER(300); };
	dll_export DllInitialize(void) { PLACE_HOLDER(301); };
	dll_export GradientFill(void) { PLACE_HOLDER(302); };
	dll_export TransparentBlt(void) { PLACE_HOLDER(303); };
	dll_export vSetDdrawflag(void) { PLACE_HOLDER(304); };
#endif

#ifdef __EXP_WINMM
#ifdef PlaySound
#ifndef __BAK__PlaySound
#define __BAK__PlaySound PlaySound
#endif
#undef PlaySound
#endif

	dll_export mciExecute(void) { PLACE_HOLDER(30); }
	dll_export CloseDriver(void) { PLACE_HOLDER(31); }
	dll_export DefDriverProc(void) { PLACE_HOLDER(32); }
	dll_export DriverCallback(void) { PLACE_HOLDER(33); }
	dll_export DrvGetModuleHandle(void) { PLACE_HOLDER(34); }
	dll_export GetDriverModuleHandle(void) { PLACE_HOLDER(35); }
	dll_export OpenDriver(void) { PLACE_HOLDER(36); }
	dll_export PlaySound(void) { PLACE_HOLDER(37); }
	dll_export PlaySoundA(void) { PLACE_HOLDER(38); }
	dll_export PlaySoundW(void) { PLACE_HOLDER(39); }
	dll_export SendDriverMessage(void) { PLACE_HOLDER(40); }
	dll_export WOWAppExit(void) { PLACE_HOLDER(41); }
	dll_export auxGetDevCapsA(void) { PLACE_HOLDER(42); }
	dll_export auxGetDevCapsW(void) { PLACE_HOLDER(43); }
	dll_export auxGetNumDevs(void) { PLACE_HOLDER(44); }
	dll_export auxGetVolume(void) { PLACE_HOLDER(45); }
	dll_export auxOutMessage(void) { PLACE_HOLDER(46); }
	dll_export auxSetVolume(void) { PLACE_HOLDER(47); }
	dll_export joyConfigChanged(void) { PLACE_HOLDER(48); }
	dll_export joyGetDevCapsA(void) { PLACE_HOLDER(49); }
	dll_export joyGetDevCapsW(void) { PLACE_HOLDER(50); }
	dll_export joyGetNumDevs(void) { PLACE_HOLDER(51); }
	dll_export joyGetPos(void) { PLACE_HOLDER(52); }
	dll_export joyGetPosEx(void) { PLACE_HOLDER(53); }
	dll_export joyGetThreshold(void) { PLACE_HOLDER(54); }
	dll_export joyReleaseCapture(void) { PLACE_HOLDER(55); }
	dll_export joySetCapture(void) { PLACE_HOLDER(56); }
	dll_export joySetThreshold(void) { PLACE_HOLDER(57); }
	dll_export mciDriverNotify(void) { PLACE_HOLDER(58); }
	dll_export mciDriverYield(void) { PLACE_HOLDER(59); }
	dll_export mciFreeCommandResource(void) { PLACE_HOLDER(60); }
	dll_export mciGetCreatorTask(void) { PLACE_HOLDER(61); }
	dll_export mciGetDeviceIDA(void) { PLACE_HOLDER(62); }
	dll_export mciGetDeviceIDFromElementIDA(void) { PLACE_HOLDER(63); }
	dll_export mciGetDeviceIDFromElementIDW(void) { PLACE_HOLDER(64); }
	dll_export mciGetDeviceIDW(void) { PLACE_HOLDER(65); }
	dll_export mciGetDriverData(void) { PLACE_HOLDER(66); }
	dll_export mciGetErrorStringA(void) { PLACE_HOLDER(67); }
	dll_export mciGetErrorStringW(void) { PLACE_HOLDER(68); }
	dll_export mciGetYieldProc(void) { PLACE_HOLDER(69); }
	dll_export mciLoadCommandResource(void) { PLACE_HOLDER(70); }
	dll_export mciSendCommandA(void) { PLACE_HOLDER(71); }
	dll_export mciSendCommandW(void) { PLACE_HOLDER(72); }
	dll_export mciSendStringA(void) { PLACE_HOLDER(73); }
	dll_export mciSendStringW(void) { PLACE_HOLDER(74); }
	dll_export mciSetDriverData(void) { PLACE_HOLDER(75); }
	dll_export mciSetYieldProc(void) { PLACE_HOLDER(76); }
	dll_export midiConnect(void) { PLACE_HOLDER(77); }
	dll_export midiDisconnect(void) { PLACE_HOLDER(78); }
	dll_export midiInAddBuffer(void) { PLACE_HOLDER(79); }
	dll_export midiInClose(void) { PLACE_HOLDER(80); }
	dll_export midiInGetDevCapsA(void) { PLACE_HOLDER(81); }
	dll_export midiInGetDevCapsW(void) { PLACE_HOLDER(82); }
	dll_export midiInGetErrorTextA(void) { PLACE_HOLDER(83); }
	dll_export midiInGetErrorTextW(void) { PLACE_HOLDER(84); }
	dll_export midiInGetID(void) { PLACE_HOLDER(85); }
	dll_export midiInGetNumDevs(void) { PLACE_HOLDER(86); }
	dll_export midiInMessage(void) { PLACE_HOLDER(87); }
	dll_export midiInOpen(void) { PLACE_HOLDER(88); }
	dll_export midiInPrepareHeader(void) { PLACE_HOLDER(89); }
	dll_export midiInReset(void) { PLACE_HOLDER(90); }
	dll_export midiInStart(void) { PLACE_HOLDER(91); }
	dll_export midiInStop(void) { PLACE_HOLDER(92); }
	dll_export midiInUnprepareHeader(void) { PLACE_HOLDER(93); }
	dll_export midiOutCacheDrumPatches(void) { PLACE_HOLDER(94); }
	dll_export midiOutCachePatches(void) { PLACE_HOLDER(95); }
	dll_export midiOutClose(void) { PLACE_HOLDER(96); }
	dll_export midiOutGetDevCapsA(void) { PLACE_HOLDER(97); }
	dll_export midiOutGetDevCapsW(void) { PLACE_HOLDER(98); }
	dll_export midiOutGetErrorTextA(void) { PLACE_HOLDER(99); }
	dll_export midiOutGetErrorTextW(void) { PLACE_HOLDER(100); }
	dll_export midiOutGetID(void) { PLACE_HOLDER(101); }
	dll_export midiOutGetNumDevs(void) { PLACE_HOLDER(102); }
	dll_export midiOutGetVolume(void) { PLACE_HOLDER(103); }
	dll_export midiOutLongMsg(void) { PLACE_HOLDER(104); }
	dll_export midiOutMessage(void) { PLACE_HOLDER(105); }
	dll_export midiOutOpen(void) { PLACE_HOLDER(106); }
	dll_export midiOutPrepareHeader(void) { PLACE_HOLDER(107); }
	dll_export midiOutReset(void) { PLACE_HOLDER(108); }
	dll_export midiOutSetVolume(void) { PLACE_HOLDER(109); }
	dll_export midiOutShortMsg(void) { PLACE_HOLDER(110); }
	dll_export midiOutUnprepareHeader(void) { PLACE_HOLDER(111); }
	dll_export midiStreamClose(void) { PLACE_HOLDER(112); }
	dll_export midiStreamOpen(void) { PLACE_HOLDER(113); }
	dll_export midiStreamOut(void) { PLACE_HOLDER(114); }
	dll_export midiStreamPause(void) { PLACE_HOLDER(115); }
	dll_export midiStreamPosition(void) { PLACE_HOLDER(116); }
	dll_export midiStreamProperty(void) { PLACE_HOLDER(117); }
	dll_export midiStreamRestart(void) { PLACE_HOLDER(118); }
	dll_export midiStreamStop(void) { PLACE_HOLDER(119); }
	dll_export mixerClose(void) { PLACE_HOLDER(120); }
	dll_export mixerGetControlDetailsA(void) { PLACE_HOLDER(121); }
	dll_export mixerGetControlDetailsW(void) { PLACE_HOLDER(122); }
	dll_export mixerGetDevCapsA(void) { PLACE_HOLDER(123); }
	dll_export mixerGetDevCapsW(void) { PLACE_HOLDER(124); }
	dll_export mixerGetID(void) { PLACE_HOLDER(125); }
	dll_export mixerGetLineControlsA(void) { PLACE_HOLDER(126); }
	dll_export mixerGetLineControlsW(void) { PLACE_HOLDER(127); }
	dll_export mixerGetLineInfoA(void) { PLACE_HOLDER(128); }
	dll_export mixerGetLineInfoW(void) { PLACE_HOLDER(129); }
	dll_export mixerGetNumDevs(void) { PLACE_HOLDER(130); }
	dll_export mixerMessage(void) { PLACE_HOLDER(131); }
	dll_export mixerOpen(void) { PLACE_HOLDER(132); }
	dll_export mixerSetControlDetails(void) { PLACE_HOLDER(133); }
	dll_export mmDrvInstall(void) { PLACE_HOLDER(134); }
	dll_export mmGetCurrentTask(void) { PLACE_HOLDER(135); }
	dll_export mmTaskBlock(void) { PLACE_HOLDER(136); }
	dll_export mmTaskCreate(void) { PLACE_HOLDER(137); }
	dll_export mmTaskSignal(void) { PLACE_HOLDER(138); }
	dll_export mmTaskYield(void) { PLACE_HOLDER(139); }
	dll_export mmioAdvance(void) { PLACE_HOLDER(140); }
	dll_export mmioAscend(void) { PLACE_HOLDER(141); }
	dll_export mmioClose(void) { PLACE_HOLDER(142); }
	dll_export mmioCreateChunk(void) { PLACE_HOLDER(143); }
	dll_export mmioDescend(void) { PLACE_HOLDER(144); }
	dll_export mmioFlush(void) { PLACE_HOLDER(145); }
	dll_export mmioGetInfo(void) { PLACE_HOLDER(146); }
	dll_export mmioInstallIOProcA(void) { PLACE_HOLDER(147); }
	dll_export mmioInstallIOProcW(void) { PLACE_HOLDER(148); }
	dll_export mmioOpenA(void) { PLACE_HOLDER(149); }
	dll_export mmioOpenW(void) { PLACE_HOLDER(150); }
	dll_export mmioRead(void) { PLACE_HOLDER(151); }
	dll_export mmioRenameA(void) { PLACE_HOLDER(152); }
	dll_export mmioRenameW(void) { PLACE_HOLDER(153); }
	dll_export mmioSeek(void) { PLACE_HOLDER(154); }
	dll_export mmioSendMessage(void) { PLACE_HOLDER(155); }
	dll_export mmioSetBuffer(void) { PLACE_HOLDER(156); }
	dll_export mmioSetInfo(void) { PLACE_HOLDER(157); }
	dll_export mmioStringToFOURCCA(void) { PLACE_HOLDER(158); }
	dll_export mmioStringToFOURCCW(void) { PLACE_HOLDER(159); }
	dll_export mmioWrite(void) { PLACE_HOLDER(160); }
	dll_export mmsystemGetVersion(void) { PLACE_HOLDER(161); }
	dll_export sndPlaySoundA(void) { PLACE_HOLDER(162); }
	dll_export sndPlaySoundW(void) { PLACE_HOLDER(163); }
	dll_export timeBeginPeriod(void) { PLACE_HOLDER(164); }
	dll_export timeEndPeriod(void) { PLACE_HOLDER(165); }
	dll_export timeGetDevCaps(void) { PLACE_HOLDER(166); }
	dll_export timeGetSystemTime(void) { PLACE_HOLDER(167); }
	dll_export timeGetTime(void) { PLACE_HOLDER(168); }
	dll_export timeKillEvent(void) { PLACE_HOLDER(169); }
	dll_export timeSetEvent(void) { PLACE_HOLDER(170); }
	dll_export waveInAddBuffer(void) { PLACE_HOLDER(171); }
	dll_export waveInClose(void) { PLACE_HOLDER(172); }
	dll_export waveInGetDevCapsA(void) { PLACE_HOLDER(173); }
	dll_export waveInGetDevCapsW(void) { PLACE_HOLDER(174); }
	dll_export waveInGetErrorTextA(void) { PLACE_HOLDER(175); }
	dll_export waveInGetErrorTextW(void) { PLACE_HOLDER(176); }
	dll_export waveInGetID(void) { PLACE_HOLDER(177); }
	dll_export waveInGetNumDevs(void) { PLACE_HOLDER(178); }
	dll_export waveInGetPosition(void) { PLACE_HOLDER(179); }
	dll_export waveInMessage(void) { PLACE_HOLDER(180); }
	dll_export waveInOpen(void) { PLACE_HOLDER(181); }
	dll_export waveInPrepareHeader(void) { PLACE_HOLDER(182); }
	dll_export waveInReset(void) { PLACE_HOLDER(183); }
	dll_export waveInStart(void) { PLACE_HOLDER(184); }
	dll_export waveInStop(void) { PLACE_HOLDER(185); }
	dll_export waveInUnprepareHeader(void) { PLACE_HOLDER(186); }
	dll_export waveOutBreakLoop(void) { PLACE_HOLDER(187); }
	dll_export waveOutClose(void) { PLACE_HOLDER(188); }
	dll_export waveOutGetDevCapsA(void) { PLACE_HOLDER(189); }
	dll_export waveOutGetDevCapsW(void) { PLACE_HOLDER(190); }
	dll_export waveOutGetErrorTextA(void) { PLACE_HOLDER(191); }
	dll_export waveOutGetErrorTextW(void) { PLACE_HOLDER(192); }
	dll_export waveOutGetID(void) { PLACE_HOLDER(193); }
	dll_export waveOutGetNumDevs(void) { PLACE_HOLDER(194); }
	dll_export waveOutGetPitch(void) { PLACE_HOLDER(195); }
	dll_export waveOutGetPlaybackRate(void) { PLACE_HOLDER(196); }
	dll_export waveOutGetPosition(void) { PLACE_HOLDER(197); }
	dll_export waveOutGetVolume(void) { PLACE_HOLDER(198); }
	dll_export waveOutMessage(void) { PLACE_HOLDER(199); }
	dll_export waveOutOpen(void) { PLACE_HOLDER(200); }
	dll_export waveOutPause(void) { PLACE_HOLDER(201); }
	dll_export waveOutPrepareHeader(void) { PLACE_HOLDER(202); }
	dll_export waveOutReset(void) { PLACE_HOLDER(203); }
	dll_export waveOutRestart(void) { PLACE_HOLDER(204); }
	dll_export waveOutSetPitch(void) { PLACE_HOLDER(205); }
	dll_export waveOutSetPlaybackRate(void) { PLACE_HOLDER(206); }
	dll_export waveOutSetVolume(void) { PLACE_HOLDER(207); }
	dll_export waveOutUnprepareHeader(void) { PLACE_HOLDER(208); }
	dll_export waveOutWrite(void) { PLACE_HOLDER(209); }

	#ifdef __BAK__PlaySound
		#define PlaySound __BAK__PlaySound
		#undef __BAK__PlaySound
	#endif
#endif

}
#pragma endregion