def f(k, n)
  n * ((k - 2) * n - k + 4) / 2
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
    b = a.clone
    (0..[s_ary[i - 1], m - f(k, i)].min).each{|j| b[j + f(k, i)] += a[j]}
    a = b
    s_ary[i] % 2 == 0 ? ary << a[s_ary[i] / 2] : ary << 0
  }
  ary
end

n = 46
ary = A(5, n)
(0..n).each{|i|
 print i
 print ' '
 puts ary[i]
}
