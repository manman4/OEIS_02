def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end


def A268504(n)
  s = 1
  ary = [1]
  (1..n).each{|i|
    s *= f(3 * i)
    ary << s
  }
  ary
end

n = 30
ary = A268504(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
