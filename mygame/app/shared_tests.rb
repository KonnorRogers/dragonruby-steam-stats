# Silly hack to make sure we inherit the chain of instance methods.
# https://github.com/DragonRuby/dragonruby-game-toolkit-contrib/blob/d1afc67aa5a94da8bc25d2ed7143eedc34f1cac3/dragon/tests.rb#L49C1-L55C8
module GTK
  class Tests
    def test_methods
      test_classes.flat_map do |klass|
        klass.instance_methods(true)
             .find_all { |m| m.start_with?("test_") }
             .map { |m| { target: klass.new, m: m } }
      end
    end
  end
end

module SharedTestsModule
  attr_gtk

  def test_module_exists(args, assert)
    assert.true!(Steam, "Steam module should be defined")
    assert.true!(Steam::ClientStats, "Steam::ClientStats module should be defined")
  end

  def test_methods_defined(args, assert)
    methods = [
      :initialized?,
      :app_id,
      :set_stat,
      :unlock_achievement,
      :clear_achievement,
      :reset_all_stats,
      :reset_all_stats_and_achievements,
      :achievement_status,
      :store_stats
    ]

    methods.each do |method|
      assert.true!(
        Steam::ClientStats.respond_to?(method),
        "Steam::ClientStats should respond to #{method}"
      )
    end
  end

  def test_app_id(args, assert)
    result = Steam::ClientStats.app_id
    assert.true!(result == nil)
  end

  def test_initialized?(args, assert)
    result = Steam::ClientStats.initialized?
    assert.true!(result == false)
  end

  def test_clear_achievement_accepts_string(args, assert)
    result = Steam::ClientStats.clear_achievement("test_achievement_id")
    assert.true!(
      result == true || result == false,
      "clear_achievement should return boolean, got #{result.class}"
    )
  end

  def test_unlock_achievement_accepts_string(args, assert)
    result = Steam::ClientStats.unlock_achievement("test_achievement_id")
    assert.true!(
      result == true || result == false,
      "unlock_achievement should return boolean, got #{result.class}"
    )
  end

  def test_achievement_status_returns_valid_values(args, assert)
    status = Steam::ClientStats.achievement_status("test_achievement_id")
    assert.true!(
      [-1, 0, 1].include?(status),
      "achievement_status should return -1, 0, or 1, got #{status}"
    )
  end

  def test_set_stat_accepts_string_and_int(args, assert)
    result = Steam::ClientStats.set_stat("test_stat", 100)
    puts "RESULT: ", result
    assert.true!(
      [true, false].include?(result),
      "set_stat with int should return boolean, got #{result.class}"
    )
  end

  def test_set_stat_accepts_string_and_float(args, assert)
    result = Steam::ClientStats.set_stat("test_stat", 3.14)
    assert.true!(
      result == true || result == false,
      "set_stat with float should return boolean, got #{result.class}"
    )
  end

  def test_reset_all_stats_no_args(args, assert)
    result = Steam::ClientStats.reset_all_stats
    assert.true!(
      result == true || result == false,
      "reset_all_stats should return boolean"
    )
  end

  def test_reset_all_stats_and_achievements_no_args(args, assert)
    result = Steam::ClientStats.reset_all_stats_and_achievements
    assert.true!(
      result == true || result == false,
      "reset_all_stats_and_achievements should return boolean"
    )
  end

  def test_store_stats_no_args(args, assert)
    result = Steam::ClientStats.store_stats
    assert.true!(
      result == true || result == false,
      "store_stats should return boolean"
    )
  end

  def test_indicate_achievement_progress(args, assert)
    result = Steam::ClientStats.indicate_achievement_progress("test_ach", 5, 10)
    assert.true!(
      result == true || result == false,
      "indicate_achievement_progress should return boolean"
    )
  end
end
