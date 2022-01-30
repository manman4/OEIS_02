# a(0)=1
def A(n)
  ary = [1, 1]
  (2..n).each{|i|
    s = 0
    (1..i - 1).each{|j|
      s += ary[j] if (i - 1) % j == 0
    }
    ary << s
  }
  ary
end

def B(n)
  a = A(n)
  ary = []
  (1..n).each{|i|
    (1..i).each{|j|
      if i % j == 0
        ary << a[i / j]
      else
        ary << 0
      end
    }
  }
  ary
end

n = 140
ary = B(n)
(1..ary.size).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
