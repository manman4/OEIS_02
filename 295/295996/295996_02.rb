require 'prime'
def A(k, n)
  ary = []
  cnt = 0
  k.step(4 * n + k, 4){|i|
    cnt += 1 if i.prime?
    ary << cnt
  }
  ary
end
def A295996(n)
  ary1 = A(1, n)
  ary3 = A(3, Math.sqrt(n).to_i) + [0]
  [0] + (1..n).map{|i| 1 + 2 * ary1[i] + ary3[(Math.sqrt(4 * i + 1).to_i - 3) / 4]}
end

n = 67
ary = A295996(n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
