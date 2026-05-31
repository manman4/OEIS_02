\\ a(n) = Sum_{i,j,k >= 0 and i+j+k=n-1} ((n-1)!/(i!*j!*k!)) * n^i * (n-i)^j * (k+1)^k.
a227278(n) = {
  sum(i=0, n-1,
    sum(j=0, n-1-i,
      my(k=n-1-i-j);
      ((n-1)!/(i!*j!*k!)) * n^i * (n-i)^j * (k+1)^k
    )
  )
};
for(n=0, 20, print1(a227278(n), ", "));

\\ a(n) = Sum_{i,j,k >= 0 and i+j+k=n-1} ((n-1)!/(i!*j!*k!)) * (-n)^i * (n-i)^j * (k+1)^k. 
a396556(n) = {
  sum(i=0, n-1,
    sum(j=0, n-1-i,
      my(k=n-1-i-j);
      ((n-1)!/(i!*j!*k!)) * (-n)^i * (n-i)^j * (k+1)^k
    )
  )
};
for(n=0, 20, print1(a396556(n), ", "));

\\ a(n) = Sum_{i,j,k >= 0 and i+j+k=n-1} ((n-1)!/(i!*j!*k!)) * n^i * (-(n-i))^j * (k+1)^k. 
a396557(n) = {
  sum(i=0, n-1,
    sum(j=0, n-1-i,
      my(k=n-1-i-j);
      ((n-1)!/(i!*j!*k!)) * n^i * (-(n-i))^j * (k+1)^k
    )
  )
};
for(n=0, 20, print1(a396557(n), ", "));

\\ a(n) = Sum_{i,j,k >= 0 and i+j+k=n-1} ((n-1)!/(i!*j!*k!)) * n^i * (n-i)^j * (-(k+1))^k. 
a396558(n) = {
  sum(i=0, n-1,
    sum(j=0, n-1-i,
      my(k=n-1-i-j);
      ((n-1)!/(i!*j!*k!)) * n^i * (n-i)^j * (-(k+1))^k
    )
  )
};
for(n=0, 20, print1(a396558(n), ", "));
