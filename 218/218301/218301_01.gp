\\ E.g.f.: A(x) = 1/2 + 1/2 * exp( 3*x - 3*LambertW(-x * exp(x)) ).
my(x='x+O(x^20)); Vec( serlaplace(1/2 + 1/2 * exp( 3*x - 3*lambertw(-x * exp(x)) ) ))

