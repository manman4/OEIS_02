M=19;

a136630(n, k) = 1/(2^k*k!)*sum(j=0, k, (-1)^(k-j)*(2*j-k)^n*binomial(k, j));
for(n=0, 10, for(k=0, n, print1(a136630(n, k),", ")); print);

a008542(n) = prod(k=0, n-1, 6*k+1);

\\ a(n) = Sum_{k=0..n} A008542(k) * (6*i)^(n-k) * A136630(n,k), where i is the imaginary unit.
a(n) = sum(k=0, n, a008542(k) * (6*I)^(n-k) * a136630(n, k));
for(n=0, M, print1(a(n), ", "));



