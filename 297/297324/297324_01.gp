default(parisize, 120000000)

a(n) = polcoef(prod(k=1,n, 1-k*x^k+x*O(x^n))^n, n);

print(a(1000))