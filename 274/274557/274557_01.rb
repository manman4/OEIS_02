require 'prime'

def sum_of_divisors(n)
  n.prime_division.inject(1){|s, a| s * (a[0] ** (a[1] + 1) - 1) / (a[0] - 1)}
end

n = 52
m, cnt = 2, 0
while cnt < n
  if sum_of_divisors(m) % (m + 6) == 0
    cnt += 1
    print cnt
    print ' '
    puts m
  end
  m += 1
end
