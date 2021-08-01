def P(n)
  s1, s2 = 2, 8
  n.times{|i|
    m = 10 ** (i + 1)
    (0..9).each{|j|
      k1, k2 = j * m + s1, (9 - j) * m + s2
      if (k1 ** 5 - k1) % (m * 10) == 0 && (k2 ** 5 - k2) % (m * 10) == 0
        s1, s2 = k1, k2
        break
      end
    }
  }
  s1
end
def Q(s, n)
  n.times{|i|
    m = 10 ** (i + 1)
    (0..9).each{|j|
      k = j * m + s
      if (k ** 2 - k) % (m * 10) == 0
        s = k
        break
      end
    }
  }
  s
end
def A290374(n)
  str = (P(n) + Q(5, n)).to_s.reverse
  (0..n).map{|i| str[i].to_i}
end
p A290374(100)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
