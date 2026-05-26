#pragma once

void signals_init(void);
void signals_restore_defaults_for_child(void);
int signals_has_sigchld(void);
void signals_clear_sigchld(void);