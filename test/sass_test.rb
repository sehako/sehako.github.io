# frozen_string_literal: true

require "minitest/autorun"
require_relative "support/sass_build"
require_relative "support/css_normalizer"

# Guards the Sass refactor: no deprecation warnings, every skin still compiles,
# and the rendered stylesheet keeps meaning what it meant.
#
# After an intentional style change, review the diff and refresh the baseline:
#   UPDATE_CSS_BASELINE=1 ruby test/sass_test.rb
class SassTest < Minitest::Test
  SKINS = %w[air aqua contrast dark default dirt mint neon plum sunrise].freeze
  BASELINE = File.join(SassBuild::ROOT, "test", "fixtures", "main.baseline.css")

  def test_site_skin_compiles_without_deprecations
    result = SassBuild.build
    assert result.status.success?, result.output
    assert_empty SassBuild.deprecations(result.output), result.output
    refute_match(/WARNING/, result.output)
  end

  def test_every_skin_compiles_without_deprecations
    SKINS.each do |skin|
      result = SassBuild.build(skin: skin)
      assert result.status.success?, "#{skin}: #{result.output}"
      assert_empty SassBuild.deprecations(result.output), "#{skin}: #{result.output}"
    end
  end

  def test_ci_overlay_with_fatal_deprecations_builds
    result = SassBuild.build(ci_config: true)
    assert result.status.success?, result.output
  end

  def test_compiled_css_matches_baseline
    normalized = CssNormalizer.normalize(SassBuild.build.css)
    if ENV["UPDATE_CSS_BASELINE"]
      File.write(BASELINE, normalized)
      skip "baseline updated"
    end
    assert_equal File.read(BASELINE), normalized,
                 "Compiled CSS changed. If intentional: UPDATE_CSS_BASELINE=1 ruby test/sass_test.rb"
  end

  def test_no_legacy_sass_constructs_remain
    sources = Dir[File.join(SassBuild::ROOT, "_sass", "**", "*.scss"), File.join(SassBuild::ROOT, "assets", "css", "*.scss")]
    sources.each do |path|
      code = File.read(path).gsub(%r{/\*.*?\*/}m, "").gsub(%r{//[^\n]*}, "")
      refute_match(/^\s*@import\b/, code, "#{path} still uses @import")
      refute_match(/@include\s+(?:breakpoint-set|susy)/, code, "#{path} uses a removed vendor mixin")
    end
  end
end
