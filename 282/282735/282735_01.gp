a(n, k=3) = sum(i=0, n*k-2, binomial(n*k-4, i)*sum(j=0, (n*k-i-2)\(k+1), (-1)^j*binomial(n, j)*binomial(n*(k+1)-i-4-(k+1)*j, n-2)));
for(n=2, 30, print1(a(n), ", "));
\\ for(n=2, 101, write("b282735_1.txt", n, " ", a(n)));

\\ a(n) = Sum_{i=0..3*n-2} binomial(3*n-4,i) * Sum_{j=0..floor((3*n-i-2)/4)} (-1)^j * binomial(n,j) * binomial(4*n-i-4-4*j,n-2).
b(n) = sum(i=0, n*3-2, binomial(n*3-4, i)*sum(j=0, (n*3-i-2)\4, (-1)^j*binomial(n, j)*binomial(n*4-i-4-4*j, n-2)));
for(n=0, 30, print1(a(n)-b(n), ", "));