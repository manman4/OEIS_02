\\ a(n) = Sum_{0 <= j,k <= n} binomial(n,j) * binomial(n,k) * multinomial(n+j+k; j, k, n).
a(n) = sum(j=0, n, sum(k=0, n, binomial(n, j)*binomial(n, k)*(n+j+k)!/(j!*k!)))/n!;
for(n=0, 17, print1(a(n),", "));