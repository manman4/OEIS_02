/*
  Square array A(n, k), n >= 0, k >= 0, read by antidiagonals downward.

  A(n, k) = n! * [x^n] exp(-W_k(-x)),
  where W_k(x) is the k-th iterate of LambertW(x).

  The 0-th iterate is the identity:
    W_0(x) = x.
*/

default_n = 15;
default_k = 3;

/*
  Return A(n, k) = n! * [x^n] exp(-W_k(-x)).
  The series is truncated to precision O(x^(n+2)).
*/
a(n, k) = {
  my(x_series, T_series, iterated, egf);

  if(n < 0,
    return(0);
  );

  x_series = 'x + O('x^(n + 2));
  T_series = -lambertw(-x_series);
  iterated = x_series;

  for(i = 1, k,
    iterated = subst(T_series, 'x, iterated);
  );

  egf = exp(iterated);
  n! * polcoef(egf, n)
};

b(k, n) = a(n, k);

matrix(7, 7, n, k, b(n - 1, k - 1))
\\ upward 
for(n=0, 10, for(k=0, n, print1(b(n-k, k),", ")));

for(n=0, 20, print1(b(1, n), ", "));
for(n=0, 20, print1(b(2, n), ", "));
for(n=0, 20, print1(b(3, n), ", "));
for(n=0, 20, print1(b(4, n), ", "));
