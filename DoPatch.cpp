#include "Common.h"
#include "DoPatch.h"
#include "PatchUtil.h"
#include "SnR_Engine.h"
#include "DetourXS/detourxs.h"

#define __ENABLE_CONSOLE 1

uAddr uBase;

void do_patch(HMODULE base) {
	dprintf("Let's patch!\n");
    printf("cocks!\n");

	wchar_t szExePath[MAX_PATH];
	GetModuleFileName(nullptr, szExePath, MAX_PATH);
	dwprintf(L"szExePath: %s\n", szExePath);
	if (0 != wcscmp(wcsrchr(szExePath, L'\\'), L"\\factorio.exe")) {
		// Not my_target
		dwprintf(L"Is not my target: %s\n", wcsrchr(szExePath, L'\\') + 1);
		return;
	}

	//uBase = uAddr(base);

	// Method 1:
	// Just write some random stuff to target memory.
	//WriteMem(uBase + 0xcafe, uint(0xdeadbeaf));
	//WriteUInt(uBase + 0xcafe, 0xdeadbeaf);

	// Method 2:
	// Use search and replace engine
	// to create a more generic patch, 
	// that may work with a future version.
	auto engine = new SnR_Engine::SnR_Engine(base);
    ubyte patchSig1[] = {
            SnR_Engine::SearchMode_Search, 12,
            0x48, 0x8b, 0x9b, 0xa0, 0x03, 0x00, 0x00, 0x48, 0x85, 0xdb, 0x74, 0x64,
            SnR_Engine::SearchMode_EOF
    };

    // Patch data begin from where search signature begin.
	ubyte patchData1[] = {
		SnR_Engine::SearchMode_Skip, 10,

		// jnz -> jmp
		SnR_Engine::SearchMode_Replace, 1,
		0xEB,


		SnR_Engine::SearchMode_EOF
	};

	if (engine->doSearchAndReplace(patchSig1, patchData1) == 0)
	{
		dprintf("Didn't patch 1.\n");
	}

    ubyte patchSig2[] = {
            SnR_Engine::SearchMode_Search, 32,
            0x48, 0x3b, 0xc2, 0x74, 0x1f, 0x48, 0x8b, 0x08, 0x80, 0x79, 0x3e, 0x00, 0x74, 0x0c, 0x80, 0x79, 0x40, 0x00, 0x74, 0x06, 0x80, 0x79, 0x41, 0x00, 0x74, 0x06, 0x48, 0x83, 0xc0, 0x08, 0xeb, 0xe0,
            SnR_Engine::SearchMode_EOF
    };
    ubyte patchData2[] = {
            SnR_Engine::SearchMode_Skip, 3,

            // jz -> jnz
            SnR_Engine::SearchMode_Replace, 1,
            0x75,


            SnR_Engine::SearchMode_EOF
    };
    if (engine->doSearchAndReplace(patchSig2, patchData2) == 0)
    {
        dprintf("Didn't patch 2.\n");
    }
    ubyte patchSig3[] = {
            SnR_Engine::SearchMode_Search, 9,
            0x48, 0x3b, 0xc2, 0x0f, 0x84, 0xe4, 0x01, 0x00, 0x00,
            SnR_Engine::SearchMode_EOF
    };
    ubyte patchData3[] = {
            SnR_Engine::SearchMode_Skip, 4,

            // jz -> jnz
            SnR_Engine::SearchMode_Replace, 1,
            0x85,


            SnR_Engine::SearchMode_EOF
    };
    if (engine->doSearchAndReplace(patchSig3, patchData3) == 0)
    {
        dprintf("Didn't patch 3.\n");
    }
}