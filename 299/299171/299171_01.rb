require 'prime'

def A(n)
  ary = [0, 1]
  a = []
  (2..10 ** 7).each{|i|
    break if a.size == n
    if i.prime?
      s, t, u = 0, 1, 0
      (1..n).each{|j|
        t += 9 * j
        u += j
        break if i <= u
        s += (-1) ** (j % 2 + 1) * (2 * j + 1) * (i - t) * ary[-u]
      }
      ary << s / (i - 1)
      a << i if ary[-1] % (i + 1) == 0
    else
      s = 1
      i.prime_division.each{|j|
        k, l = j[0], j[1]
        if l == 1
          s *= ary[k]
        else
          s *= ary[k ** (l - 1)] * ary[k] - k ** 11 * ary[k ** (l - 2)]
        end
      }
      ary << s
    end
  }
  a
end

n = 200
ary = A(n)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
