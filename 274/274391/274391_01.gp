\\ A(n,k) = Sum_{j=0..k-1} binomial(k-1,j) * k^j * A(n-1,k-j)
a(n, k) = if(n*k==0, 1, sum(j=0, k-1, binomial(k-1, j) * k^j * a(n-1, k-j)));
for(n=0, 10, for(k=0, n, print1(a(n-k, k), ", ")));

\\ a(n) = Sum_{k=0..n} binomial(n,k) * k*(k+1)^(k-1) * n^(n-k-1) for n>0 with a(0)=1.
a227176(n) = if(n==0, 1, sum(k=0, n, binomial(n, k) * k * (k+1)^(k-1) * n^(n-k-1)));
for(n=0, 10, print1(a227176(n), ", "));

\\ a(0) = 1; a(n) = Sum_{k=0..n-1} binomial(n-1,k) * n^k * A227176(n-k).
a268653(n) = if(n==0, 1, sum(k=0, n-1, binomial(n-1, k) * n^k * a227176(n-k)));
for(n=0, 10, print1(a268653(n), ", "));

\\ a(0) = 1; a(n) = Sum_{k=0..n-1} binomial(n-1,k) * n^k * A268653(n-k).
a268654(n) = if(n==0, 1, sum(k=0, n-1, binomial(n-1, k) * n^k * a268653(n-k)));
for(n=0, 10, print1(a268654(n), ", "));

{ITERATE(F, n, k) = my(G=x +x*O(x^k)); for(i=1,  n,  G=subst(G, x, F)); G}
{A(n, k) = my(TREE = serreverse(x*exp(-x +x*O(x^k)))); k!*polcoeff(exp(ITERATE(TREE, n, k)), k)}
/* Print this table as a square array */
for(n=0,  10,  for(k=0,  10,  print1(A(n, k), ", ")); print(""))

/* Print this table as a flattened array */
for(n=0,  10,  for(k=0,  n,  print1(A(n-k, k), ", ")); )
for(n=0,  10,  for(k=0,  n,  print1(A(n-k, k)-a(n-k, k), ", ")); )