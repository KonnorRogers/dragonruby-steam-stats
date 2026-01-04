# Steam Stats + Achievement interface for DragonRuby

> NOTE!
> This extension is still untested as I do not have a game with Steam and am unable to actually test it in a Steam environment.

## What is this?

These are DragonRuby bindings for the [ISteamUserStats](https://partner.steamgames.com/doc/api/ISteamUserStats) interface from Steam. It is not complete, and has renamed some portion of the API, but is meant to be a minimal amount for single player games.

## Installation

Grab the precompiled binaries from [/releases](./releases) and place them in `mygame/vendor/steam_stats`

## Getting Started

```rb
def boot(args)
  GTK.dlopen("vendor/steam_stats")
  require "vendor/steam_stats/steam_stats.rb"
  # Will register `Steam::ClientStats`, even on unsupported platforms and running the game outside of Steam.
end

def tick(args)
  if Kernel.tick_count == 0
    # In memory unlocks the achievement.
    Steam::ClientStats.unlock_achievement("GETTING_STARTED_ACHIEVEMENT")

    # Calls the Steam backend to actually save the achievement to Steam's servers.
    Steam::ClientStats.store_stats
  end
end
```

## API

```rb
Steam::ClientStats.app_id # => gets the Steam App ID, generally not useful. `nil` if Steam failed to initialize.
# => [nil, Integer]

Steam::ClientStats.initialized? # => Checks if Steam API successfully initialized.
# => [true, false]

# Sets a stat for your game. Things like "miles ran", "mountains climbed", etc.
Steam::ClientStats.set_stat("STAT_NAME", 5)
# => [true, false]

Steam::ClientStats.unlock_achievement("ACHIEVEMENT_NAME")
# => [true, false]

Steam::ClientStats.clear_achievement(name) # Resets an achievement. Useful in development, not in production.
# => [true, false]

Steam::ClientStats.reset_all_stats # Resets all stats (not achievements)
# => [true, false]

Steam::ClientStats.reset_all_stats_and_achievements
# => [true, false]

# This returns -1, 0, 1
# -1 means unable to retrieve status, 0 means "not completed", 1 means "completed"
Steam::ClientStats.achievement_status(achievement_name)
# => [-1, 0, 1]

# Will show a popup with the user's current progress towards an achievement. If this returns false, it did not show / get called by Steam.
Steam::ClientStats.indicate_achievement_progress(achievement_name, current_progress, max_progress)
# => [true, false]

# !! IMPORTANT !!
# After doing anything like `indicate_achievement_progress`, `unlock_achievement`, `set_stat`, etc, make sure to call this afterwards. This is what calls the Steam backend and saves the achievement status. Without it, everything is just saved in memory. For more, read here:
# https://partner.steamgames.com/doc/api/ISteamUserStats#StoreStats
Steam::ClientStats.store_stats
# => [true, false]
```

## Unsupported Platforms

On unsupported platforms, all boolean APIs will return `false`, and `achievement_status` will return `-1`.

This is so that your game doesn't error on unsupported platforms when you call `Steam::ClientStats.<method>`, and so you don't need tons of conditionals everywhere. Especially as DragonRuby doesn't support `defined?(Steam::ClientStats)`, and using `Object.const_get` will raise an error you need to catch. This may be a bad decision on my part, and I'm not married to the idea, but I thought it was convenient.

## Developing locally

```
git clone https://github.com/KonnorRogers/dragonruby-steam-achievements
cd dragonruby-steam-achievements
```

Download the Steam SDK here:

<https://partner.steamgames.com/downloads/list>

Unzip your files to `./steam-sdk` within `dragonruby-steam-achievements`

## Build

`make`

## Test

```bash
make test-native
make test-non-native
```

