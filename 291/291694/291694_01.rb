def A(n)
  cnt = 1
  (1..n).each{|k|
    (1..k).each{|j|
      (1..j).each{|i|
        break if i * j > k
        if i * i + j * j + k * k == 3 * i * j * k
          print cnt
          print ' '
          puts i
          print cnt + 1
          print ' '
          puts j
          print cnt + 2
          print ' '
          puts k
          cnt += 3
        end
      }
    }
  }
  ary
end

A(500000000)

