\\ A(n,k) = (1/4)^n * Sum_{j=0..n} (k-2)^j * (k+2)^(n-j) * binomial(2*j,j) * binomial(2*(n-j),n-j).
a(n, k) = (1/4)^n * sum(j=0, n, (k-2)^j * (k+2)^(n-j) * binomial(2*j,j) * binomial(2*(n-j),n-j))
for(n=0, 10, for(k=0, n, print1(a(k,n-k),", ")));
