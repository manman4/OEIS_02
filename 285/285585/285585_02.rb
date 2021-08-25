def s(k, m, n)
  s = 0
  (1..n).each{|i| s += i if n % i == 0 && i % k == m}
  s
end

def A(k, n)
  ary = [1]
  a = [0] + (1..n).map{|i| s(5, 1, i) + s(5, 4, i) - s(5, 2, i) - s(5, 3, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s - a[j] * ary[-j]} / i}
  a_ary = Array.new(n + 1, 0)
  (0..n / k).each{|i| a_ary[k * i] = ary[i]}
  a_ary
end

def B(k, n)
  ary = [1]
  a = [0] + (1..n).map{|i| s(5, 1, i) + s(5, 4, i) - s(5, 2, i) - s(5, 3, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s + k * a[j] * ary[-j]} / i}
  ary
end

def C(k, n)
  ary = [1]
  a = [0] + (1..n).map{|i| s(5, 1, i) + s(5, 4, i) - s(5, 2, i) - s(5, 3, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s - k * a[j] * ary[-j]} / i}
  ary
end

# m���ȉ������o��
def mul(f_ary, b_ary, m)
  s1, s2 = f_ary.size, b_ary.size
  ary = Array.new(s1 + s2 - 1, 0)
  (0..s1 - 1).each{|i|
    (0..s2 - 1).each{|j|
      ary[i + j] += f_ary[i] * b_ary[j]
    }
  }
  ary[0..m]
end

# m���ȉ������o��
def power(ary, n, m)
  return [1] if n == 0
  k = power(ary, n >> 1, m)
  k = mul(k, k, m)
  return k if n & 1 == 0
  return mul(k, ary, m)
end

n = 100

(1..5).each{|i|
  a = A(i, n)
  p b = B(i, n)
  p [i, mul(a, b, n)]
}

u = [0] + C(5, n)[0..-2]
v = A(5, n)
v1 = [0] * 1 + power(v, 1, n)[0..-2]
v2 = [0] * 2 + power(v, 2, n)[0..-3]
v3 = [0] * 3 + power(v, 3, n)[0..-4]
v4 = [1] + [0] * 3 + power(v, 4, n)[0..-5]
p x = (0..n).map{|i| v4[i] + 2 * v3[i] + 4 * v2[i] + 3 * v1[i]}
mul(u, x, n)

p y = (0..n).map{|i| v4[i] - 3 * v3[i] + 4 * v2[i] - 2 * v1[i]}
mul(v1, y, n)

p w = mul(A(5, n), B(5, n), n)
p mul(w, y, n)
