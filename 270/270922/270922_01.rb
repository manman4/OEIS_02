def s(k, m, n)
  s = 0
  (1..n).each{|i| s += (-1) ** (n / i + 1) * i * i if n % i == 0 && i % k == m}
  s
end

def A(k, n)
  if k == 0
    ary = Array.new(n + 1, 0)
    ary[0] = 1
    return ary
  else
    ary = [1]
    s_ary = [0] + (1..k * n).map{|i| s(k, 0, i)}
    (1..k * n).each{|i| ary << (1..i).inject(0){|s, j| s + ary[-j] * s_ary[j]} / i}
    return (0..n).map{|i| ary[i * k]}
  end
end

n = 23
(0..n).each{|i|
  j = A(i, i)[-1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
