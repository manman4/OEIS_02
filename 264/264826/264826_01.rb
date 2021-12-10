def A(n)
  ary = []
  (1..n).each{|i|
    (i + 1..n).each{|j|
      if i.gcd(j) == 1 && (i - j) % 3 > 0
        x, y, z = j * j, i * j, i * i
        b = x + y + y
        c = x + y + z 
        a = y + y + z
        ary << [    a, c, b]
        ary << [b - a, c, b]
      end
    }
  }
  ary
end

n = 9999
ary = A(1000).sort.flatten[0..n - 1]
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
