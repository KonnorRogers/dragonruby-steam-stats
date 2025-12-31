#include "steam/isteamuserstats.h"
#include <cstdlib>
#include <cstring>

extern "C" {
#include <mruby.h>
#include <mruby/array.h>
#include <dragonruby.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/sysctl.h>
#include <sys/types.h>
#include <unistd.h>
}

#include "./extension.h"

extern "C" {
    // Stupid silly hack to be able to use `OutputDebugString` from Steam's API page. Maybe not needed?
    // https://stackoverflow.com/a/417846
    bool IsDebuggerPresent() {
        int mib[4];
        struct kinfo_proc info;
        size_t size;

        info.kp_proc.p_flag = 0;
        mib[0] = CTL_KERN;
        mib[1] = KERN_PROC;
        mib[2] = KERN_PROC_PID;
        mib[3] = getpid();

        size = sizeof(info);
        sysctl(mib, sizeof(mib) / sizeof(*mib), &info, &size, NULL, 0);

        return ((info.kp_proc.p_flag & P_TRACED) != 0);
    }

    void OutputDebugString(const char *__restrict fmt, ...) {
        if( !IsDebuggerPresent() )
            return;

        va_list args;
        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);
    }

    static mrb_value _mrb_hash_or_class_value (mrb_state *state, mrb_value elem, mrb_value key) {
    	    auto is_hash = mrb_bool(mrb_check_hash_type(state, elem));
    	    if (is_hash) {
    	    	    return mrb_hash_get(state, elem, key);
    	    } else {
    	    	if (mrb_class_p(elem)) {
    	    		auto klass = mrb_class_ptr(elem);
    	    		auto sym = mrb_symbol(key);

    	    		if ( mrb_obj_respond_to(state, klass, sym) ) {
    	    			mrb_funcall_id(state, mrb_obj_value(klass), sym, MRB_ARGS_NONE());
    	    		}
    	    	}
    	    }

    	    return mrb_nil_value();
    }

}

// https://partner.steamgames.com/doc/api/ISteamGameServerStats <-- setting achieves from server
// https://partner.steamgames.com/doc/api/ISteamUserStats <-- setting achievements client side

CSteamAchievements::CSteamAchievements():
 app_id( 0 ),
 initialized( false ),
 callbackUserStatsReceived( this, &CSteamAchievements::OnUserStatsReceived ),
 callbackUserStatsStored( this, &CSteamAchievements::OnUserStatsStored ),
 callbackAchievementStored( this, &CSteamAchievements::OnAchievementStored )
{
     bool steam_initialized = SteamAPI_Init();
     if (steam_initialized) {
     	app_id = SteamUtils()->GetAppID();
     }

}

bool CSteamAchievements::UnlockAchievement(const char* ID)
{
	// Have we received a call back from Steam yet?
	if (initialized)
	{
		return SteamUserStats()->SetAchievement(ID);
	}
	// If not then we can't set achievements yet
	return false;
}

int CSteamAchievements::GetAchievementStatus(const char* ID) {
	bool isUnlocked;
	if (SteamUserStats()->GetAchievement(ID, &isUnlocked)) {
    		if (isUnlocked) {
    			return 1;
    		} else {
        		return 0;
    		}
	} else {
    		return -1;
	}
}

// Can get an arbitrary user's achievement status given their steamId (Not implemented)
int CSteamAchievements::GetUserAchievementStatus(CSteamID steamIDUser, const char* ID) {
	bool isUnlocked;
	if (SteamUserStats()->GetUserAchievement(steamIDUser, ID, &isUnlocked)) {
    		if (isUnlocked) {
    			return 1;
    		} else {
        		return 0;
    		}
	} else {
    		return -1;
	}
}

// This will flash the progress dialog
bool CSteamAchievements::IndicateAchievementProgress(const char *name, uint32 currentProgress, uint32 maxProgress) {

	if (!initialized) { return false; }

	return SteamUserStats()->IndicateAchievementProgress(name, currentProgress, maxProgress);
}


bool CSteamAchievements::ResetAllStats() {
	if (!initialized) {
		return false;
	}

	return SteamUserStats()->ResetAllStats(false);
}


bool CSteamAchievements::ResetAllStatsAndAchievements() {
	if (!initialized) {
		return false;
	}

	return SteamUserStats()->ResetAllStats(true);
}

bool CSteamAchievements::SetStat(const char *name, int32 data) {
	if (!initialized) { return false; }

	return SteamUserStats()->SetStat(name, data);
}

bool CSteamAchievements::SetStat(const char *name, float data) {
	if (!initialized) { return false; }

	return SteamUserStats()->SetStat(name, data);
}

bool CSteamAchievements::ClearAchievement(const char* ID)
{
	// Have we received a call back from Steam yet?
	if (!initialized) {
		// If not then we can't set achievements yet
		return false;
	}
	return SteamUserStats()->ClearAchievement(ID);
}

void CSteamAchievements::OnUserStatsReceived( UserStatsReceived_t *pCallback )
{
	// we may get callbacks for other games' stats arriving, ignore them
	if (app_id == pCallback->m_nGameID )
	{
		if ( k_EResultOK == pCallback->m_eResult )
		{
			OutputDebugString("Received stats and achievements from Steam\n");
		}
		else
		{
			char buffer[128];
			snprintf( buffer, 128, "RequestStats - failed, %d\n", pCallback->m_eResult );
			OutputDebugString( buffer );
		}
	}
}

void CSteamAchievements::OnUserStatsStored( UserStatsStored_t *pCallback )
{
	// we may get callbacks for other games' stats arriving, ignore them
	if ( app_id == pCallback->m_nGameID )
	{
		if ( k_EResultOK == pCallback->m_eResult )
		{
			OutputDebugString( "Stored stats for Steam\n" );
		}
		else
		{
			char buffer[128];
			snprintf( buffer, 128, "StatsStored - failed, %d\n", pCallback->m_eResult );
			OutputDebugString( buffer );
		}
	}
}

void CSteamAchievements::OnAchievementStored( UserAchievementStored_t *pCallback )
{
     // we may get callbacks for other games' stats arriving, ignore them
     if ( app_id == pCallback->m_nGameID )
     {
          OutputDebugString( "Stored Achievement for Steam\n" );
     }
}


extern "C" {
    CSteamAchievements *steamStats = new CSteamAchievements();

    static void steam_achievements_shutdown () {
    	    SteamAPI_Shutdown();
    }

    static void steam_achievements_mrb_shutdown (mrb_state *state) {
    	    SteamAPI_Shutdown();
    }

    static mrb_value steam_achievements_mrb_set_stat(mrb_state *state, mrb_value self) {
    		mrb_value name;
		mrb_value int_or_float;

		// https://github.com/mruby/mruby/blob/510ebd738dc1326fd8c9f3a702f0d826d7fb330a/src/class.c#L1704-L1723
		// expect 2 ints / floats
    		mrb_get_args(state, "ii", &name, &int_or_float);

		const char* c_str = mrb_str_to_cstr(state, name);

    		if (mrb_integer_p(int_or_float)) {
        		int32_t i32 = (int32_t)mrb_integer(int_or_float);
        		return mrb_bool_value(steamStats->SetStat(c_str, i32));
    		}
    		else if (mrb_float_p(int_or_float)) {
        		float c_float = (float)mrb_float(int_or_float);
        		return mrb_bool_value(steamStats->SetStat(c_str, c_float));
    		}

		// neither int or float given, just return false. (should we raise?)
		auto standard_error = mrb_class_get(state, "StandardError");
		mrb_raise(state, standard_error, "Float or Integer not given to SetStat");
    		return mrb_false_value();
    }

    static mrb_value steam_achievements_mrb_unlock_achievement(mrb_state *state, mrb_value self) {
		char *c_str;

		// CString
		mrb_get_args(state, "z", &c_str);

		return mrb_bool_value(steamStats->UnlockAchievement(c_str));
    }

    static mrb_value steam_achievements_mrb_indicate_achievement_progress(mrb_state *state, mrb_value self) {
		char *c_str;
		uint32 c_currentProgress;
		uint32 c_maxProgress;

		// CString, int, int
		mrb_get_args(state, "zii", &c_str, &c_currentProgress, &c_maxProgress);

		return mrb_bool_value(steamStats->IndicateAchievementProgress(c_str, c_currentProgress, c_maxProgress));
    }

    // Will "reset" an achievement
    static mrb_value steam_achievements_mrb_clear_achievement(mrb_state *state, mrb_value self) {
		char *c_str;

		// CString
		mrb_get_args(state, "z", &c_str);

		return mrb_bool_value(steamStats->ClearAchievement(c_str));
    }

    static mrb_value steam_achievements_mrb_reset_all_stats(mrb_state *state, mrb_value self) {
		return mrb_bool_value(steamStats->ResetAllStats());
    }

    static mrb_value steam_achievements_mrb_reset_all_stats_and_achievements(mrb_state *state, mrb_value self) {
		return mrb_bool_value(steamStats->ResetAllStatsAndAchievements());
    }

    static mrb_value steam_achievements_mrb_get_achievement_status(mrb_state *state, mrb_value self) {
		char *c_str;

		// CString
		mrb_get_args(state, "z", &c_str);

		return mrb_int_value(state, steamStats->GetAchievementStatus(c_str));
    }

    static mrb_value steam_achievements_mrb_store_stats(mrb_state *state, mrb_value self) {
		return mrb_bool_value(steamStats->StoreStats());
    }

    static drb_api_t *drb_api;
    mrb_state *global_state;

    DRB_FFI_EXPORT
    void drb_register_c_extensions_with_api(mrb_state *state, struct drb_api_t *api) {
        drb_api = api;
        struct RClass *SteamModule = drb_api->mrb_define_module(state, "Steam");
        struct RClass *SteamStats = drb_api->mrb_define_module_under(state, SteamModule, "Stats");

	// Singleton for steam achievements, may live to regret this, but fine for now.

	// SetStat
	mrb_define_module_function(state, SteamStats, "set_stat", steam_achievements_mrb_set_stat, MRB_ARGS_REQ(2));
	// UnlockAchievement
	mrb_define_module_function(state, SteamStats, "unlock_achievement", steam_achievements_mrb_unlock_achievement, MRB_ARGS_REQ(1));
	// ClearAchievement
	mrb_define_module_function(state, SteamStats, "reset_achievement", steam_achievements_mrb_clear_achievement, MRB_ARGS_REQ(1));
	// ResetAllStats
	mrb_define_module_function(state, SteamStats, "reset_all_stats", steam_achievements_mrb_reset_all_stats, MRB_ARGS_NONE());
	// ResetAllStatsAndAchievements
	mrb_define_module_function(state, SteamStats, "reset_all_stats_and_achievements", steam_achievements_mrb_reset_all_stats_and_achievements, MRB_ARGS_NONE());
	// GetAchievementStatus
	mrb_define_module_function(state, SteamStats, "achievement_status", steam_achievements_mrb_get_achievement_status, MRB_ARGS_REQ(1));
	// StoreStats
	mrb_define_module_function(state, SteamStats, "store_stats", steam_achievements_mrb_store_stats, MRB_ARGS_NONE());

        printf("* INFO: C extension 'Steam Stats & Achievements' registration completed.\n");

	mrb_atexit_func(steam_achievements_mrb_shutdown);
        atexit(steam_achievements_shutdown);
    }
}
