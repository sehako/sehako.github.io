# frozen_string_literal: true

require "fileutils"
require "date"
require "pathname"
require "uri"
require "yaml"

module BlogSync
  class Error < StandardError; end

  class Synchronizer
    MANIFEST_PATH = "_data/blog_sync_manifest.yml"
    SOURCE_POSTS = "area/블로그/글"
    SOURCE_ASSETS = "_assets/area/블로그/글"
    SOURCE_SHARED_ASSETS = "_assets/area/블로그/assets/images"
    LEGACY_INTERNAL_LINKS = {
      "https://sehako.github.io/data%20structure/graph/" => "/algorithm/graph/",
      "https://sehako.github.io/data%20structure/disjoint-set/" => "/algorithm/disjoint-set/",
      "https://sehako.github.io/spring/management-massege-with-massege-code/" =>
        "/spring/management-messege-with-messege-code/"
    }.freeze

    attr_reader :root, :source

    def initialize(root:, source:, output: $stdout)
      @root = Pathname(root).expand_path
      @source = Pathname(source).expand_path
      @output = output
    end

    def run(write: false)
      desired = desired_files
      managed = managed_files
      stale = managed - desired.keys
      changes = compare(desired, stale)

      if write
        apply(desired, stale)
        @output.puts(changes.empty? ? "Already synchronized." : "Synchronized #{changes.length} change(s).")
        true
      else
        changes.each { |change| @output.puts(change) }
        @output.puts(changes.empty? ? "Blog is synchronized." : "Blog is not synchronized (#{changes.length} change(s)).")
        changes.empty?
      end
    end

    def desired_files
      @desired_files ||= begin
        files = {}
        categories.each { |category| add_posts_and_assets(files, category) }
        add_shared_assets(files)
        add_category_pages(files)
        add_redirect_pages(files)
        manifest_files = files.keys.sort + [MANIFEST_PATH]
        files[MANIFEST_PATH] = YAML.dump("files" => manifest_files.sort)
        files
      end
    end

    private

    def categories_data
      @categories_data ||= load_yaml(root.join("_data/blog_categories.yml"))
    end

    def categories
      @categories ||= categories_data.fetch("categories").tap do |items|
        names = items.map { |item| item.fetch("name") }
        slugs = items.map { |item| item.fetch("slug") }
        raise Error, "Category names and slugs must be unique" unless names.uniq == names && slugs.uniq == slugs
      end
    end

    def legacy_urls
      @legacy_urls ||= load_yaml(root.join("_data/blog_post_urls.yml"))
    end

    def load_yaml(path)
      raise Error, "Missing required file: #{path}" unless path.file?

      YAML.safe_load(path.read, permitted_classes: [Date, Time], aliases: true) || {}
    end

    def add_posts_and_assets(files, category)
      name = category.fetch("name")
      slug = category.fetch("slug")
      directory = source.join(SOURCE_POSTS, name)
      raise Error, "Missing source category: #{directory}" unless directory.directory?

      directory.glob("*.md").sort.each do |post|
        stem = post.basename(".md").to_s
        raw = post.binread.force_encoding("UTF-8")
        metadata = front_matter(raw, post)
        actual_categories = Array(metadata["categories"])
        unless actual_categories == [name]
          raise Error, "#{post} must have exactly one category matching its directory (#{name})"
        end

        permalink = legacy_urls[stem] || "/#{slug}/#{post_slug(stem)}/"
        transformed = transform_asset_references(raw, name, slug, stem)
        transformed = transform_internal_links(transformed)
        transformed = normalize_trailing_whitespace(transformed)
        files["_posts/#{slug}/#{post.basename}"] = with_permalink(transformed, permalink)

        asset_directory = source.join(SOURCE_ASSETS, name, stem)
        next unless asset_directory.directory?

        asset_directory.glob("**/*").select(&:file?).sort.each do |asset|
          relative = asset.relative_path_from(asset_directory).to_s
          files["assets/images/#{slug}/#{stem}/#{relative}"] = asset.binread
        end
      end
    end

    def front_matter(raw, path)
      match = raw.match(/\A---[ \t]*\r?\n(.*?)\r?\n---[ \t]*\r?\n/m)
      raise Error, "Missing front matter: #{path}" unless match

      YAML.safe_load(match[1], permitted_classes: [Date, Time], aliases: true) || {}
    rescue Psych::SyntaxError => error
      raise Error, "Invalid front matter in #{path}: #{error.message}"
    end

    def with_permalink(raw, permalink)
      match = raw.match(/\A---[ \t]*\r?\n(.*?)\r?\n---[ \t]*\r?\n/m)
      yaml = match[1]
      line = "permalink: #{permalink.inspect}"
      yaml = if yaml.match?(/^permalink:/)
               yaml.sub(/^permalink:.*$/, line)
             else
               "#{yaml.rstrip}\n\n#{line}"
             end
      raw.sub(match[0], "---\n#{yaml}\n---\n")
    end

    def transform_asset_references(raw, category_name, category_slug, stem)
      expected_prefix = "_assets/area/블로그/글/#{category_name}/#{stem}/"
      raw.gsub(%r{_assets/area/(?:%[0-9A-Fa-f]{2}|[^\s)'\"<>])+}) do |reference|
        decoded = URI::DEFAULT_PARSER.unescape(reference)
        unless decoded.start_with?(expected_prefix)
          raise Error, "Unexpected asset reference in #{stem}: #{reference}"
        end

        relative = decoded.delete_prefix(expected_prefix)
        asset = source.join(expected_prefix, relative)
        raise Error, "Missing referenced asset: #{asset}" unless asset.file?

        "/assets/images/#{category_slug}/#{stem}/#{relative}"
      end
    end

    def normalize_trailing_whitespace(raw)
      raw.gsub(/[ \t]+(?=\r?$)/, "")
    end

    def transform_internal_links(raw)
      LEGACY_INTERNAL_LINKS.reduce(raw) do |content, (old_url, current_url)|
        content.gsub(old_url, current_url)
      end
    end

    def add_shared_assets(files)
      directory = source.join(SOURCE_SHARED_ASSETS)
      return unless directory.directory?

      directory.glob("**/*").select(&:file?).sort.each do |asset|
        relative = asset.relative_path_from(directory).to_s
        files["assets/images/#{relative}"] = asset.binread
      end
    end

    def add_category_pages(files)
      files["_pages/category.md"] = <<~MARKDOWN
        ---
        title: "카테고리"
        layout: categories
        permalink: /categories/
        author_profile: true
        ---
      MARKDOWN

      categories.each do |category|
        name = category.fetch("name")
        slug = category.fetch("slug")
        files["_pages/categories/category-#{slug}.md"] = <<~MARKDOWN
          ---
          title: #{name.inspect}
          layout: category
          permalink: /categories/#{slug}/
          taxonomy: #{name.inspect}
          author_profile: true
          sidebar_main: true
          ---
        MARKDOWN
      end
    end

    def add_redirect_pages(files)
      categories_data.fetch("redirects", {}).sort.each do |from, to|
        target = to == "categories" ? "/categories/" : "/categories/#{to}/"
        files["_pages/redirects/category-#{from}.html"] = <<~HTML
          ---
          layout: null
          permalink: /categories/#{from}/
          sitemap: false
          ---
          <!doctype html>
          <html lang="ko">
            <head>
              <meta charset="utf-8">
              <meta http-equiv="refresh" content="0; url={{ '#{target}' | relative_url }}">
              <link rel="canonical" href="{{ '#{target}' | absolute_url }}">
              <title>카테고리 이동</title>
            </head>
            <body>
              <p><a href="{{ '#{target}' | relative_url }}">새 카테고리로 이동합니다.</a></p>
            </body>
          </html>
        HTML
      end
    end

    def post_slug(stem)
      stem.sub(/\A\d{2,4}-\d{2}-\d{2}-/, "")
    end

    def managed_files
      manifest = root.join(MANIFEST_PATH)
      return load_yaml(manifest).fetch("files", []) if manifest.file?

      bootstrap_managed_files
    end

    def bootstrap_managed_files
      patterns = ["_posts/**/*.md", "assets/images/**/*", "_pages/categories/*", "_pages/category.md"]
      patterns.flat_map { |pattern| root.glob(pattern).select(&:file?) }
              .map { |path| path.relative_path_from(root).to_s }
              .uniq
    end

    def compare(desired, stale)
      changes = []
      desired.sort.each do |relative, content|
        path = root.join(relative)
        changes << "ADD #{relative}" unless path.file?
        changes << "UPDATE #{relative}" if path.file? && path.binread != content.b
      end
      stale.sort.each { |relative| changes << "DELETE #{relative}" if root.join(relative).file? }
      changes
    end

    def apply(desired, stale)
      desired.each do |relative, content|
        path = root.join(relative)
        next if path.file? && path.binread == content.b

        path.dirname.mkpath
        path.binwrite(content.b)
      end

      stale.each do |relative|
        path = root.join(relative)
        path.delete if path.file?
      end
      remove_empty_managed_directories
    end

    def remove_empty_managed_directories
      %w[_posts assets/images _pages/categories _pages/redirects].each do |relative|
        base = root.join(relative)
        next unless base.directory?

        base.glob("**/*").select(&:directory?).sort_by { |path| -path.to_s.length }.each do |directory|
          directory.rmdir if directory.children.empty?
        end
      end
    end
  end
end
