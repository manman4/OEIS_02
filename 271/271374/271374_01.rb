def A(n)
  # 0���܂�
  ary = [n]
  cnt = 0
  a = (1..n - 1).map{|i| [i]}
  while a.size > 0
    cnt += 1
    b = []
    a.each{|i|
      (0..n - 1).each{|j|
        k = i.clone + [j]
        b << k if (0..cnt).inject(0){|s, m| s + k[m] * n ** (cnt - m)} % (cnt + 1) == 0
      }
    }
    a = b
    # p a.map{|i| i.join.to_i}
    ary << a.size if a.size > 0
  end
  ary.inject(:+)
end

b=[]
(2..4).each{|i|
  print i
  print ' '
  puts j = A(i)
b<<j
}
p b
