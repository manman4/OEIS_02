def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def s(k, n)
  s = 0
  (1..n).each{|i| s += i ** (k * n / i) if n % i == 0}
  s
end

def A(k, n)
  ary = [1]
  s_ary = [0] + (1..n).map{|i| s(k, i)}
  (1..n).each{|i|
    ary << (f(i - 1) * (1..i).inject(0){|s, j| s + s_ary[j] * ary[-j] / f(i - j).to_r}).to_i
  }
  ary
end


n = 50
ary = A(1, n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}

