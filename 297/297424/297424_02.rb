require 'prime'

def s(n)
  s = 0
  (1..n).each{|i| s += i if n % i == 0}
  s
end

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
  p_ary = [0] + Prime.take(n).to_a
  m = p_ary[-1]
  ary = A(m)
  (1..n).map{|i| ary[p_ary[i]]}
end

n = 4
ary = B(10 ** n)
b = []
(0..n).each{|i|
  print i
  print ' '
  puts ary[10 ** i - 1]
  b << ary[10 ** i - 1]
}
p b
