a(n, k=3) = sum(i=0, n*k-2, binomial(n*k-4, i)*sum(j=0, (n*k-i-2)\(k+1), (-1)^j*binomial(n, j)*binomial(n*(k+1)-i-4-(k+1)*j, n-2)));
for(n=0, 101, write("b282735_1.txt", n, " ", a(n)));

