require "prime"

def f(n)
  Prime.int_from_prime_division(
    n.prime_division.map{|p,q| [p, q / 2 * 2]}
  )
end

def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

n = 58
# b=[]
(0..n).each{|i|
  j = f(ncr(2 * i, i))
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
# b<<j
}
# p b
# p b.map{|i| Math.sqrt(i).to_i}
