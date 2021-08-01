def bernoulli(n)
  ary = []
  a = []
  (0..n).each{|i|
    a << 1r / (i + 1)
    i.downto(1){|j| a[j - 1] = j * (a[j - 1] - a[j])}
    ary << a[0]
  }
  ary
end

def A036968(n)
  ary = bernoulli(n)
  [1] + (2..n).map{|i| (2 * (1 - 2 ** i) * ary[i]).to_i}
end

n = 700
g_ary = [0] + A036968(n)
(2..n).each{|i|
  j = 2 * i * g_ary[i - 1] + 2 * (i - 1) * g_ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
