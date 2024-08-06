\\ a(n) = Sum_{k>=0} (1/2)^(k+1) * binomial(n-1+7*k,n).
a(n) = sum(k=0, n+1000, (1/2.0)^(k+1) * binomial(n-1+7*k,n));                                                                                  
for(n=0, 25, print1(round(a(n)),", "))   
for(n=0, 25, print1(a(n),", ")) 

