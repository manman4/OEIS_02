M=28;

a(n) = n!*sum(k=0, n\5, 1/5!^k*binomial(n-4*k, k)/(n-4*k)!); 
for(n=0, M, print1(a(n), ", "));