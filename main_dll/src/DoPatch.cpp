#include "Common.h"
#include "DoPatch.h"
//#include "PatchUtil.h"
#include "SnR_Engine.h"
#include "DetourXS/detourxs.h"
#include <vector>

#define __ENABLE_CONSOLE 1

uAddr uBase;

void do_patch(HMODULE base) {
  dprintf("do_patch called...\n");

  wchar_t szExePath[MAX_PATH];
  GetModuleFileName(nullptr, szExePath, MAX_PATH);
  dwprintf(L"szExePath: %s\n", szExePath);
  if (0 != _wcsicmp(wcsrchr(szExePath, L'\\'), L"\\factorio.exe")) {
    // Not my_target
    dwprintf(L"Is not my target: %s\n", wcsrchr(szExePath, L'\\') + 1);
    fflush( stdout );
    return;
  }
  // Search and replace with generic patterns
  auto engine = new SnR_Engine::SnR_Engine(base);
  // 80 79 ?? ?? 74 ?? 80 79 ?? ?? 74 ?? 80 79 ?? ?? 74 ?? matches a set of 3 jz, the first two apply if not modded, the last if modded. Patch the first and the rest are skipped.
  // variant: 80 78 ?? ?? 74 ?? 80 78 ?? ?? 74 ?? 80 78 ?? ?? 0f 84 ?? ?? ?? ??
  // These two patterns will find most callsites checking for mods
  std::vector<std::vector<ubyte>> searches = {
      // 1: AchievementStats::allowed (2 replacements), 48 8b 8f 58 04 00 00 48 85 c9 74 ??
      {
          SnR_Engine::SearchMode_Search, 11,
          0x48, 0x8b, 0x8f, 0x58, 0x04, 0x00, 0x00, 0x48, 0x85, 0xc9, 0x74,
          SnR_Engine::SearchMode_Skip, 1,
          SnR_Engine::SearchMode_EOF
      },
      // 2: PlayerData::PlayerData (while loop), 48 3B C2 74 1F 48 8B 08 80 79 3E 00 74 0C 80 79 40 00 74 06 80 79 41 00 74 06 48 83 C0 08 EB E0
      {
          SnR_Engine::SearchMode_Search, 32,
          0x48, 0x3b, 0xc2, 0x74, 0x1f, 0x48, 0x8b, 0x08, 0x80, 0x79, 0x3e, 0x00, 0x74, 0x0c, 0x80, 0x79, 0x40, 0x00,
          0x74, 0x06, 0x80, 0x79, 0x41, 0x00, 0x74, 0x06, 0x48, 0x83, 0xc0, 0x08, 0xeb, 0xe0,
          SnR_Engine::SearchMode_EOF
      },
      // 3: PlayerData::PlayerData (for loop), 49 3b d0 74 ?? 48 8b 02 80 78 [rest of pattern not necessary] ?? ?? 74 ?? 80 78 ?? ?? 74 ?? 80 78 ?? ?? 74 ??
      {
          SnR_Engine::SearchMode_Search, 4,
          0x49, 0x3b, 0xd0, 0x74,
          SnR_Engine::SearchMode_Skip, 1,
          SnR_Engine::SearchMode_Search, 5,
          0x48, 0x8b, 0x02, 0x80, 0x78,
          SnR_Engine::SearchMode_EOF
      },
      /* 4: AchievementGUI::updateModdedLabel, 48 8b 02 44 38 48 ?? 74 ?? 44 38 48 ?? 74 ?? 44 38 48 ??
       * For some reason the jmp (0F 84 DF 01 00 00) becomes 0F 84 19 02 00 00 in running memory */
      {
          SnR_Engine::SearchMode_Search, 6,
          0x48, 0x8b, 0x02, 0x44, 0x38, 0x48,
          SnR_Engine::SearchMode_Skip, 1,
          SnR_Engine::SearchMode_Search, 1,
          0x74,
          SnR_Engine::SearchMode_Skip, 1,
          SnR_Engine::SearchMode_Search, 3,
          0x44, 0x38, 0x48,
          SnR_Engine::SearchMode_Skip, 1,
          SnR_Engine::SearchMode_Search,1,
          0x74,
          SnR_Engine::SearchMode_Skip, 1,
          SnR_Engine::SearchMode_Search, 3,
          0x44, 0x38, 0x48,
          SnR_Engine::SearchMode_EOF
      },
      // 5: SteamContext::onUserStatsReceived, 8B ?? 08 ?? 3B ?? 74 22 48 8B 01 80 78
      {
          SnR_Engine::SearchMode_Search, 1,
          0x8B,
          SnR_Engine::SearchMode_Skip, 1,
          SnR_Engine::SearchMode_Search, 1,
          0x08,
          SnR_Engine::SearchMode_Skip, 1,
          SnR_Engine::SearchMode_Search, 1,
          0x3B,
          SnR_Engine::SearchMode_Skip, 1,
          SnR_Engine::SearchMode_Search, 7,
          0x74, 0x22, 0x48, 0x8B, 0x01, 0x80, 0x78,
          SnR_Engine::SearchMode_EOF
      },
      /* 6: SteamContext::setStat, SteamContext::unlockAchievement (2 matches), C2 74 1E 48 8B 08 80 79 3E 00 74 0C 80 79 40 00 74 06 80 79 41 00 74
       * There's a third match here within ModManager::isVanilla
       * isVanilla is only called (currently) to hide the "alternative reverse select" bind within the settings GUI
       * if we disable the check, it won't show even with mods enabled (breaking the default behavior)
       */
      {
          SnR_Engine::SearchMode_Search, 23,
          0xc2, 0x74, 0x1e, 0x48, 0x8b, 0x08, 0x80, 0x79, 0x3e, 0x00, 0x74, 0x0c, 0x80, 0x79, 0x40, 0x00, 0x74, 0x06, 0x80, 0x79, 0x41, 0x00, 0x74,
          SnR_Engine::SearchMode_EOF
      },
      // 7: SteamContext::unlockAchievementsThatAreOnSteamButArentActivatedLocally, 48 8B 02 80 78 3E 00 74 10 80 78 40 00 74
      {
          SnR_Engine::SearchMode_Search, 14,
          0x48, 0x8b, 0x02, 0x80, 0x78, 0x3e, 0x00, 0x74, 0x10, 0x80, 0x78, 0x40, 0x00, 0x74,
          SnR_Engine::SearchMode_EOF
      },
      /* 8: ControlSettings:ControlSettings e8 4b 96 8c ff 84 c0 74 ??
       * Once again, the call address is different during runtime
       * call -> test -> jz (74 0F) -> xor -> lea r12+0B548 (the lea is our easy match)
       * This just skips the isVanilla call we break above :^)
       * I'm a good programmer sometimes, but not today */
      {
          SnR_Engine::SearchMode_Search, 8,
          0xe8, 0x4b, 0x96, 0x8c, 0xff, 0x84, 0xc0, 0x74,
          SnR_Engine::SearchMode_EOF
      },
  };
  std::vector<std::vector<ubyte>> patches = {
      //1
      {
          SnR_Engine::SearchMode_Skip, 10,
          // jz -> jmp
          SnR_Engine::SearchMode_Replace, 1,
          0xEB,
          SnR_Engine::SearchMode_EOF
      },
      //2
      {
          SnR_Engine::SearchMode_Skip, 3,
          // jz -> jmp
          SnR_Engine::SearchMode_Replace, 1,
          0xEB,
          SnR_Engine::SearchMode_EOF
      },
      //3
      {
          SnR_Engine::SearchMode_Skip, 3,
          // jz -> jmp
          SnR_Engine::SearchMode_Replace, 1,
          0xEB,
          SnR_Engine::SearchMode_EOF
      },
      //4
      {
          SnR_Engine::SearchMode_Skip, 7,
          // jz -> jmp
          SnR_Engine::SearchMode_Replace, 1,
          0xEB,
          SnR_Engine::SearchMode_EOF
      },
      //5
      {
          SnR_Engine::SearchMode_Skip, 6,
          // jz -> jnz
          SnR_Engine::SearchMode_Replace, 1,
          0x75,
          SnR_Engine::SearchMode_EOF
      },
      //6
      {
          SnR_Engine::SearchMode_Skip, 10,
          // jz -> jmp
          SnR_Engine::SearchMode_Replace, 1,
          0xEB,
          SnR_Engine::SearchMode_EOF
      },
      //7
      {
          SnR_Engine::SearchMode_Skip, 13,
          // jz -> jmp
          SnR_Engine::SearchMode_Replace, 1,
          0xEB,
          SnR_Engine::SearchMode_EOF
      },
      //8
      {
          SnR_Engine::SearchMode_Skip, 7,
          // jz -> jmp
          SnR_Engine::SearchMode_Replace, 1,
          0xEB,
          SnR_Engine::SearchMode_EOF
      },
  };

  int max_i = (int)(searches.size());

  for (int i = 0; i < max_i; i++) {
    ubyte *search[] = {searches[i].data()};
    ubyte *patch[] = {patches[i].data()};
    if (engine->doSearchAndReplace(*search, *patch) == 0) {
      dprintf("Didn't patch %i of %i.\n", (i+1), max_i);
    }else{
      dprintf("Patched %i of %i.\n", (i+1), max_i);
    }
  }
  fflush( stdout );
}
