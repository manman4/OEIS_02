\\ a(n) = Sum_{k=0..n} binomial(n,k) * binomial(7*k/2+1/2,n)/(7*k+1). 
a(n) = sum(k=0, n, binomial(n,k)*binomial(7*k/2+1/2,n)/(7*k+1));
for(n=0, 50, print1(a(n), ", ")) 