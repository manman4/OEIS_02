\\ A(n,k) = Sum_{j=0..floor(n/2)} k^(n-2*j) * binomial(n,2*j) * binomial(2*j,j).
a(n, k) = sum(j=0, n\2, k^(n-2*j) * binomial(n,2*j) * binomial(2*j,j))
for(n=0, 10, for(k=0, n, print1(a(k,n-k),", ")));
