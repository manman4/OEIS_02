\\ a(n) = Sum_{k=0..n} (-1)^k * binomial(n,k) * Sum_{j=0..n-k} multinomial(3*n-j-2*k; j, n, {n-j-k}^2).
a(n) = sum(k=0, n, (-1)^k*binomial(n, k)*sum(j=0, n-k, (3*n-j-2*k)!/(j!*(n-j-k)!^2)))/n!;
for(n=0, 18, print1(a(n),", "));