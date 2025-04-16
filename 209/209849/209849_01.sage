def a_row(n):
    s = sum(2^(n-k)*stirling_number2(n, k)*falling_factorial(x, k) for k in (0..n))
    return expand(s).list()[1:]
for n in (1..10): print(a_row(n)) 
