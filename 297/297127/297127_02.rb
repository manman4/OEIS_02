require 'prime'

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

def B(n)
  m = 4 * n
  (1..(2 * Math.sqrt(n)).to_i).inject(m ** 5 * A(m) / 2){|s, i| s + (m - i * i) ** 5 * A(m - i * i)}.to_i
end

def C(n)
  p_ary = [0] + Prime.take(n).to_a
  (1..n).map{|i| B(p_ary[i])}
end

n = 19
ary = C(n)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
