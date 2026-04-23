EULERIAN_MEMO = {[0, 0] => 1}
COMB_MEMO = {}

def comb(n, r)
  return 0 if r < 0 || r > n
  r = [r, n - r].min
  key = [n, r]
  return COMB_MEMO[key] if COMB_MEMO.key?(key)
  num = 1
  den = 1
  1.upto(r){|i|
    num *= n - r + i
    den *= i
  }
  COMB_MEMO[key] = num / den
end

def eulerian(n, k)
  return 1 if n == 0 && k == 0
  return 0 if n <= 0 || k < 0 || k >= n
  key = [n, k]
  return EULERIAN_MEMO[key] if EULERIAN_MEMO.key?(key)
  EULERIAN_MEMO[key] =
    (n - k) * eulerian(n - 1, k - 1) +
    (k + 1) * eulerian(n - 1, k)
end

def A(n, k)
  return 1 if n == 0 && k == 0
  return 0 if n < 0 || k < 0 || k >= n

  # derangement かつ excedance が k 個の個数
  # = sum_{j=0..n} (-1)^j C(n,j) * Eulerian(n-j, k)
  s = 0
  0.upto(n){|j|
    term = comb(n, j) * eulerian(n - j, k)
    s += j.even? ? term : -term
  }
  s
end
n = 139
ary = []
(0..n).each{|i|
  (0..i).each{|j|
    ary << A(i, j)
  }
}
(0..ary.size - 1).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
