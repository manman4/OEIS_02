default(parisize, 120000000)

\\ \r
\\ Nは使わない 

v(n)={x='x+O('x^(n+10)); sum(k=1, (n+1)\3, x^(3*k-1)*prod(j=1, k-1, (1-x^(2*k+j-1))/(1-x^j)))};
M=1000;
v=v(M);
for(n=1, M, write("/Users/xxx/Desktop/b237753_1.txt", n, " ", polcoef(v, n))) 