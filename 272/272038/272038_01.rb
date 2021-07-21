def b(k, n)
  b = Array.new(2 * k + 3, 1)
  (0..2 * k + 2).each{|i|
    print i
    print ' '
    puts 1
  }
  (2 * k + 3..n).each{|i|
    j = (b[i - 1] * b[i - 2 * k - 2] + b[i - k - 1] * b[i - k - 2]) / b[i - 2 * k - 3].to_r
    j = j.to_i if j.denominator == 1
    b[i] = j
    break if j.to_s.size > 1000
    print i
    print ' '
    puts j
  }
end
b(9, 1000)
