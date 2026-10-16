# frozen_string_literal: true

# Reduces compiled CSS to a comment-free, one-rule-per-line form so that
# refactors of the Sass sources can be compared by rendered meaning.
#
# Each output line is "<at-rule context> | <selector> { <declarations> }".
# Rule order is preserved because it decides the cascade.
module CssNormalizer
  module_function

  def normalize(css)
    css = css.gsub(%r{/\*.*?\*/}m, "").sub(/\A\s*@charset\s+"[^"]*";/i, "")
    css = css.gsub(/#([0-9a-fA-F]{6})\b/) { shorten_hex(Regexp.last_match(1).downcase) }
    css = css.gsub(/#([0-9a-fA-F]{3})\b/) { "##{Regexp.last_match(1).downcase}" }
    css = css.gsub(/\d+\.\d{5,}/) { |n| format("%.4f", n.to_f).sub(/\.?0+\z/, "") }
    lines = []
    walk(css, [], lines)
    lines.join("\n") + "\n"
  end

  def shorten_hex(hex)
    hex.match?(/\A(.)\1(.)\2(.)\3\z/) ? "##{hex[0]}#{hex[2]}#{hex[4]}" : "##{hex}"
  end

  def walk(css, context, lines)
    scanner = css.strip
    until scanner.empty?
      head, body, rest = take_block(scanner)
      if body.nil?
        lines << "#{context.join(' > ')} | #{squish(head)}" unless squish(head).empty?
        break
      end
      head = squish(head)
      if head.start_with?("@media", "@supports", "@layer") && body.include?("{")
        walk(body, context + [head], lines)
      else
        lines << "#{context.join(' > ')} | #{head} { #{squish(body)} }"
      end
      scanner = rest.strip
    end
  end

  # Splits "head { body } rest" honouring nested braces and strings.
  def take_block(text)
    open = text.index("{")
    return [text, nil, ""] unless open

    depth = 0
    quote = nil
    (open...text.length).each do |i|
      char = text[i]
      if quote
        quote = nil if char == quote && text[i - 1] != "\\"
      elsif char == '"' || char == "'"
        quote = char
      elsif char == "{"
        depth += 1
      elsif char == "}"
        depth -= 1
        return [text[0...open], text[(open + 1)...i], text[(i + 1)..]] if depth.zero?
      end
    end
    raise "unbalanced braces in CSS"
  end

  def squish(text)
    text.gsub(/\s+/, " ").gsub(/\s*([{};,>+~])\s*/, '\1').gsub(/;}/, "}").strip
  end
end
