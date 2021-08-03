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

# nŸ‚Ü‚Å
def q(n)
  return [1] if n == 0
  ary = [1]
  (1..n).each{|i|
    b_ary = Array.new(i + 1, 0)
    b_ary[0], b_ary[-1] = 1, -1i
    ary = mul(ary, b_ary, n)
  }
  ary
end

n = 1000
ary = q(n).map{|i| i.imaginary}
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
