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

def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
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

def A294212(n)
  a = []
  (0..n).each{|i| a << B(i, n - i)}
  ary = []
  (0..n).each{|i|
    (0..i).each{|j|
      ary << a[i - j][j]
    }
  }
  ary
end
n = 10
ary = A294212(n)
(0..ary.size - 1).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
