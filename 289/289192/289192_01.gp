\\ A341014
\\ T(n, k) = Sum_{j=0..n} k^j    *j!*binomial(n, j)^2.
\\ A289192
\\ T(n, k) = Sum_{j=0..n} k^(n-j)*j!*binomial(n, j)^2.

T(n, k) = sum(j=0, n, k^(n-j)*j!*binomial(n, j)^2);
for(n=0, 9, for(k=0, n, print1(T(k, n-k), ", ")))