\\ a(0) = 1; a(n) = Sum_{k=0..n-1} (-n)^(n-1-k) * (k+2)^k * binomial(n-1,k).
a340473(n) = if(n==0, 1, sum(k=0, n-1, (-n)^(n-1-k) * (k+2)^k * binomial(n-1, k)));
for(n=0, 20, print1(a340473(n), ", "));

\\ a(0) = 1; a(n) = Sum_{k=0..n-1} n^(n-1-k) * (k+2)^k * binomial(n-1,k).
a227176(n) = if(n==0, 1, sum(k=0, n-1, n^(n-1-k) * (k+2)^k * binomial(n-1, k)));
for(n=0, 20, print1(a227176(n), ", "));

print("-------");

\\ a(0) = 1; a(n) = Sum_{i,j,k >= 0 and i+j+k=n-1} ((n-1)!/(i!*j!*k!)) * (-n)^i * (n-i)^j * (k+2)^k.
a394239(n) = {
  if(n==0, 1,
    sum(i=0, n-1,
      sum(j=0, n-1-i,
        my(k=n-1-i-j);
        ((n-1)!/(i!*j!*k!)) * (-n)^i * (n-i)^j * (k+2)^k
      )
    )
  )
};
for(n=0, 19, print1(a394239(n), ", "));

\\ a(0) = 1; a(n) = Sum_{i,j,k >= 0 and i+j+k=n-1} ((n-1)!/(i!*j!*k!)) * n^i * (n-i)^j * (-k)^k.
a395376(n) = {
  if(n==0, 1,
    sum(i=0, n-1,
      sum(j=0, n-1-i,
        my(k=n-1-i-j);
        ((n-1)!/(i!*j!*k!)) * n^i * (n-i)^j * (-k)^k
      )
    )
  )
};
for(n=0, 18, print1(a395376(n), ", "));

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
for(n=0, 18, print1(a268653(n), ", "));

print("-------");

\\ a(0) = 1; a(n) = Sum_{i,j,k,l >= 0 and i+j+k+l=n-1} ((n-1)!/(i!*j!*k!*l!)) * n^i * (n-i)^j * (n-i-j)^k * (l+2)^l.
a268654(n) = {
  if(n==0, 1,
    sum(i=0, n-1,
      sum(j=0, n-1-i,
        sum(k=0, n-1-i-j,
          my(l=n-1-i-j-k);
          ((n-1)!/(i!*j!*k!*l!)) * n^i * (n-i)^j * (n-i-j)^k * (l+2)^l
        )
      )
    )
  )
};
for(n=0, 18, print1(a268654(n), ", "));

\\ a(0) = 1; a(n) = Sum_{k=0..n-1} binomial(n-1,k) * n^k * A268653(n-k).
a(n) = if(n==0, 1, sum(k=0, n-1, binomial(n-1, k) * n^k * a268653(n-k)));
for(n=0, 30, print1(a(n)-a268654(n), ", "));
