\\ A(n,k) = n! * Sum_{j=0..n} binomial(n+k-1,j)/(n-j)!.
a(n, k) = n!*sum(j=0, n, binomial(n+k-1,j)/(n-j)!);
for(n=0, 9, for(k=0, n, print1(a(k, n-k),", ")))