\\ a(n) = Sum_{i,j,k,l >= 0 and i+j+k+l=n-1} ((n-1)!/(i!*j!*k!*l!)) * n^i * (-n)^j * (-j)^k * (l+1)^l. 
a396556(n) = {
  sum(i=0, n-1,
    sum(j=0, n-1-i,
      sum(k=0, n-1-i-j,
      my(l = n-1-i-j-k);
      ((n-1)!/(i!*j!*k!*l!)) * n^i * (-n)^j * (-j)^k * (l+1)^l
      
      )
    )
  )
};
for(n=0, 15, print1(a396556(n), ", "));

\\ a(n) = Sum_{i,j,k,l >= 0 and i+j+k+l=n-1} ((n-1)!/(i!*j!*k!*l!)) * (-n)^i * n^j * j^k * (l+1)^l. 
a396557(n) = {
  sum(i=0, n-1,
    sum(j=0, n-1-i,
      sum(k=0, n-1-i-j,
      my(l = n-1-i-j-k);
      ((n-1)!/(i!*j!*k!*l!)) * (-n)^i * n^j * j^k * (l+1)^l
      
      )
    )
  )
};
for(n=0, 15, print1(a396557(n), ", "));

\\ a(n) = Sum_{i,j,k,l >= 0 and i+j+k+l=n-1} ((n-1)!/(i!*j!*k!*l!)) * n^(i+j) * (-j)^k * (l+1)^l.
a227278(n) = {
  sum(i=0, n-1,
    sum(j=0, n-1-i,
      sum(k=0, n-1-i-j,
      my(l = n-1-i-j-k);
      ((n-1)!/(i!*j!*k!*l!)) * n^(i+j) * (-j)^k * (l+1)^l
      
      )
    )
  )
};
for(n=0, 15, print1(a227278(n), ", "));
