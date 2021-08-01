def A(k, n)
  ary = []
  i = 0
  while ary.size < n
    i += 1
    j = (i * i * i).to_s.split('').map(&:to_i).inject(:+)
    ary << i if j == k
  end
  ary
end

n = 10000
ary = A(64, n)
(1..n).each{|i|
  print i
  print " "
  puts ary[i - 1]
}
