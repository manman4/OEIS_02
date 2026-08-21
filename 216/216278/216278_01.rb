def A(n)
  m = Math.sqrt(n).floor + 1
  ary = Array.new(n + 1, 0)
  (1..m).each{|i|
    (1..m).each{|j|
      k = i * i + 2 * j * j
      if k <= n
        ary[k] += 1
      end
    }
  }
  ary
end

n = 11000
m = 10000
ary = A(n)
(0..m).each{|i|
  print i
  print ' '
  puts ary[i]
}
