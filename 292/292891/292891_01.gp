M=30;

a(n) = n!*sum(k=0, n\4, stirling(n-3*k, k, 2)/(n-3*k)!); 
for(n=0, M, print1(a(n), ", "));