\\ a(n) = Sum_{k=0..n} Stirling2(n+k,2*k).
a(n) = sum(k=0, n, stirling(n+k, 2*k, 2));
for(n=0, 20, print1(a(n), ", "));