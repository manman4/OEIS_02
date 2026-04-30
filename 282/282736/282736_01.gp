a(n, k=4) = sum(i=0, n*k-2, binomial(n*k-4, i)*sum(j=0, (n*k-i-2)\(k+1), (-1)^j*binomial(n, j)*binomial(n*(k+1)-i-4-(k+1)*j, n-2)));
for(n=2, 30, print1(a(n), ", "));

\\ a(n) = Sum_{i=0..4*n-2} binomial(4*n-4,i) * Sum_{j=0..floor((4*n-i-2)/5)} (-1)^j * binomial(n,j) * binomial(5*n-i-4-5*j,n-2). 
b(n) = sum(i=0, 4*n-2, binomial(4*n-4, i)*sum(j=0, (4*n-i-2)\5, (-1)^j*binomial(n, j)*binomial(5*n-i-4-5*j, n-2)));
for(n=0, 30, print1(a(n)-b(n), ", "));