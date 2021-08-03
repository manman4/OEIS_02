require 'prime'

# @p_ary[0] = 2‚Æ‚È‚èˆÈ‰º‚Å‚Íg‚í‚È‚¢B
@p_ary = Prime.take(1000).to_a

def g(n)
  @p_ary[2 * n - 1]
end

def C(n)
  ary = [1]
  s_ary = [0]
  (1..n).each{|i|
    s_ary << s_ary[-1] + g(i)
  }
  m = s_ary[-1]
  a = Array.new(m + 1){0}
  a[0] = 1
  (1..n).each{|i|
    b = a.clone
    (0..[s_ary[i - 1], m - g(i)].min).each{|j| b[j + g(i)] += a[j]}
    a = b
    s_ary[i] % 2 == 0 ? ary << a[s_ary[i] / 2] : ary << 0
  }
  ary
end

def D(n)
  a = C(2 * n)
  (1..n).map{|i| a[2 * i] / 2}
end

n = 200
ary = D(n)
(1..n).each{|i|
 print i
 print ' '
 puts ary[i - 1]
}
