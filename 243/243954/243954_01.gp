M=21;

\\ a(n) = n! * Sum_{k=0..n-1} (-1)^(n-1-k) * n^(k-1) * binomial(n-2,n-1-k)/k! for n > 0.
a(n) = if(n==0, 1, n!*sum(k=0, n-1, (-1)^(n-1-k)*n^(k-1)*binomial(n-2, n-1-k)/k!));
for(n=0, 20, print1(a(n),", "));
