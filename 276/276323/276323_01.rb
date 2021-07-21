require 'prime'

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

def A005259(n)
  i = 0
  a, b = 1, 5
  ary = [1]
  while i < n
    i += 1
    a, b = b, ((((34 * i + 51) * i + 27) * i + 5) * b - i ** 3 * a) / (i + 1) ** 3
    ary << a
  end
  ary
end

def A276323(n)
  p_ary = Prime.take(n + 3)[3..-1]
  a = A005259(p_ary[-1] - 1)
  ary = []
  p_ary.each{|i|
    j = C(2 * i, i) * a[i - 1] - 2
    break if j % i ** 5 > 0
    ary << j / i ** 5
  }
  ary
end

n = 100
ary = A276323(n)
(1..n).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
