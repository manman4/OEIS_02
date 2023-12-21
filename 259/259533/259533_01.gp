A000670(n) = polcoef(sum(m=0, n, m!*x^m/prod(k=1, m, 1-k*x+x*O(x^n))), n);
for(n=0, 20, print1(A000670(n), ", "));

a(n) = 8*A000670(n) - (2^n + 2 + 4*0^n);
for(n=0, 20, print1(a(n), ", "));