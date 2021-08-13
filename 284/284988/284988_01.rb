def A(n)
  ary = []
  (1..n).each{|a|
    (1..a).each{|b|
      (1..b).each{|c|
        (1..c).each{|d|
          (1..d).each{|e|
            ary << a * b * c * d * e
          }
        }
      }
    }
  }
  ary.uniq.size
end

n = 100
(1..n).each{|i|
  print i
  print ' '
  puts A(i)
}
