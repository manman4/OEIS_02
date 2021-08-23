require 'prime'

def A(n)
  ary = [1, 2]
  s = 0
  while ary.size < n
    if !ary.include?(s)
      if (ary[-2].to_s + ary[-1].to_s + s.to_s).to_i.prime?
        ary << s
        s = 0
      end
    end
    s += 1
  end
  ary[0..n - 1]
end

n = 1000
ary = A(n)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
