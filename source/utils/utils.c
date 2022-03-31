#include "utils.h"

#include <lv2/sysfs.h>
#include <string.h>
#include <stdio.h>

const char* lbp1_ids[] = {
    "BCUS98148", //LittleBigPlanet™ [US] [Disc]
    "BCES00141", //LittleBigPlanet™ [EU] [Disc]
    "NPUA80472", //LittleBigPlanet™ [US] [PSN]
    "NPEA00241", //LittleBigPlanet™ [EU] [PSN]
    "BCUS98208", //LittleBigPlanet™ Game of the Year Edition [US] [Disc]
    "BCES00611", //LittleBigPlanet™ Game of the Year Edition [EU] [Disc]
};

const char* lbp2_ids[] = {
    "BCUS98245", //LittleBigPlanet™ 2 [US] [Disc]
    "BCES01086", //LittleBigPlanet™ 2 [EU] [Disc]
    "BCES00850", //LittleBigPlanet™ 2 [EU] [Disc]
    "BCAS20113", //LittleBigPlanet™ 2 [AS] [Disc]
    "NPUA80662", //LittleBigPlanet™ 2 [US] [PSN]
    "NPEA00324", //LittleBigPlanet™ 2 [EU] [PSN]
    "BCES01693", //LittleBigPlanet™ 2 Extras Edition [EU] [Disc]
};

const char* lbp3_ids[] = {
    "BCUS98362", //LittleBigPlanet™ 3 [US] [Disc]
    "BCES02068", //LittleBigPlanet™ 3 [EU] [Disc]
    "BCES01663", //LittleBigPlanet™ 3 [EU] [Disc]
    "NPUA81116" //LittleBigPlanet™ 3 [US] [PSN]
};

void get_installed_lbp_versions(
    bool* lbp1_versions, bool* lbp1_installed,
    bool* lbp2_versions, bool* lbp2_installed,
    bool* lbp3_versions, bool* lbp3_installed
)
{
    printf("Opening /dev_hdd0/game directory");
    s32 game_dir = -1;
    sysFsOpendir("/dev_hdd0/game/", &game_dir);

    sysFSDirectoryEntry entries[1024] = {};

    u32 count = 0;

    printf("Reading /dev_hdd0/game entries");
    sysFsGetDirectoryEntries(game_dir, entries, 1024, &count);

    for (int i = 0; i < count; i++) {
        printf("Found %s", entries[i].entry_name.d_name);
        if (lbp1_versions != NULL) {
            for (int j = 0; j < sizeof(lbp1_ids) / sizeof(lbp1_ids[0]); j++) {
                if (!strcmp(entries[i].entry_name.d_name, lbp1_ids[j])) {
                    if (lbp1_installed != NULL) {
                        *lbp1_installed = true;
                    }

                    lbp1_versions[j] = true;
                    printf("Found LBP1!");
                }
            }
        }

        if (lbp2_versions != NULL) {
            for (int j = 0; j < sizeof(lbp2_ids) / sizeof(lbp2_ids[0]); j++) {
                if (!strcmp(entries[i].entry_name.d_name, lbp2_ids[j])) {
                    if (lbp2_installed != NULL) {
                        *lbp2_installed = true;
                    }

                    lbp2_versions[j] = true;
                }
            }
        }

        if (lbp3_versions != NULL) {
            for (int j = 0; j < sizeof(lbp3_ids) / sizeof(lbp3_ids[0]); j++) {
                if (!strcmp(entries[i].entry_name.d_name, lbp3_ids[j])) {
                    if (lbp3_installed != NULL) {
                        *lbp3_installed = true;
                    }

                    lbp3_versions[j] = true;
                }
            }
        }
    }
}
