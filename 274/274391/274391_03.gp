/*
  Square array A(n, k), n >= 0, k >= 0, read by antidiagonals upward.

  Original form:
    A(n, 0) = 1;
    A(n, k) = Sum_{x_1,...,x_{n+1} >= 0 and x_1+...+x_{n+1} = k-1}
                multinomial(k-1; x_1, x_2, ..., x_{n+1})
                * Product_{i=1..n} (k - Sum_{j=1..i-1} x_j)^(x_i).

  Reduced form for n > 0 and k > 0:
    A(0, k) = A(n, 0) = 1;
    A(n, k) = Sum_{x_1,...,x_n >= 0 and x_1+...+x_n = k-1}
                multinomial(k-1; x_1, ..., x_n)
                * (x_n + 2)^(x_n)
                * Product_{i=1..n-1} (k - Sum_{j=1..i-1} x_j)^(x_i).

  This is obtained from the (n+1)-variable form by combining the last two
  variables with the binomial theorem:

    sum_{a+b=t} binomial(t, a) * (t+1)^a = (t+2)^t.

  So the last pair (x_n, x_{n+1}) in the original sum collapses to the
  single factor (x_n + 2)^(x_n).

  This file verifies that the two formulas agree.
*/

default_n = 10;
default_k = 10;

ffact(x) = x!;

/*
  Full formula:
    A(n, k) = (k-1)! * Sum over compositions of k-1 into n+1 parts.
*/
full_weight(n, k, pos, prefix, x) = if(pos <= n, (k - prefix)^x, 1);

full_rec(n, k, pos, remaining, prefix, coeff) = {
  my(x, sum = 0);

  if(pos == n + 2,
    return(if(remaining == 0, coeff, 0));
  );

  for(x = 0, remaining,
    sum += full_rec(n, k, pos + 1, remaining - x, prefix + x,
      coeff / ffact(x) * full_weight(n, k, pos, prefix, x)
    )
  );

  sum
};

a_full(n, k) = {
  if(n < 0 || k < 0,
    return(0);
  );
  if(k == 0,
    return(1);
  );

  ffact(k - 1) * full_rec(n, k, 1, k - 1, 0, 1)
};

/*
  Reduced formula:
    A(n, k) = (k-1)! * Sum over compositions of k-1 into n parts.
*/
reduced_weight(n, k, pos, prefix, x) = if(pos < n, (k - prefix)^x, (x + 2)^x);

reduced_rec(n, k, pos, remaining, prefix, coeff) = {
  my(x, sum = 0);

  if(pos == n + 1,
    return(if(remaining == 0, coeff, 0));
  );

  for(x = 0, remaining,
    sum += reduced_rec(n, k, pos + 1, remaining - x, prefix + x,
      coeff / ffact(x) * reduced_weight(n, k, pos, prefix, x)
    )
  );

  sum
};

a_reduced(n, k) = {
  if(n < 0 || k < 0,
    return(0);
  );
  if(k == 0 || n == 0,
    return(1);
  );

  ffact(k - 1) * reduced_rec(n, k, 1, k - 1, 0, 1)
};

validate_formulas(max_n, max_k) = {
  my(n, k, lhs, rhs);

  for(n = 0, max_n,
    for(k = 0, max_k,
      if(a_full(n, k) != a_reduced(n, k),
        error(Str("mismatch at n=", n, ", k=", k, ": full=", a_full(n, k), ", reduced=", a_reduced(n, k)))
      )
    )
  );

  print(Str("Validated all cases for 0 <= n <= ", max_n, ", 0 <= k <= ", max_k, "."));
  1
};

validate_formulas(default_n, default_k);

A(n, k) = a_reduced(n, k);

for(n = 0, 10, for(k = 0, n, print1(A(n - k, k), ", ")));
