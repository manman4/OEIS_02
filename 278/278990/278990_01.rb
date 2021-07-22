def C(n, r)
  r = [r, n - r].min
  return 1 if r == 0
  return n if r == 1
  numerator = (n - r + 1..n).to_a
  denominator = (1..r).to_a
  (2..r).each{|p|
    pivot = denominator[p - 1]
    if pivot > 1
      offset = (n - r) % p
      (p - 1).step(r - 1, p){|k|
        numerator[k - offset] /= pivot
        denominator[k] /= pivot
      }
    end
  }
  result = 1
  (0..r - 1).each{|k|
    result *= numerator[k] if numerator[k] > 1
  }
  return result
end

def mul(f_ary, b_ary)
  s1, s2 = f_ary.size, b_ary.size
  ary = Array.new(s1 + s2 - 1, 0)
  (0..s1 - 1).each{|i|
    (0..s2 - 1).each{|j|
      ary[i + j] += f_ary[i] * b_ary[j]
    }
  }
  ary
end

def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def A(k, n)
  a = [1]
  ary = [1]
  ary0 = [0] + (1..k).map{|i| (-1) ** (k - i) * C(k - 1, k - i) / f(i).to_r}
  n.times{|i|
    ary = mul(ary, ary0)
    a << (0..ary.size - 1).inject(0){|s, j| s + f(j) * ary[j]}.to_i / f(i + 1)
  }
  a
end

n = 500
ary = A(2, n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
