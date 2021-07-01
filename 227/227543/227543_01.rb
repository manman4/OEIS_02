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

def add(f_ary, b_ary, m)
  s1, s2 = f_ary.size, b_ary.size
  s = [s1, s2].max
  ary = Array.new(s, 0)
  (0..s1 - 1).each{|i| ary[i] += f_ary[i]}
  (0..s2 - 1).each{|i| ary[i] += b_ary[i]}
  ary[0..m]
end

def A(n)
  ary = [[1]]
  m = 10 ** 5
  (1..n).each{|i| ary << (0..i - 1).inject([0]){|s, j| add(s, [0] * j + mul(ary[j], ary[i - 1 - j], m), m)}}
  ary
end

n = 39
ary = A(n).flatten
(0..ary.size - 1).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
