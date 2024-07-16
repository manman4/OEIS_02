a(n) = sum(k=0, n, k/(n+k)*binomial(n+k, k)^2)/(n+1);           
for(n=1, 15, print1(a(n),", "))              
