# Pell number
def pell(n)
  ary = [0, 1]
  (2..n).each{|i|
    ary[i] = 2 * ary[-1] + ary[-2]
  }
  ary
end

n = 100
ary = pell(n * n)
(1..n).each{|i|
  j = ary[i * i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}