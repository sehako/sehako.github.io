# frozen_string_literal: true

require "minitest/autorun"
require "stringio"
require "tmpdir"
require_relative "../lib/blog_sync"

class BlogSyncTest < Minitest::Test
  def setup
    @temporary = Dir.mktmpdir("blog-sync-test")
    @root = File.join(@temporary, "blog")
    @source = File.join(@temporary, "obsidian")
    write("#{@root}/_data/blog_categories.yml", <<~YAML)
      categories:
        - name: 인증과 보안
          slug: authentication-security
      redirects:
        oauth2: authentication-security
    YAML
    write("#{@root}/_data/blog_post_urls.yml", <<~YAML)
      24-01-02-existing: "/spring/existing/"
    YAML
  end

  def teardown
    FileUtils.remove_entry(@temporary)
  end

  def test_category_mapping_front_matter_and_legacy_permalink
    source_post("24-01-02-existing", published: false, date: "2024-01-01")

    sync.run(write: true)
    post = read("#{@root}/_posts/authentication-security/24-01-02-existing.md")

    assert_includes post, "categories:\n  - 인증과 보안"
    assert_includes post, "published: false"
    assert_includes post, "date: 2024-01-01"
    assert_includes post, 'permalink: "/spring/existing/"'
  end

  def test_new_post_uses_category_slug_permalink
    old_link = "https://sehako.github.io/data%20structure/graph/"
    typo_link = "https://sehako.github.io/spring/management-massege-with-massege-code/"
    source_post("25-03-04-new-post", body: "[Graph](#{old_link}) [Message](#{typo_link}) with trailing space  \n")

    sync.run(write: true)

    post = read("#{@root}/_posts/authentication-security/25-03-04-new-post.md")
    assert_includes post, 'permalink: "/authentication-security/new-post/"'
    assert_includes post, "[Graph](/algorithm/graph/)"
    assert_includes post, "[Message](/spring/management-messege-with-messege-code/)"
    refute_match(/[ \t]+$/, post)
  end

  def test_markdown_and_html_images_are_copied_and_rewritten
    encoded = "_assets/area/%EB%B8%94%EB%A1%9C%EA%B7%B8/%EA%B8%80/%EC%9D%B8%EC%A6%9D%EA%B3%BC%20%EB%B3%B4%EC%95%88/25-03-04-new-post/01.png"
    source_post("25-03-04-new-post", body: "![diagram](#{encoded})\n<img src=\"#{encoded}\">\n")
    write("#{@source}/_assets/area/블로그/글/인증과 보안/25-03-04-new-post/01.png", "image")

    sync.run(write: true)
    post = read("#{@root}/_posts/authentication-security/25-03-04-new-post.md")
    path = "/assets/images/authentication-security/25-03-04-new-post/01.png"

    assert_equal 2, post.scan(path).length
    assert_equal "image", read("#{@root}#{path}")
  end

  def test_write_deletes_only_previously_managed_files
    source_post("25-03-04-new-post")
    sync.run(write: true)
    write("#{@root}/assets/images/unmanaged.txt", "keep")
    FileUtils.rm("#{@source}/area/블로그/글/인증과 보안/25-03-04-new-post.md")

    sync_again.run(write: true)

    refute File.exist?("#{@root}/_posts/authentication-security/25-03-04-new-post.md")
    assert_equal "keep", read("#{@root}/assets/images/unmanaged.txt")
  end

  def test_check_is_read_only_and_reports_differences
    source_post("25-03-04-new-post")

    refute sync.run(write: false)
    refute File.exist?("#{@root}/_posts/authentication-security/25-03-04-new-post.md")
    sync.run(write: true)
    assert sync_again.run(write: false)
  end

  private

  def source_post(stem, published: true, date: "2025-03-04", body: "Body\n")
    write("#{@source}/area/블로그/글/인증과 보안/#{stem}.md", <<~MARKDOWN)
      ---
      title: Test
      categories:
        - 인증과 보안
      published: #{published}
      date: #{date}
      ---

      #{body}
    MARKDOWN
  end

  def sync
    @sync ||= BlogSync::Synchronizer.new(root: @root, source: @source, output: StringIO.new)
  end

  def sync_again
    BlogSync::Synchronizer.new(root: @root, source: @source, output: StringIO.new)
  end

  def write(path, content)
    FileUtils.mkdir_p(File.dirname(path))
    File.binwrite(path, content)
  end

  def read(path)
    File.binread(path).force_encoding("UTF-8")
  end
end
