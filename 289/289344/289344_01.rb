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

c_ary = [0] +
[24,348,6424,129300,2778648,62114524,1428337176,
 33527349924,799482197272,19302454317660,
 470740035601176,11575875047000596,
 286650683468840472,7140515309818664028,
 178783562850377621272,4496350112540599930692]
c_ary = c_ary.map{|i| i / 2}
n = c_ary.size - 1
ary = [1]
(1..n).each{|i|
  a = [1] + [0] * (i - 1) + [-1]
  b = power(a, c_ary[i], n)
  ary = mul(ary, b, n)
}
p ary

def s(n)
  s = 0
  (1..n).each{|i| s += i if n % i == 0}
  s
end

def A(k, n)
  ary = [1]
  a = [0] + (1..n).map{|i| s(i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s - k * a[j] * ary[-j]} / i}
  ary
end

b = A(-1, n)
ary = mul(ary, b, n)

(1..24).each{|i|
  # i=1のときA289344
  p [i, power(ary, i, n)]
}
