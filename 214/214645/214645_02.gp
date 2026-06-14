\\ a(n,0,l) = 0^(n-1),
\\ a(n,k,l) = Sum_{j=0..n-1} binomial(n-1,j) * b(j,k+l-1,l) * a(n-j,k-1,l),
\\ b(0,k,l) = 1,
\\ b(n,k,l) = Sum_{j=1..n} binomial(n-1,j-1) * a(j,k,l) * b(n-j,k,l).

\\ E.g.f. A(x) satisfies A'(x) = exp(A^l(x)),
\\ where A^l(x) denotes the l-th iterate of A, with A(0) = 0.
\\ a(n,k,l) = n! * [x^n] A^k(x)
\\ b(n,k,l) = n! * [x^n] exp(A^k(x))

a(n, k, l, MA, MB) = {
  my(key = Str(n, ",", k), v);
  if(mapisdefined(MA, key, &v), return(v));
  if(k == 0, return(if(n == 1, 1, 0)));
  v = sum(j = 0, n - 1,
    binomial(n - 1, j) * b(j, k + l - 1, l, MA, MB) * a(n - j, k - 1, l, MA, MB)
  );
  mapput(MA, key, v);
  v
};

b(n, k, l, MA, MB) = {
  my(key = Str(n, ",", k), v);
  if(mapisdefined(MB, key, &v), return(v));
  if(n == 0, return(1));
  v = sum(j = 1, n,
    binomial(n - 1, j - 1) * a(j, k, l, MA, MB) * b(n - j, k, l, MA, MB)
  );
  mapput(MB, key, v);
  v
};

a_vector(N, k = 1, l = 3) = {
  my(MA = Map(), MB = Map());
  vector(N, n, a(n, k, l, MA, MB))
};

b_vector(N, k = 1, l = 3) = {
  my(MA = Map(), MB = Map());
  vector(N + 1, n, b(n - 1, k, l, MA, MB))
};

\\ print(a_vector(9, 1, 1));
\\ print(b_vector(9, 1, 1));

print(a_vector(9, 1, 2));
\\ print(b_vector(9, 1, 2));

\\ print(a_vector(9, 1, 3));
\\ print(b_vector(9, 1, 3));

