def f(k, n)
  (2 * n - 1) ** k
end

def A(k, n)
  ary = [1]
  s_ary = [0]
  (1..n).each{|i|
    s_ary << s_ary[-1] + f(k, i)
  }
  m = s_ary[-1]
  a = Array.new(m + 1){0}
  a[0] = 1
  (1..n).each{|i|
p i
    b = a.clone
    (0..[s_ary[i - 1], m - f(k, i)].min).each{|j| b[j + f(k, i)] += a[j]}
    a = b
    s_ary[i] % 2 == 0 ? ary << a[s_ary[i] / 2] : ary << 0
  }
  ary
end

n = 20
ary = A(3, 2 * n)
b = []
(0..n).each{|i|
  print i
  print ' '
  puts j = ary[2 * i]
b<<j
}
p b
