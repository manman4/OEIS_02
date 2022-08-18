M=23;

a(n) = n!*sum(k=0, n\2, (n-2*k)^k/(k!*(n-2*k)!));
for(n=1, M, print1(a(n), ", "));