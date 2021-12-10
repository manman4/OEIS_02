def A(n)
  ary = []
  (1..n).each{|i|
    (i + 1..n).each{|j|
      if i.gcd(j) == 1 && (i - j) % 3 > 0
        x, y, z = j * j, i * j, i * i
        b = x + y + y
        c = x + y + z 
        a = y + y + z
        ary << [    a, b, c]
        ary << [b - a, b, c]
      end
    }
  }
  ary
end

n = 1494
ary = A(1000).sort{|a, b|
  (a[1] <=> b[1]).nonzero? ||
    (a[0] <=> b[0])
}.flatten[0..n - 1]
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
