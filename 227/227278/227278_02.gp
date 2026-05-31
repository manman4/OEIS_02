\\ a(n) = Sum_{i,j,k >= 0 and i+j+k=n-1} ((n-1)!/(i!*j!*k!)) * n^i * (n-i)^j * (k+1)^k.
a227278(n) = {
  sum(i=0, n-1,
    sum(j=0, n-1-i,
      my(k=n-1-i-j);
      ((n-1)!/(i!*j!*k!)) * n^i * (n-i)^j * (k+1)^k
    )
  )
};
for(n=0, 15, print1(a227278(n), ", "));
