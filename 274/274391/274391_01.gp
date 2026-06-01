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


matrix(7, 7, n, k, a(n - 1, k - 1))
for(n=0, 10, for(k=0, n, print1(a(k, n-k),", ")));

for(n=0, 20, print1(a(n, 1), ", "));
for(n=0, 20, print1(a(n, 2), ", "));
for(n=0, 20, print1(a(n, 3), ", "));
for(n=0, 20, print1(a(n, 4), ", "));
