\\ a(n) = Sum_{k>=1} (1/2)^k * binomial(k*n-1,n).
a(n) = sum(k=1, n+1000, (1/2.0)^k * binomial(k*n-1,n));                                                                            
for(n=0, 15, print1(round(a(n)),", "))   
for(n=0, 15, print1(a(n),", ")) 

