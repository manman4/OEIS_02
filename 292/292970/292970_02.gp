M=30;

a(n) = n!*sum(k=0, n\5, (-k)^(n-5*k)/(k!*(n-5*k)!)); 
for(n=0, M, print1(a(n), ", "));