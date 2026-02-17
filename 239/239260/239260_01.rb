# m>0

def s(f_ary, g_ary, n, m)
  s = 0
  (m..n).each{|i| s += i * f_ary[i] * g_ary[i] ** (n / i) if n % i == 0}
  s
end

def A(ary, n, m = 1)
  a_ary = [1]
  a = [0] + (1..n).map{|i| ary.inject(0){|s, j| s + s(j[0], j[1], i, m)}}
  (1..n).each{|i| a_ary << (1..i).inject(0){|s, j| s + a[j] * a_ary[-j]} / i}
  a_ary
end

# aa(n) = Sum_{k=0..floor((n-1)/4)} A000009(n-2*k) * A000041(k).
def a(n)
  ary = []
  ary1 = Array.new(n + 1, 1)
  ary2 = Array.new(n + 1, -1)
  a_odd = A([[ary2, ary2]], n)
  a_all = A([[ary1, ary1]], n)
  (0..n).each{|i|
    ary << (0..(i - 1) / 4).inject(0){|s, k| s + a_odd[i - 2 * k] * a_all[k]}
  }
  (0..n).map{|i| a_all[i] - ary[i]}
end

n = 10000
ary = a(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}