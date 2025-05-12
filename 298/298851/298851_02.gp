\\ a(n) = Sum_{k=0..2*n} (-1)^k * Stirling2(k+n,n) * Stirling2(3*n-k,n).
a(n) = sum(k=0, 2*n, (-1)^k * stirling(k+n, n, 2) * stirling(3*n-k, n, 2));;
for(n=0, 20, print1(a(n),", "))                   

