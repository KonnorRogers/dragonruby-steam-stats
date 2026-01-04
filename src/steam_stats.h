#include "mruby.h"
#include <steam/steam_api_flat.h>

class CSteamStats
{
public:
	CSteamStats();
	~CSteamStats();

	int64 app_id; // Our current AppID
	bool initialized; // Have we called Request stats and received the callback?
	bool UnlockAchievement(const char *ID);
	bool ClearAchievement(const char *ID);
	bool UpdateAchievement(const char *ID);

	// Updates the server to store the stats.
	bool StoreStats();

	// updates a stat for a user
	bool SetStat( const char *pchName, int32 nData );
	bool SetStat( const char *pchName, float fData );

	/**
 	* @brief Gets the unlock status of a Steam achievement
 	*
 	* @param ID The 'API Name' of the achievement to check
 	*
 	* @return Achievement status:
 	*         - -1: Achievements aren't loaded (call RequestCurrentStats first)
 	*         -  0: Achievement is locked/not unlocked
 	*         -  1: Achievement is unlocked
 	*
 	* @note Requires RequestCurrentStats to have completed successfully
 	*/
	int GetAchievementStatus(const char *ID);

	// Gets the achievment status of a given steamId
	int GetUserAchievementStatus(CSteamID steamIDUser, const char *ID);

	bool IndicateAchievementProgress(const char *pchName, uint32 nCurProgress, uint32 nMaxProgress);

	// For some reason you have to reset stats and achievements together.
	bool ResetAllStats();
	bool ResetAllStatsAndAchievements();

	STEAM_CALLBACK( CSteamStats, OnUserStatsReceived, UserStatsReceived_t,
		callbackUserStatsReceived );
	STEAM_CALLBACK( CSteamStats, OnUserStatsStored, UserStatsStored_t,
		callbackUserStatsStored );
	STEAM_CALLBACK( CSteamStats, OnAchievementStored,
		UserAchievementStored_t, callbackAchievementStored );
};
