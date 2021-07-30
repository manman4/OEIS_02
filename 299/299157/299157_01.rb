require 'prime'

def A(n)
  # n = 1‚ÍŠÜ‚Ü‚ê‚È‚¢
  ary = []
  a = [0, 1]
  (2..10 ** 8).each{|i|
    break if ary.size == n
    if i.prime?
      s, t, u = 0, 1, 0
      (1..n).each{|j|
        t += 9 * j
        u += j
        break if i <= u
        s += (-1) ** (j % 2 + 1) * (2 * j + 1) * (i - t) * a[-u]
      }
      v = s / (i - 1)
      a << v
      if v % (i + 1) == 0
        ary << i
      end
    else
      s = 1
      i.prime_division.each{|j|
        k, l = j[0], j[1]
        if l == 1
          s *= a[k]
        else
          s *= a[k ** (l - 1)] * a[k] - k ** 11 * a[k ** (l - 2)]
        end
      }
      a << s
      if s % (i + 1) == 0
        ary << i
      end
    end
  }
  ary
end

n = 500
ary = A(n)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
