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

def A(n)
  m = 4 * n
  (1..(2 * Math.sqrt(n)).to_i).inject(m ** 5 * A259825(m) / 2){|s, i| s + (m - i * i) ** 5 * A259825(m - i * i)} / 12
end

# n�͑f��
def B(n)
  A(n) - (462 * n ** 6 + 330 * n ** 4 - 165 * n ** 3 + 55 * n ** 2 - 11 * n + 1)
end

[2, 29, 541, 7919, 104729].each{|i| p B(i)}

# (10 ** 8..10 ** 8 + 10 ** 6).each{|i| p [i, A259825(i)]}
