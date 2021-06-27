# mŸˆÈ‰º‚ğæ‚èo‚·
def mul(f_ary, b_ary, m)
  s1, s2 = f_ary.size, b_ary.size
  ary = Array.new(s1 + s2 - 1, 0)
  (0..s1 - 1).each{|i|
    (0..s2 - 1).each{|j|
      ary[i + j] += f_ary[i] * b_ary[j]
    }
  }
  ary[0..m]
end

def A131945(n)
  ary5 = [1]
  3.step(n, 6){|i|
    b_ary = Array.new(i + 1, 0)
    b_ary[0], b_ary[-1] = 1, -1
    ary5 = mul(ary5, b_ary, n)
  }
  ps = Array.new(n + 1){0}
  ps[0] = 1
  (1..n).each{|num|
    (num..n).each{|i|
      ps[i] += ps[i - num]
    }
  }
  mul(ary5, ps, n)
end

def A279320(n)
  a = A131945(2 * n)
  (0..n).map{|i| a[2 * i]}
end

n = 40
p ary = A279320(n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
