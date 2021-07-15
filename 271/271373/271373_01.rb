def A(n)
  # 0‚ðŠÜ‚Þ
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
    ary << a.size if a.size > 0
  end
  ary
end

ary = (2..16).map{|i| A(i)}.flatten
(1..ary.size).each{|i|
  print i + 1
  print ' '
  puts ary[i - 1]
}
