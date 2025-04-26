M=11;

\\ A(n,k) = n! * Sum_{j=0..n} (-k)^(n-j) * binomial(j+k-1,j)/(n-j)!.
a(n, k) = n! * sum(j=0, n, (-k)^(n-j) * binomial(j+k-1,j)/(n-j)!);
for(n=0, M, for(k=0, n, print1(a(k, n-k),", ")))


