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

def Leibniz_formula(n)
  4 * (0..3 * n - 1).inject(0){|s, i| s + (-1) ** i / (2 * i + 1r)}
end

def f(n)
  (0..2 * n - 1).inject(0){|s, i| s + (-1) ** i / (2 ** (2 * n - i - 2) * (8 * n - i - 1r) * C(8 * n - i - 2, 4 * n + i))}
end

n = 20
ary = (1..n).map{|i| Leibniz_formula(i) + (-1) ** (i + 1) * f(i)}
(1..n).each{|i|
  j = ary[i - 1].numerator
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
