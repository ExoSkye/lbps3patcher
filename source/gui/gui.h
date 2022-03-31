#pragma once
#include <stdbool.h>

typedef struct {

} GUI_State;

bool gui_init();

bool gui_update(GUI_State* state);

void gui_exit();