def A(n)
  return -1r / 12 if n == 0
  s = 0
  (1..Math.sqrt(n / 3).to_i).each{|a|
    (0..a).each{|b|
      c4 = n + b * b
      if c4 % (4 * a) == 0
        c = c4 / (4 * a)
        if a == c
          if a == b
            s += 1r / 3
          elsif b == 0
            s += 1r / 2
          else
            s += 1
          end
        elsif a < c
          if a == b
            s += 1
          elsif b == 0
            s += 1
          else
            s += 2
          end
        end
      end
    }
  }
  s
end

n = 10000
(0..n).each{|i|
  print i
  print ' '
  puts (12 * A(i)).to_i
}
