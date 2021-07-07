require 'prime'

def A259825(n)
  return -1 if n == 0
  return 0 if n % 4 == 1 || n % 4 == 2
  s = 0
  (1..Math.sqrt(n / 3).to_i).each{|a|
    (0..a).each{|b|
      c4 = n + b * b
      if c4 % (4 * a) == 0
        c = c4 / (4 * a)
        if a == c
          if a == b
            s += 4
          elsif b == 0
            s += 6
          else
            s += 12
          end
        elsif a < c
          if a == b
            s += 12
          elsif b == 0
            s += 12
          else
            s += 24
          end
        end
      end
    }
  }
  s
end

def A(k, n)
  m = 4 * n
  (1..(2 * Math.sqrt(n)).to_i).inject(0 ** k * A259825(m) / 2){|s, i| s + i ** k * A259825(m - i * i)} / 12
end

def B(k, n)
  p_ary = [0] + Prime.take(n).to_a
  m = p_ary[-1]
  (1..n).map{|i| A(k, p_ary[i])}
end

n = 100
ary = B(8, n)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
