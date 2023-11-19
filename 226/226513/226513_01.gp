\\ S(n,k) = Sum_{i=0..k} S2_k(i)*i!*binomial(n+i,i).
S(n,k) = sum(i=0, k, stirling(k, i, 2)*i!*binomial(n+i,i));
for(n=0, 10, for(k=0, n, print1(S(n-k,k),", ")))

\\ T(n,0) = 1; T(n,k) = Sum_{j=1..k} (n*j/k + 1) * binomial(k,j) * T(n,k-j).
T(n,k) = if(k==0, 1, sum(j=1, k, (n*j/k + 1) * binomial(k,j) * T(n,k-j)));               
for(n=0, 10, for(k=0, n, print1(S(n-k,k)-T(n-k,k),", ")))

\\ T(n,0) = 1; T(n,k) = (n+1)*T(n,k-1) - 2*Sum_{j=1..k-1} (-1)^j * binomial(k-1,j) * T(n,k-j).
T(n,k) = if(k==0, 1, (n+1)*T(n,k-1) - 2*sum(j=1, k-1, (-1)^j * binomial(k-1,j) * T(n,k-j)));
for(n=0, 10, for(k=0, n, print1(S(n-k,k)-T(n-k,k),", ")))                                     