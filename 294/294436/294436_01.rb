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

def A(k, n)
  s = 0
  (0..n).each{|i|
    t = 0
    (0..i).each{|j|
      t += C(n, j)
    }
    s += t ** k
  }
  s
end

n = 300
(0..n).each{|i|
  j = A(5, i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
