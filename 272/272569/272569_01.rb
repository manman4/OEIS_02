def A272569(n)
  ary = [0, 0, 0]
  (2..n).each{|i|
    if i.even?
      ary[i] = ary[i / 2]
    else
      j = (i - 1) / 2
      k, l = ary[j], ary[j + 1]
      ary[i] = k + l + Math.sqrt(4 * k * l + 1).to_i
    end
  }
  ary[1..n]
end

n = 100
p ary = A272569(n)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
