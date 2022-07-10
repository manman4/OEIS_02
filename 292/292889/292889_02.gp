M=30;

a(n) = n!*sum(k=0, n\3, k^(n-3*k)/(k!*(n-3*k)!)); 
for(n=0, M, print1(a(n), ", "));