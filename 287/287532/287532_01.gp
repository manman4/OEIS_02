\\ A(n,k) = Sum_{j=0..n} Stirlin2(j+k-1,k-1) * Stirling2(n-j+k,k) for k >= 1.

a(n, k) = sum(j=0, n, stirling(j+k-1, k-1, 2)*stirling(n-j+k, k, 2));;
for(n=0, 9, for(k=0, n, print1(a(n-k, k+1),", ")));                   

