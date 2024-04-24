\\ E.g.f.: A(x) = 1/2 + 1/2 * exp( 3*x - 3/4 * LambertW(-4*x * exp(4*x)) ).
my(x='x+O(x^20)); Vec( serlaplace(1/2 + 1/2 * exp( 3*x - 3/4 * lambertw(-4*x * exp(4*x)) ) ))


