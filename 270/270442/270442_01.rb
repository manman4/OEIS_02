require 'prime'

def A(k, n)
  p k
  (0..k + 1).each{|i| p (n * n - i).prime_division}
end

# 確認
A(8, 30167284)
A(9, 46952141)
