def s(k, n)
  s = 0
  (1..n).each{|i| s += i if n % i == 0 && i % k == 0}
  s
end

def A(n)
  ary = [1]
  a = [0] + (1..n).map{|i| s(1, i) - s(2, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s - 24 * a[j] * ary[-j]} / i}
  ary
end

def B(n)
  ary = [1]
  a = [0] + (1..n).map{|i| s(1, i) - s(2, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s + 24 * a[j] * ary[-j]} / i}
  ary
end

# -1����n
def C(n)
  ary = []
  a_ary = A(n + 1)
  b_ary = [0, 0] + B(n - 1)
 -1.upto(n){|i|
    ary << a_ary[i + 1] + 4096 * b_ary[i + 1] if i != 0
    ary << a_ary[i + 1] + 4096 * b_ary[i + 1] + 24 + 104 if i == 0
  }
  ary
end

def D(n)
  # -1����n - 2
  a = C(n)[0..-3]
  b = [0, 1]
  i = 1
  while i < n
    b << -(1..i).inject(0){|s, j| s + a[1 + i - j] * b[j]}
    i += 1
  end
  b
end

p D(20)
