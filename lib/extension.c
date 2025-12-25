#include <dragonruby.h>
#include <mruby.h>
#include <mruby/array.h>
#include <stdio.h>
#include <stdlib.h>

static drb_api_t *drb_api;
mrb_state *global_state;  // Make it global so signal handler can access it

DRB_FFI_EXPORT
void drb_register_c_extensions_with_api(mrb_state *state, struct drb_api_t *api) {
    drb_api = api;
    struct RClass *SteamAchievements = drb_api->mrb_define_module(state, "SteamAchievements");
    // Cleanup
    // drb_api->mrb_state_atexit(state, file_dialog_mrb_at_exit);
    // atexit(file_dialog_at_exit);
    printf("* INFO: C extension 'SteamAchievements' registration completed.\n");
}
