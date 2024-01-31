bell(n, x) = sum(k=0, n, x^k * stirling(n, k, 2));
\\ a(n) = Sum_{k=0..n} 3^k * Stirling1(n,k) * Bell_k(-1/3)
a(n) = sum(k=0, n, 3^k * stirling(n, k, 1) * bell(k, -1/3));
for(n=0, 20, print1(a(n), ", "));