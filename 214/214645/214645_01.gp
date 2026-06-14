\\ E.g.f. A'(x) = exp(A^l(x)), A(0) = 0.
\\ A^l(x) denotes the l-th iterate of A.

iter(F, k, N) = {
  my(y = 'x + O('x^(N + 1)));
  if(k == 0, return(y));
  for(i = 1, k, y = subst(F, 'x, y));
  y
};

Aseries(N, l = 3) = {
  my(A = 'x + O('x^(N + 1)));
  for(i = 1, N, A = intformal(exp(iter(A, l, N))));
  A
};

Akseries(N, k = 1, l = 3) = {
  iter(Aseries(N, l), k, N)
};

Bkseries(N, k = 1, l = 3) = {
  exp(Akseries(N, k, l)) + O('x^(N + 1))
};

a_vector(N, k = 1, l = 3) = {
  my(Ak = Akseries(N, k, l));
  vector(N, n, n! * polcoef(Ak, n))
};

b_vector(N, k = 1, l = 3) = {
  my(Bk = Bkseries(N, k, l));
  vector(N + 1, n, (n - 1)! * polcoef(Bk, n - 1))
};




print(a_vector(35, 1, 2));
print(b_vector(35, 1, 2));

\\ print(a_vector(15, 2, 2));
\\ print(b_vector(15, 2, 2));


print("================================");


\\ print(a_vector(35, 1, 3));
\\ print(b_vector(35, 1, 3));

\\ print(a_vector(15, 2, 3));
\\ print(b_vector(15, 2, 3));

