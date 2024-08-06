\\ A(n,k) = Sum_{j>=0} (1/2)^(j+1) * binomial(n-1+k*j,n). 
a(n,k) = sum(j=0, n+1000, (1/2.0)^(j+1) * binomial(n-1+k*j,n));                                                                              
for(n=0, 10, for(k=0, n, print1(round(a(k,n-k)),", ")))  
for(n=0, 10, for(k=0, n, print1(a(k,n-k),", "))) 

