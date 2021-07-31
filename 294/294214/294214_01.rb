def A(k, n)
  ary = (1..k).to_a
  ps = Array.new(n + 1, 0)
  ps[0] = 1
  ary.each{|num|
    (num..n).each{|i|
      ps[i] += ps[i - num]
    }
  }
  ps
end

def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def B(k, n)
  p_ary = A(k, n)
  ary = [1]
  (1..n).each{|i|
    ary << (f(i - 1) * (1..i).inject(0){|s, j| s + j * p_ary[j] * ary[i - j] / f(i - j).to_r}).to_i
  }
  ary
end

n = 19
ary = B(3, n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
