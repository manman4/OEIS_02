a000670(n) = sum(k=0, n, k!*stirling(n, k, 2));
for(n=0, 10, print1(a000670(n), ", "));
a(n) = 2^n*a000670(n)-sum(k=0, n-1, 2^k*(n-1-k)^n); 
for(n=0, 10, print1(a(n), ", "));
