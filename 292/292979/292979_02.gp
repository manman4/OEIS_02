M=30;

a(n) = n!*sum(k=0, n\4, k^(n-4*k)/(k!*(n-4*k)!)); 
for(n=0, M, print1(a(n), ", "));