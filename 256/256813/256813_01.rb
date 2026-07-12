#!/usr/bin/env ruby
# Number of binary words of length n+5 avoiding the consecutive word ababab.
# OFFSET 1: a(1) counts words of length 6.

def terms(count)
  # Avoiding g.f. for ababab:
  # (1 + x^2 + x^4)/(x^6 + (1 - 2*x)*(1 + x^2 + x^4)).
  den = [1, -2, 1, -2, 1, -2, 1]
  num = [1, 0, 1, 0, 1]
  avoid = []

  (count + 6).times do |n|
    rhs = num[n] || 0
    sum = 0
    1.upto([n, den.length - 1].min) do |k|
      sum += den[k] * avoid[n - k]
    end
    avoid[n] = rhs - sum
  end

  avoid[6, count]
end

count = Integer(ARGV[0] || 35)
ary = terms(count)
(1..count).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}