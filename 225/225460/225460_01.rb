# x‚©‚çŠJn‚µyæ‚µ‚Äp/q
def A(x, y, p, q, base, n)
  s = x
  n.times{|i|
    m = base ** (i + 1)
    (0..base - 1).each{|j|
      k = j * m + s
      if (q * k ** y - p) % (m * base) == 0
        s = k
        break
      end
    }
  }
  s
end

def A225460(n)
  str = A(3, 9, -7, 1, 10, n).to_s.reverse
  (0..n).map{|i| str[i].to_i}
end

n = 10100
ary = A225460(n)
(0..10000).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
