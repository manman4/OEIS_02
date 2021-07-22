def ncr(n, r)
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

def A(n)
  ary = []
  (0..n).each{|i|
    if i % 2 == 0
      j = i / 2
      ary << (ncr(4 * j, j).to_r / ncr(6 * j, 3 * j)) ** 2
    else
      j = (i - 1) / 2
      ary << -((ncr(4 * j + 1, j).to_r / ncr(6 * j + 1, 3 * j)) ** 2) / 3
    end
  }
  ary
end

n = 50
ary = A(n)
(0..n).each{|i|
  j = ary[i].denominator
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
