require 'prime'

def A(n)
  ary = [0, 0]
  # 1 + i
  cnt = 1
  (2..n).each{|i|
    sq = Math.sqrt(i).to_i
    if i.prime? && i % 4 == 1
      s = 0
      (1..sq).each{|j|
        # i = q * q�ƂȂ�Ȃ��̂ŁAk > 0
        k = i - j * j
        s += 1 if Math.sqrt(k).to_i ** 2 == k
      }
      cnt += s
    elsif sq ** 2 == i
      cnt += 1 if sq.prime? && sq % 4 == 3
    end
    ary << cnt
  }
  ary[0..n]
end

n = 4
ary = A(4 * 10 ** n + 1)
(0..10 ** n).each{|i|
  print i
  print ' '
  puts ary[4 * i + 1]
}
