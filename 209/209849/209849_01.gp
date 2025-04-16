M=140;

T(n, k) = sum(j=k, n, 2^(n-j)*stirling(n, j, 2)*stirling(j, k, 1));

cnt=1;
for(n=1, M, for(k=1, n, write("/Users/xxx/Desktop/b209849_1.txt", cnt, " ", T(n, k)); cnt++));
