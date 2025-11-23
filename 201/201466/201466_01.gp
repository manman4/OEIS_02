M=20;

\\ O.g.f.: x * Sum_{n>=0} 3^n / Product_{k=0..n} (4 - k*x).

my(N=15, x='x+O('x^N)); Vec( x * sum(k=0, 700, 3^k / prod(j=0, k, (4. - j*x))) )
my(N=15, x='x+O('x^N)); apply(round, Vec( x * sum(k=0, 700, 3^k / prod(j=0, k, (4. - j*x))) ))

