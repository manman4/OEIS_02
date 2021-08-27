def A001158(n)
  s = 0
  (1..n).each{|i| s += i * i * i if n % i == 0}
  s
end

def A288485(n)
  a = [0] + (1..n).map{|i| A001158(i)}
  ary = a.clone
  (1..n).each{|i|
    ary[i] -= 28 * a[i / 2] if i % 2 == 0
    ary[i] += 63 * a[i / 3] if i % 3 == 0
    ary[i] -= 36 * a[i / 6] if i % 6 == 0
  }
  ary[1..-1]
end

n = 10000
ary = A288485(n)
(1..n).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
