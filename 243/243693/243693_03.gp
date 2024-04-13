\\ a(n) = (1/n) * Sum_{k=1..n} 3^(n-k) * binomial(n, k) * binomial(2*n, k-1) for n > 0.
a(n) = if(n==0, 1, sum(k=1, n, 3^(n-k) * binomial(n, k) * binomial(2*n, k-1) )/n);
for(n=0, 22, print1(a(n),", "))          

