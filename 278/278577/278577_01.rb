require 'prime'

def A(n)
  ary = [0, 1]
  (2..n).each{|i|
    a = i.prime_division
    if a.size == 1
      s, t, u = 0, 1, 0
      (1..n).each{|j|
        t += 9 * j
        u += j
        break if i <= u
        s += (-1) ** (j % 2 + 1) * (2 * j + 1) * (i - t) * ary[-u]
      }
      ary << s / (i - 1)
    else
      s = 1
      a.each{|j|
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
  ary
end

def B(n)
  a = [1]
  (2..10 ** 7).each{|i|
    break if a.size == n
    b = i.prime_division
    a << i if b.size == 1
  }
  t_ary = A(a[-1])
  (1..n).map{|i| t_ary[a[i - 1]]}
end

n = 10000
ary = B(n)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
