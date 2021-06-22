def square?(n)
  Math.sqrt(n).to_i ** 2 == n
end

def A275028(n)
  ary = []
  a = [0, 1, 2, 3, 3, 5, 9, 3, 8, 6]
  i = 1
  while ary.size < n
    if i % 10 > 0
      j = i * i
      k = j.to_s.split('').map(&:to_i)
      if !k.include?(3) && !k.include?(4) && !k.include?(7)
        m = k.reverse.map{|i| a[i]}.join.to_i
        ary << j if square?(m)
      end
    end
    i += 1
  end
  ary
end

n = 26
ary = A275028(n)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
