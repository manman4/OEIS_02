n = 135
p (0..n).select{|i| i % 3 == 0 || i % 5 == 0}
