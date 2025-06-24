M=21;

a136630(n, k) = 1/(2^k*k!)*sum(j=0, k, (-1)^(k-j)*(2*j-k)^n*binomial(k, j));
for(n=0, 10, for(k=0, n, print1(a136630(n, k),", ")); print);

\\ a(n) = (1/(n+1)) * Sum_{k=0..n} k! * binomial(n+1,k) * i^(n-k) * A136630(n,k), where i is the imaginary unit.
a(n) = (1/(n+1)) * sum(k=0, n, k! * binomial(n+1, k) * I^(n-k) * a136630(n, k)); 
for(n=0, M, print1(a(n), ", "));



