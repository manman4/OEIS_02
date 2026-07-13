#!/usr/bin/env ruby
# Counts q-ary words avoiding a fixed consecutive word, via autocorrelation.
#
# Usage:
#   ruby xxx.rb q pattern [terms]
#
# Examples:
#   ruby xxx.rb 2 ababab 35
#   ruby xxx.rb 3 abcabc 35
#   ruby xxx.rb --gf 3 abcabc

def usage!
  program = File.basename($PROGRAM_NAME)
  puts <<~USAGE
    Usage:
      ruby #{program} q pattern [terms]

    This prints an OEIS-ready draft for
    a(n) = number of q-ary words of length n avoiding the given consecutive word.

    Arguments:
      q        Alphabet size. For binary words use 2, for ternary words use 3.
      pattern  Consecutive word to be avoided, e.g., ababab or abcabc.
      terms    Number of initial terms to print. Default: 35.

    Options:
      --gf, --gf-only
               Print only the generating function for words avoiding
               the given pattern.
  USAGE
  exit
end

def trim(poly)
  poly = poly.dup
  poly.pop while poly.length > 1 && poly[-1] == 0
  poly
end

def add_to(poly, index, value)
  poly[index] ||= 0
  poly[index] += value
end

def poly_to_s(poly, var = "x")
  poly = trim(poly)
  terms = []
  poly.each_with_index do |coef, exp|
    next if coef == 0

    abs = coef.abs
    body =
      if exp == 0
        abs.to_s
      elsif exp == 1
        abs == 1 ? var : "#{abs}*#{var}"
      else
        abs == 1 ? "#{var}^#{exp}" : "#{abs}*#{var}^#{exp}"
      end

    terms << [coef < 0 ? "-" : "+", body]
  end

  return "0" if terms.empty?

  sign, body = terms.shift
  out = sign == "-" ? "-#{body}" : body
  terms.each { |sgn, term| out << " #{sgn} #{term}" }
  out
end

def var_power(var, exp)
  exp == 1 ? var : "#{var}^#{exp}"
end

def autocorrelation(pattern)
  chars = pattern.chars
  m = chars.length
  Array.new(m, 0).tap do |corr|
    m.times do |shift|
      len = m - shift
      corr[shift] = chars[shift, len] == chars[0, len] ? 1 : 0
    end
  end
end

def avoid_denominator(q, corr)
  m = corr.length
  den = Array.new(m + 1, 0)
  corr.each_with_index do |c, i|
    next if c == 0
    add_to(den, i, c)
    add_to(den, i + 1, -q * c)
  end
  add_to(den, m, 1)
  trim(den)
end

def avoiding_terms(num, den, count)
  terms = Array.new(count, 0)
  count.times do |n|
    rhs = num[n] || 0
    sum = 0
    1.upto([n, den.length - 1].min) do |k|
      sum += den[k] * terms[n - k]
    end
    terms[n] = rhs - sum
  end
  terms
end

def recurrence_string(den)
  pieces = []
  1.upto(den.length - 1) do |k|
    coef = -den[k]
    next if coef == 0
    abs = coef.abs
    term = abs == 1 ? "a(n-#{k})" : "#{abs}*a(n-#{k})"
    pieces << [coef < 0 ? "-" : "+", term]
  end

  sign, term = pieces.shift
  out = sign == "-" ? "-#{term}" : term
  pieces.each { |sgn, body| out << " #{sgn} #{body}" }
  out
end

def word_class(q)
  case q
  when 2 then "binary"
  when 3 then "ternary"
  else "#{q}-ary"
  end
end

def initial_values_string(terms, last_index)
  0.upto(last_index).map { |i| "a(#{i})=#{terms[i]}" }.join(", ")
end

usage! if ARGV.include?("-h") || ARGV.include?("--help")
gf_only = false
["--gf", "--gf-only"].each do |option|
  gf_only = true if ARGV.delete(option)
end

q = Integer(ARGV[0] || usage!)
pattern = ARGV[1] || usage!
term_count = Integer(ARGV[2] || 35)
usage! if q <= 0 || pattern.empty? || term_count <= 0

distinct = pattern.chars.uniq.length
if distinct > q
  abort "The pattern uses #{distinct} distinct letters, but q=#{q}."
end

m = pattern.length
corr = autocorrelation(pattern)
c_poly = trim(corr)
den = avoid_denominator(q, corr)
terms = avoiding_terms(c_poly, den, term_count)
c_expr = poly_to_s(c_poly, "x")
den_expr = "#{var_power('x', m)} + (1 - #{q}*x)*(#{c_expr})"
gf = "(#{c_expr})/(#{den_expr})"
rec_start = den.length - 1
pari = "a(n) = polcoef(#{gf} + O(x^(n+1)), n);"

if gf_only
  puts gf
  exit
end

puts "Name:"
puts "Number of #{word_class(q)} words of length n avoiding the consecutive word #{pattern}."
puts
puts "Formula:"
puts "G.f.: #{gf}."
puts
puts "Comment:"
puts "The autocorrelation polynomial of #{pattern} is c(x) = #{c_expr}. Therefore the OGF for #{word_class(q)} words avoiding #{pattern} is c(x)/(x^#{m} + (1 - #{q}*x)*c(x))."
puts
puts "Recurrence:"
puts "a(n) = #{recurrence_string(den)} for n >= #{rec_start}, with #{initial_values_string(terms, rec_start - 1)}."
puts
puts "PARI/GP:"
puts pari
puts
puts "Terms:"
puts terms.join(",")
