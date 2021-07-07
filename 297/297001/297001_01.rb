require 'prime'

def f(m, n)
  s = 0
  while n % m == 0
    n = n / m
    s += 1
  end
  s
end

def A295645(n)
  ary = [-1]
  a = [0, 1]
  (2..n).each{|i|
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
      ary << f(i, v)
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
      ary << f(i, s)
    end
  }
  ary
end

ary0 = A295645(23 * 10 ** 6)
ary =(1..ary0.size).select{|i| ary0[i - 1] == 2}
(1..500).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
