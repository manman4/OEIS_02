\\ \r
\\ Nは使わない 
{a(n) = my(t='t); polcoef(n!*prod(j=1, n, (1-x^j+x*O(x^n))^(t-1)), n)};
M=100;
cnt = 0;
for(n=0, M, v=a(n); for (k=0, n, write("C:\\Users\\xxx\\Desktop\\b234937.txt", cnt, " ", polcoef(v, k)); cnt++))
