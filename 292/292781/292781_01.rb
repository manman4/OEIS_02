def A292781(n)
  ary = [[1]]
  (1..n).each{|i|
    m = ((Math.sqrt(1 + 8 * i) - 1) / 2).to_i
    a = (1..m).map{|j| ary[i - j][j - 1]}
    ary << [-(1..m).inject(0){|s, j| s + (-1) ** (j % 2) * (2 * j + 1) * (i - 9 * j * (j + 1) / 2) * a[j - 1]} / i] + a
  }
  ary.flatten
end
p A292781(20)
n = 1
ary = A292781(n)
(0..ary.size - 1).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
