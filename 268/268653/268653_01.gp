\\ a(0) = 1; a(n) = Sum_{k=0..n-1} (-n)^(n-1-k) * (k+2)^k * binomial(n-1,k).
a340753(n) = if(n==0, 1, sum(k=0, n-1, (-n)^(n-1-k) * (k+2)^k * binomial(n-1, k)));
for(n=0, 20, print1(a340753(n), ", "));

\\ a(0) = 1; a(n) = Sum_{k=0..n-1} n^(n-1-k) * (k+2)^k * binomial(n-1,k).
a227176(n) = if(n==0, 1, sum(k=0, n-1, n^(n-1-k) * (k+2)^k * binomial(n-1, k)));
for(n=0, 20, print1(a227176(n), ", "));

print("-------")

\\ a(0) = 1; a(n) = Sum_{i,j,k >= 0 and i+j+k=n-1} ((n-1)!/(i!*j!*k!)) * n^i * (n-i)^j * (k+2)^k. 
a268653(n) = {
  if(n==0, 1,
    sum(i=0, n-1,
      sum(j=0, n-1-i,
        my(k=n-1-i-j);
        ((n-1)!/(i!*j!*k!)) * n^i * (n-i)^j * (k+2)^k
      )
    )
  )
};
for(n=0, 20, print1(a268653(n), ", "));
