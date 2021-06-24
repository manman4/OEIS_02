n = 40
ary = (1..(n + 1) / 2).inject([]){|s, i| s + [(10 * i - 7) ** 2, (10 * i - 3) ** 2]}[0..n - 1]
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
