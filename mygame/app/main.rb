def boot(args)
  GTK.dlopen("extension")
end

class SteamAchievement
  def initialize(id:, name:, description:, image:, achieved: false)

  end
end

def tick args
  args.state.achievements ||= new SteamAchievements([
    # SteamAchievements::Achievement.new(
    #   api_name: "ACH_FINISHED_GAME",
    #   display_name: "Winner!",
    #   description: "Won the game!",
    #   image: 0
    # ),
    {
      api_name: "ACH_FINISHED_EVERYTHING", display_name: "Completionist", description: "100% of the game complete", image: 0
    }
  ])


  # achievement_id =
  steam_achievements = args.state.steam_achievements.unlock(id)
end

def shutdown(args)
  FileDialog.cleanup
end
