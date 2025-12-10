\\ a(n) = (1/n) * Sum_{k=1..n} k * Pell(k+1) * binomial(2*n,n-k) for n > 0. 
pell(n) = ([2, 1; 1, 0]^n)[2, 1];
for(n=0, 20, print1(pell(n), ", ")); 
a(n) = if(n==0, 1, sum(k=1, n, k*pell(k+1)*binomial(2*n, n-k))/n);
for(n=0, 20, print1(a(n),", "))  