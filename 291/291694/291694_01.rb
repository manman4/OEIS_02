def A(n)
  ary = []
  (1..n).each{|k|
    (1..k).each{|j|
      (1..j).each{|i|
        break if i * j > k
        ary << [i, j, k] if i * i + j * j + k * k == 3 * i * j * k
      }
    }
  }
  ary
end

p A(14701).flatten

