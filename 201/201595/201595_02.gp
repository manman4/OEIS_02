M=17;

\\ a(n) = n! * Sum_{k=0..n} 2^(n-k) * Stirling2(n,k)/(n-k+1)!.
a(n) = n! * sum(k=0, n, 2^(n-k) * stirling(n,k,2)/(n-k+1)!);
for(n=0, M, print1(a(n),", "));

