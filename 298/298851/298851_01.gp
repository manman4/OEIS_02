\\ a(n) = Sum_{k=0..2*n} (-n)^k * binomial(4*n,k) * Stirling2(4*n-k,2*n).
a(n) = sum(k=0, 2*n, (-n)^k * binomial(4*n,k) * stirling(4*n-k, 2*n, 2));;
for(n=0, 20, print1(a(n),", "))                   

