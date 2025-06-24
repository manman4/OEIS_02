M=19;

a136630(n, k) = 1/(2^k*k!)*sum(j=0, k, (-1)^(k-j)*(2*j-k)^n*binomial(k, j));
for(n=0, 10, for(k=0, n, print1(a136630(n, k),", ")); print);

a045755(n) = prod(k=0, n-1, 8*k+1);

\\ a(n) = Sum_{k=0..n} A045755(k) * (8*i)^(n-k) * A136630(n,k), where i is the imaginary unit.
a(n) = sum(k=0, n, a045755(k) * (8*I)^(n-k) * a136630(n, k));
for(n=0, M, print1(a(n), ", "));



