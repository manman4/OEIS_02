M=21;

\\ a(n) = (-25)^n * binomial(n/5+1/5,n)/(n+1).
a(n) = (-25)^n * binomial(n/5+1/5,n)/(n+1);
for(n=0, M, print1(a(n), ", ")) 

