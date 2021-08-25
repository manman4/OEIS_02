def A(n)
  i = 0
  s = 1
  ary = [1]
  while i < n
    i += 1
    s *= i
    ary << s % (i + 3)
  end
  ary
end

n = 82
p ary = A(n)
(0..n).each{|i|
  j = ary[i]
  if j > 0
    p i if j != i / 2 + 1
  end
}
