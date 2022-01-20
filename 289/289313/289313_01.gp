default(parisize, 120000000)

\\ \r
\\ Nは使わない 

v(n)={x='x+O('x^(n+10)); sum(k=0, n, prod(j=1, k, 1-((1-x)/(1+x))^(2*j-1)))};
M=200;
v=v(M);
for(n=0, M, write("/Users/xxx/Desktop/b289313_1.txt", n, " ", polcoef(v, n)))