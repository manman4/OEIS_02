def A276401(n)
  i = 2
  ary = []
  while i < n
    i += 1
    if i % 3 == 0
      ary << 3 ** (i / 3) + 12 * i
    elsif i % 3 == 1
      ary << 4 * 3 ** ((i - 4) / 3) + 12 * i + 51
    else
      ary << 2 * 3 ** ((i - 2) / 3) + 12 * i - 36
    end
  end
  ary
end

n = 7000
ary = A276401(n)
(3..n).each{|i|
  j = ary[i - 3]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
