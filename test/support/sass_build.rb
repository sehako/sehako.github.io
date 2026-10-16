# frozen_string_literal: true

require "open3"
require "tmpdir"
require "yaml"

# Builds the site with Jekyll and returns the compiled stylesheet plus the
# stderr/stdout text so tests can assert on Sass diagnostics.
module SassBuild
  ROOT = File.expand_path("../..", __dir__)

  Result = Struct.new(:css, :output, :status, keyword_init: true)

  module_function

  def build(skin: nil, extra_sass: {}, ci_config: false)
    Dir.mktmpdir("sass-build") do |dir|
      overlay = File.join(dir, "overlay.yml")
      config = { "sass" => { "style" => "expanded", "sourcemap" => "never", "verbose" => true }.merge(extra_sass) }
      config["minimal_mistakes_skin"] = skin if skin
      File.write(overlay, YAML.dump(config))
      destination = File.join(dir, "site")
      output, status = Open3.capture2e(
        "bundle", "exec", "jekyll", "build",
        "--config", configs(ci_config, overlay), "--destination", destination,
        chdir: ROOT
      )
      css_path = File.join(destination, "assets", "css", "main.css")
      Result.new(css: File.exist?(css_path) ? File.read(css_path) : nil, output: output, status: status)
    end
  end

  def configs(ci_config, overlay)
    ["_config.yml", ("_config.ci.yml" if ci_config), overlay].compact.join(",")
  end

  def deprecations(output)
    output.scan(/DEPRECATION WARNING \[([a-z-]+)\]/).flatten.tally
  end
end
