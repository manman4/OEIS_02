\\ E.g.f.: A(x) = 1/2 + 1/2 * exp( 3*x - 3/2 * LambertW(-2*x * exp(2*x)) ).
my(x='x+O(x^20)); Vec( serlaplace(1/2 + 1/2 * exp( 3*x - 3/2 * lambertw(-2*x * exp(2*x)) )))


