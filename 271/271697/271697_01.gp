M=139;

E(n, k) = sum(j=0, k, (-1)^j*binomial(n+1, j)*(k+1-j)^n);
a271697(n, k) = sum(j=0, n, (-1)^(n-j)*binomial(n, j)*E(j, k));
cnt=0;
for(n=0, M, for(k=0, n, write("b271697_1.txt", cnt, " ", a271697(n, k)); cnt++));