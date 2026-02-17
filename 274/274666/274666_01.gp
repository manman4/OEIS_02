\\ a(n) = Sum_{k=0..n} binomial(n+k,k) * Sum_{j=0..n-k} binomial(n+j,j) * multinomial(n; j, k, n-j-k).
a(n) = n!*sum(k=0, n, binomial(n+k, k)*sum(j=0, n-k, binomial(n+j, j)/(j!*k!*(n-j-k)!))); 
for(n=0, 30, print1(a(n), ", "));