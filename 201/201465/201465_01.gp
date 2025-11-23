M=20;

\\ O.g.f.: x * Sum_{n>=0} 2^n / Product_{k=0..n} (3 - k*x).

my(N=15, x='x+O('x^N)); Vec( x * sum(k=0, 700, 2^k / prod(j=0, k, (3. - j*x))) )
my(N=15, x='x+O('x^N)); apply(round, Vec( x * sum(k=0, 700, 2^k / prod(j=0, k, (3. - j*x))) ))
