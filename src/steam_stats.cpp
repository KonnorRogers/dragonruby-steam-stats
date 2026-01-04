#include "steam/isteamuserstats.h"
#include <cstdlib>
#include <cstring>

extern "C" {
#include <string.h>
#include <mruby.h>
#include <mruby/array.h>
#include <dragonruby.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
}

#include "./steam_stats.h"

extern "C" {
    typedef enum LogLevel
	{
	LOG_LEVEL_SPAM=0,  // periodic (SPAMMY!) logging, like framerate updates, etc.
	LOG_LEVEL_DEBUG, // Debug stuff that might be uninteresting/spammy
	LOG_LEVEL_INFO,  // basic information. Good default log level.
	LOG_LEVEL_WARN,  // warnings that should be noticed.
	LOG_LEVEL_ERROR,  // errors that should always be noticed.
	LOG_LEVEL_UNFILTERED=0x7FFFFFFE,   // items that should be logged (almost) unconditionally.
	LOG_LEVEL_NOTHING=0x7FFFFFFF   // don't ever log this thing.
    } LogLevel;

    static drb_api_t *drb_api;
    mrb_state *global_state;
}

// https://partner.steamgames.com/doc/api/ISteamGameServerStats <-- setting achieves from server
// https://partner.steamgames.com/doc/api/ISteamUserStats <-- setting achievements client side

CSteamStats::CSteamStats():
 callbackUserStatsReceived( this, &CSteamStats::OnUserStatsReceived ),
 callbackUserStatsStored( this, &CSteamStats::OnUserStatsStored ),
 callbackAchievementStored( this, &CSteamStats::OnAchievementStored )
{
     this->app_id = -1;
     this->initialized = SteamAPI_Init();
     if (this->initialized) {
     	this->app_id = SteamUtils()->GetAppID();

     	if (!this->app_id) {
     		this->app_id = -1;
     	}
     }

}

bool CSteamStats::UnlockAchievement(const char* ID)
{
	// Have we received a call back from Steam yet?
	if (this->initialized)
	{
		return SteamUserStats()->SetAchievement(ID);
	}
	// If not then we can't set achievements yet
	return false;
}

int CSteamStats::GetAchievementStatus(const char* ID) {
	bool isUnlocked;

	if (this->initialized && SteamUserStats()->GetAchievement(ID, &isUnlocked)) {
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
int CSteamStats::GetUserAchievementStatus(CSteamID steamIDUser, const char* ID) {
	bool isUnlocked;
	if (this->initialized && SteamUserStats()->GetUserAchievement(steamIDUser, ID, &isUnlocked)) {
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
bool CSteamStats::IndicateAchievementProgress(const char *name, uint32 currentProgress, uint32 maxProgress) {
	if (!this->initialized) { return false; }

	return SteamUserStats()->IndicateAchievementProgress(name, currentProgress, maxProgress);
}


bool CSteamStats::ResetAllStats() {
	if (!this->initialized) {
		return false;
	}

	return SteamUserStats()->ResetAllStats(false);
}


bool CSteamStats::ResetAllStatsAndAchievements() {
	if (!this->initialized) {
		return false;
	}

	return SteamUserStats()->ResetAllStats(true);
}

bool CSteamStats::SetStat(const char *name, int32 data) {
	if (!this->initialized) { return false; }

	return SteamUserStats()->SetStat(name, data);
}

bool CSteamStats::SetStat(const char *name, float data) {
	if (!this->initialized) { return false; }

	return SteamUserStats()->SetStat(name, data);
}

bool CSteamStats::ClearAchievement(const char* ID)
{
	// Have we received a call back from Steam yet?
	if (!this->initialized) {
		// If not then we can't set achievements yet
		return false;
	}
	return SteamUserStats()->ClearAchievement(ID);
}

bool CSteamStats::StoreStats() {
	if (!this->initialized) { return false; }
	return SteamUserStats()->StoreStats();
}

void CSteamStats::OnUserStatsReceived( UserStatsReceived_t *pCallback )
{
	// we may get callbacks for other games' stats arriving, ignore them
	if (app_id == pCallback->m_nGameID )
	{
		if ( k_EResultOK == pCallback->m_eResult )
		{
			drb_api->drb_log_write("steam_stats", LOG_LEVEL_DEBUG,"Received stats and achievements from Steam\n");
		}
		else
		{
			char buffer[128];
			snprintf( buffer, 128, "RequestStats - failed, %d\n", pCallback->m_eResult );
			drb_api->drb_log_write("steam_stats", LOG_LEVEL_DEBUG, buffer );
		}
	}
}

void CSteamStats::OnUserStatsStored( UserStatsStored_t *pCallback )
{
	// we may get callbacks for other games' stats arriving, ignore them
	if ( app_id == pCallback->m_nGameID )
	{
		if ( k_EResultOK == pCallback->m_eResult )
		{
    			drb_api->drb_log_write("steam_stats", LOG_LEVEL_DEBUG, "Stored stats for Steam\n");
		}
		else
		{
			char buffer[128];
			snprintf( buffer, 128, "StatsStored - failed, %d\n", pCallback->m_eResult );
    			drb_api->drb_log_write("steam_stats", LOG_LEVEL_DEBUG, buffer);
		}
	}
}

void CSteamStats::OnAchievementStored( UserAchievementStored_t *pCallback )
{
     // we may get callbacks for other games' stats arriving, ignore them
     if ( app_id == pCallback->m_nGameID )
     {
          drb_api->drb_log_write("steam_stats", LOG_LEVEL_DEBUG, "Stored Achievement for Steam\n" );
     }
}


extern "C" {
    CSteamStats *steamStats = new CSteamStats();

    static void steam_stats_shutdown () {
    	    SteamAPI_Shutdown();
    }

    static void steam_stats_mrb_shutdown (mrb_state *state) {
    	    SteamAPI_Shutdown();
    }

    static mrb_value steam_stats_mrb_set_stat(mrb_state *state, mrb_value self) {
    	const char *name;
	int32 integer;

	// https://github.com/mruby/mruby/blob/510ebd738dc1326fd8c9f3a702f0d826d7fb330a/src/class.c#L1704-L1723
	// expect 1 string, 1 int
    	drb_api->mrb_get_args(state, "zi", &name, &integer);

        return mrb_bool_value(steamStats->SetStat(name, integer));
    }

    static mrb_value steam_stats_mrb_unlock_achievement(mrb_state *state, mrb_value self) {
		char *c_str;

		// CString
		drb_api->mrb_get_args(state, "z", &c_str);

		return mrb_bool_value(steamStats->UnlockAchievement(c_str));
    }

    static mrb_value steam_stats_mrb_indicate_achievement_progress(mrb_state *state, mrb_value self) {
		char *c_str;
		uint32 c_currentProgress;
		uint32 c_maxProgress;

		// CString, int, int
		drb_api->mrb_get_args(state, "zii", &c_str, &c_currentProgress, &c_maxProgress);

		return mrb_bool_value(steamStats->IndicateAchievementProgress(c_str, c_currentProgress, c_maxProgress));
    }

    static void log_method_call(const char *method_name, const char *args[], int arg_length) {
    	const char *prefix = ": ";
    	const char *open_parentheses = "(";
    	const char *close_parentheses = ")";

    	// Calculate total buffer size needed
    	int buffer_length = strlen(prefix) + strlen(method_name) +
                        	strlen(open_parentheses) + strlen(close_parentheses);

    	for (int i = 0; i < arg_length; i++) {
        	buffer_length += strlen(args[i]);
        	if (i < arg_length - 1) buffer_length++; // for commas
    	}
    	buffer_length++; // for null terminator

    	char buffer[buffer_length];
    	buffer[0] = '\0';

    	// Build the string
    	strcat(buffer, prefix);
    	strcat(buffer, method_name);
    	strcat(buffer, open_parentheses);

    	for (int i = 0; i < arg_length; i++) {
        	strcat(buffer, args[i]);
        	if (i < arg_length - 1) strcat(buffer, ", ");
    	}

    	strcat(buffer, close_parentheses);
    	drb_api->drb_log_write("steam_stats", LOG_LEVEL_DEBUG, buffer);
    }

    // Will "reset" an achievement
    static mrb_value steam_stats_mrb_clear_achievement(mrb_state *state, mrb_value self) {
		char *c_str;

		// CString
		drb_api->mrb_get_args(state, "z", &c_str);

		const char *args[1] = {c_str};
		log_method_call("clear_achievement", args, 1);
		return mrb_bool_value(steamStats->ClearAchievement(c_str));
    }

    static mrb_value steam_stats_mrb_reset_all_stats(mrb_state *state, mrb_value self) {
		return mrb_bool_value(steamStats->ResetAllStats());
    }

    static mrb_value steam_stats_mrb_reset_all_stats_and_achievements(mrb_state *state, mrb_value self) {
		return mrb_bool_value(steamStats->ResetAllStatsAndAchievements());
    }

    static mrb_value steam_stats_mrb_get_achievement_status(mrb_state *state, mrb_value self) {
		char *c_str;

		// CString
		drb_api->mrb_get_args(state, "z", &c_str);

		// Returns -1, 0, or 1. Should we convert to symbols? throw an error? Return :failed | true | false?
		// no idea.
		auto status = steamStats->GetAchievementStatus(c_str);

		return drb_api->mrb_int_value(state, status);
    }

    static mrb_value steam_stats_mrb_store_stats(mrb_state *state, mrb_value self) {
		return mrb_bool_value(steamStats->StoreStats());
    }

    static mrb_value steam_stats_mrb_initialized(mrb_state *state, mrb_value self) {
		return mrb_bool_value(steamStats->initialized);
    }

    static mrb_value steam_stats_mrb_app_id(mrb_state *state, mrb_value self) {
		int64 app_id = steamStats->app_id;
		if (app_id == -1) {
			return drb_api->mrb_nil_value();
		}

		return drb_api->mrb_int_value(state, app_id);
    }

    DRB_FFI_EXPORT
    void drb_register_c_extensions_with_api(mrb_state *state, struct drb_api_t *api) {
        drb_api = api;
        struct RClass *SteamModule = drb_api->mrb_define_module(state, "Steam");
        struct RClass *SteamStats = drb_api->mrb_define_module_under(state, SteamModule, "ClientStats");

	// Singleton for steam achievements, may live to regret this, but fine for now.
	drb_api->mrb_define_module_function(state, SteamStats, "initialized?", steam_stats_mrb_initialized, MRB_ARGS_NONE());
	drb_api->mrb_define_module_function(state, SteamStats, "app_id", steam_stats_mrb_app_id, MRB_ARGS_NONE());

	// SetStat
	drb_api->mrb_define_module_function(state, SteamStats, "set_stat", steam_stats_mrb_set_stat, MRB_ARGS_REQ(2));
	// UnlockAchievement
	drb_api->mrb_define_module_function(state, SteamStats, "unlock_achievement", steam_stats_mrb_unlock_achievement, MRB_ARGS_REQ(1));
	// ClearAchievement
	drb_api->mrb_define_module_function(state, SteamStats, "clear_achievement", steam_stats_mrb_clear_achievement, MRB_ARGS_REQ(1));
	// ResetAllStats
	drb_api->mrb_define_module_function(state, SteamStats, "reset_all_stats", steam_stats_mrb_reset_all_stats, MRB_ARGS_NONE());
	// ResetAllStatsAndAchievements
	drb_api->mrb_define_module_function(state, SteamStats, "reset_all_stats_and_achievements", steam_stats_mrb_reset_all_stats_and_achievements, MRB_ARGS_NONE());
	// GetAchievementStatus
	drb_api->mrb_define_module_function(state, SteamStats, "achievement_status", steam_stats_mrb_get_achievement_status, MRB_ARGS_REQ(1));
	// StoreStats
	drb_api->mrb_define_module_function(state, SteamStats, "store_stats", steam_stats_mrb_store_stats, MRB_ARGS_NONE());

	// IndicateAchievementProgress
	drb_api->mrb_define_module_function(state, SteamStats, "indicate_achievement_progress", steam_stats_mrb_indicate_achievement_progress, MRB_ARGS_REQ(3));

        printf("* INFO: C extension 'steam_stats' registration completed.\n");

	drb_api->mrb_state_atexit(state, steam_stats_mrb_shutdown);
	mrb_atexit_func(steam_stats_mrb_shutdown);
        atexit(steam_stats_shutdown);
    }
}
