# a(n) = 99*a(n-1) - 99*a(n-2) + a(n-3)
def A(n, a, b, c)
  ary = [a, b, c]
  (3..n).each{|i| ary << 99*ary[-1] - 99*ary[-2] + ary[-3]}
  ary
end

n = 500
# A261004
a_ary = A(n, -3, -461, -45343)
# # A269548
# p b_ary = A(n, -1, -233, -22961)
# # A269549
# p c_ary = A(n, 1, -199, -19799)
# # A269550
# p d_ary = A(n, 7, 465, 45347)
# # A269551
# p e_ary = A(n, 5, 237, 22965)
# # A269552
# p f_ary = A(n, 3, 203, 19803)
# # A269553
# p p_ary = A(n, -3, -435, -42763)
# # A269554
# p q_ary = A(n, -1, -343, -33861)
# # A269555
# p r_ary = A(n, 7, 439, 42767)
# # A269556
# p s_ary = A(n, 5, 347, 33865)
# # A010701
# p t_ary = [3] * (n + 1)

(0..n).each{|i|
  j = a_ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}