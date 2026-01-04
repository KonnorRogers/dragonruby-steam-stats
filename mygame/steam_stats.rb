begin
  # Do nothing. Defined by native extensions.
  Object.const_get("Steam::ClientStats")
rescue
  module Steam
    module ClientStats
      def self.app_id
        nil
      end

      def self.initialized?
        false
      end

      def self.set_stat(name, int)
        false
      end

      def self.unlock_achievement(name)
        false
      end

      def self.clear_achievement(name)
        false
      end

      def self.reset_all_stats
        false
      end

      def self.reset_all_stats_and_achievements
        false
      end

      def self.achievement_status(name)
        # This returns -1, 0, 1
        # -1 means unable to retrieve status, 0 means "not completed", 1 means "completed"
        -1
      end

      def self.store_stats
        false
      end

      def self.indicate_achievement_progress(name, current_progress, max_progress)
        false
      end
    end
  end
end

