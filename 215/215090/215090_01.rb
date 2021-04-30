def f(k, ary)
  (0..ary.size - 1).inject(0){|s, i| s += ary[i] * k ** i}
end
def A(k, n)
  (0..n).map{|i| f(k, i.to_s(k + 1).split('').map(&:to_i).reverse)}
end

n = 10000
ary = A(3, n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
