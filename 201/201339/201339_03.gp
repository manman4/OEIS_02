A(n) = polcoef(sum(m=0, n, 2^m*m!*x^m/prod(k=1, m, 1-k*x+x*O(x^n))), n);
A004123(n) = A(n-1);
\\ offset 1に注意
for(n=1, 20, print1(A004123(n), ", "));

a(n) = (3/2)*A004123(n+1) - (1/2)*0^n;
for(n=0, 20, print1(a(n), ", "));