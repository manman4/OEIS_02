def A(k, n)
  i = 0
  s = 1
  ary = []
  while ary.size < n
    i += 1
    s *= k
    str = s.to_s.split('')
    ary << i if str.include?("0") && str.include?("1") && str.include?("2") && str.include?("3") && str.include?("4") && str.include?("5") && str.include?("6") && str.include?("7") && str.include?("8") && str.include?("9")
  end
  ary
end

n = 10000
ary = A(5, n)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
