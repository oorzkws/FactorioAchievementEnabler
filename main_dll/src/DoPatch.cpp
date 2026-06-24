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
  // e.g. `if ( *(_BYTE *)(*v3 + 0x3E) && *(_BYTE *)(v5 + 0x40) && !*(_BYTE *)(v5 + 0x41) )`
  // These two patterns will find most callsites checking for mods
  std::vector<std::vector<ubyte>> searches = {
      // 1: AchievementStats::allowed (1 replacement), 75 ?? 80 bf ?? ?? ?? ?? 00 75 ?? 48 8b 9f ?? ?? ?? ?? 48 85 DB 74
      // jnz / cmp / jnz / mov / test / jz
      // if (AssociatedContext) -> if (false). BUG: Trailing SearchMode_Skip makes match fail.
      {
          SnR_Engine::SearchMode_Search, 1,
          0x75,
          SnR_Engine::SearchMode_Skip, 1,
          SnR_Engine::SearchMode_Search, 2,
          0x80, 0xbf,
          SnR_Engine::SearchMode_Skip, 5,
          SnR_Engine::SearchMode_Search, 1,
          0x75,
          SnR_Engine::SearchMode_Skip, 1,
          SnR_Engine::SearchMode_Search, 3,
          0x48, 0x8b, 0x9f,
          SnR_Engine::SearchMode_Skip, 4,
          SnR_Engine::SearchMode_Search, 4,
          0x48, 0x85, 0xdb, 0x74,
          SnR_Engine::SearchMode_EOF
      },
      /* 2: SteamContext::setStat, SteamContext::unlockAchievement (2 replacements total) 48 8B 08 80 79 ?? ?? 74 ?? 80 79 ?? ?? 74 ?? 80 79 ?? ?? 74 ??
       * mov this, [rax] / cmp byte ptr [this+3Eh], 0 / jz short / cmp byte ptr [this+40h], 0 / jz short / cmp byte ptr [this+41h], 0 / jz short
       */
      {
          SnR_Engine::SearchMode_Search, 5,
          0x48, 0x8b, 0x08, 0x80, 0x79,
          SnR_Engine::SearchMode_Skip, 2,
          SnR_Engine::SearchMode_Search, 1,
          0x74,
          SnR_Engine::SearchMode_Skip, 1,
          SnR_Engine::SearchMode_Search, 2,
          0x80, 0x79,
          SnR_Engine::SearchMode_Skip, 2,
          SnR_Engine::SearchMode_Search, 1,
          0x74,
          SnR_Engine::SearchMode_Skip, 1,
          SnR_Engine::SearchMode_Search, 2,
          0x80, 0x79,
          SnR_Engine::SearchMode_Skip, 2,
          SnR_Engine::SearchMode_Search, 1,
          0x74,
          SnR_Engine::SearchMode_EOF
      },
      /* 3: AchievementGUI::updateModdedLabel, SteamContext::unlockAchievementsThatAreOnSteamButArentActivatedLocally (2 replacements total) 48 8b 02 80 78 ?? ?? 74 ?? 80 78 ?? ?? 74 ?? 80 78 ?? ??
        * Basically the same pattern as [2] but goes jz-jz-jnz instead of jz-jz-jz
        * *v7 == 0x646F6D5F74736574LL seems to indicate an internal mod */
      {
          SnR_Engine::SearchMode_Search, 5,
          0x48, 0x8b, 0x02, 0x80, 0x78,
          SnR_Engine::SearchMode_Skip, 2,
          SnR_Engine::SearchMode_Search, 1,
          0x74,
          SnR_Engine::SearchMode_Skip, 1,
          SnR_Engine::SearchMode_Search, 2,
          0x80, 0x78,
          SnR_Engine::SearchMode_Skip, 2,
          SnR_Engine::SearchMode_Search,1,
          0x74,
          SnR_Engine::SearchMode_Skip, 1,
          SnR_Engine::SearchMode_Search, 2,
          0x80, 0x78,
          SnR_Engine::SearchMode_EOF
      },
      // 4: SteamContext::onUserStatsReceived, 8B ?? 08 ?? 3B ?? 74 22 48 8B 01 80 78
      // cmp, jz, cmp, jz, cmp
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
      /* 5: PlayerData::PlayerData,  48 8d 0d ?? ?? ?? ?? 48 8d 15 ?? ?? ?? ?? 84 c0 48 0f 45 d1 49 8d 8d
       * lea "achievements.dat" / lea "achievements-modded.dat" / test / cmovnz / lea
       * The final lea (8d 8d) isn't related to our patch but necessary to find the right match
       * */
    {
        SnR_Engine::SearchMode_Search, 3,
        0x48, 0x8d, 0x0d,
        SnR_Engine::SearchMode_Skip, 4,
        SnR_Engine::SearchMode_Search, 3,
        0x48, 0x8d, 0x15,
        SnR_Engine::SearchMode_Skip, 4,
        SnR_Engine::SearchMode_Search, 9,
        0x84, 0xC0, 0x48, 0x0f, 0x45, 0xd1, 0x49, 0x8d, 0x8d,
        SnR_Engine::SearchMode_EOF
      },
      /* 6: PlayerData::PlayerData,  e8 ?? ?? ?? ?? 34 ?? 41 88 85 ?? ?? ?? ?? 48 8d 94 24 ?? ?? ?? ??
       * call / xor / mov
       * xor (34 ??) changed
       * v1->achievementsAreModded = !v9; */
    {
        SnR_Engine::SearchMode_Search, 1,
        0xe8,
        SnR_Engine::SearchMode_Skip, 4,
        SnR_Engine::SearchMode_Search, 1,
        0x34,
        SnR_Engine::SearchMode_Skip, 1,
        SnR_Engine::SearchMode_Search, 3,
        0x41, 0x88, 0x85,
        SnR_Engine::SearchMode_Skip, 4,
        SnR_Engine::SearchMode_Search, 4,
        0x48, 0x8d, 0x94, 0x24,
        SnR_Engine::SearchMode_EOF
    },
  };
  std::vector<std::vector<ubyte>> patches = {
      //1
      {
          SnR_Engine::SearchMode_Skip, 21,
          // jz -> jmp
          SnR_Engine::SearchMode_Replace, 1,
          0xEB,
          SnR_Engine::SearchMode_EOF
      },
      //2
      {
          SnR_Engine::SearchMode_Skip, 7,
          // jz -> jmp
          SnR_Engine::SearchMode_Replace, 1,
          0xEB,
          SnR_Engine::SearchMode_EOF
      },
      //3
      {
          SnR_Engine::SearchMode_Skip, 7,
          // jz -> jmp
          SnR_Engine::SearchMode_Replace, 1,
          0xEB,
          SnR_Engine::SearchMode_EOF
      },
      //4
      {
          SnR_Engine::SearchMode_Skip, 6,
          // jz -> jnz
          SnR_Engine::SearchMode_Replace, 1,
          0x75,
          SnR_Engine::SearchMode_EOF
      },
      //5
      {
          SnR_Engine::SearchMode_Skip, 16,
          // cmovnz -> nop, nop, nop, mov (r16/32, r/m16/32)
          SnR_Engine::SearchMode_Replace, 5,
          0x90, 0x48, 0x8B,
          SnR_Engine::SearchMode_EOF
      },
      //6
      {
          SnR_Engine::SearchMode_Skip, 5,
          // xor al, 0 -> xor al, al
          SnR_Engine::SearchMode_Replace, 2,
          0x30, 0xC0,
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
