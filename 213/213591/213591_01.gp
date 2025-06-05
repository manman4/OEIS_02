M=20;

\\ b(n,0) = 0^n; b(n,k) = k * Sum_{j=0..n} binomial(n+j+k,j)/(n+j+k) * b(n-j,2*j).
\\ a(n) = b(n-1,1).
b(n, k) = if(k==0, 0^n, k*sum(j=0, n, binomial(n+j+k, j)/(n+j+k)*b(n-j, 2*j)));
a(n) = b(n-1, 1);
for(n=1, M, print1(a(n),", "));

\\ G.f. A(x) satisfies A(x) = 1/( 1 - x*A(x) * A(x*A(x))^2 ).
c(n, k) = my(A=1); for(i=1, n, A = 1/(1 - x * A * subst(A, x, x*A)^2 + x*O(x^n))); polcoef(A^k, n);
for(n=0, 10, for(k=0, 15, print1(c(n, k)-b(n, k),", ")); print);

