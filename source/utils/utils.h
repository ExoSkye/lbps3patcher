#pragma once

#include <stdbool.h>

extern const char* lbp1_ids[];

extern const char* lbp2_ids[];

extern const char* lbp3_ids[];

void get_installed_lbp_versions(bool* lbp1_versions, bool* lbp1_installed,
                                bool* lbp2_versions, bool* lbp2_installed,
                                bool* lbp3_versions, bool* lbp3_installed);