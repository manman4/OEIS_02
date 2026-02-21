use std::cmp::min;
use std::time::Instant;

const MAX_LIMIT: usize = 30_000_000;

fn sieve_mobius(n: usize) -> Vec<i8> {
    let mut mu = vec![0_i8; n + 1];
    let mut primes: Vec<usize> = Vec::with_capacity(n / 10);
    let mut is_not_prime = vec![0_u8; n + 1];

    mu[1] = 1;
    for i in 2..=n {
        if is_not_prime[i] == 0 {
            primes.push(i);
            mu[i] = -1;
        }
        for &p in &primes {
            let target = i * p;
            if target > n {
                break;
            }
            is_not_prime[target] = 1;
            if i % p == 0 {
                mu[target] = 0;
                break;
            }
            mu[target] = -mu[i];
        }
    }

    mu
}

fn isqrt_u128(x: u128) -> u128 {
    if x <= 1 {
        return x;
    }
    let mut left = 1_u128;
    let mut right = x;
    let mut ans = 1_u128;

    while left <= right {
        let mid = left + (right - left) / 2;
        if mid <= x / mid {
            ans = mid;
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    ans
}

fn range_sum_floor(v: i64, low: i64, mut high: i64) -> i64 {
    if low > high || low > v {
        return 0;
    }
    if high > v {
        high = v;
    }

    let mut sum = 0_i64;
    let mut i = low;
    while i <= high {
        let val = v / i;
        if val == 0 {
            break;
        }
        let mut next_i = v / val;
        if next_i > high {
            next_i = high;
        }
        sum += (next_i - i + 1) * val;
        i = next_i + 1;
    }

    sum
}

fn solve(n: i64, mu: &[i8]) -> i64 {
    let mut total_sum = 0_i64;
    let base = (n as u128) / 12;
    let mut limit = isqrt_u128(base) as usize;
    limit = min(limit, MAX_LIMIT);

    let mut d = 1_usize;
    while d <= limit {
        if mu[d] != 0 {
            let d_i64 = d as i64;
            let k = n / (2_i64 * d_i64 * d_i64);
            let mut f_k = 0_i64;

            let mut s = 3_i64;
            while (s as i128) * (s as i128) <= 2_i128 * (k as i128) {
                let v = k / s;
                let m_start = s / 2 + 1;
                let m_end = s - 1;
                f_k += range_sum_floor(v, m_start, m_end);
                s += 2;
            }

            if mu[d] == 1 {
                total_sum += f_k;
            } else {
                total_sum -= f_k;
            }
        }
        d += 2;
    }

    total_sum
}

fn main() {
    let mu = sieve_mobius(MAX_LIMIT);

    println!(" i | N = 10^i   | Result               | Time   ");
    println!("---+------------+----------------------+----------");

    for i in 1..=16 {
        let n = 10_i64.pow(i as u32);
        let start = Instant::now();
        let res = solve(n, &mu);
        let elapsed = start.elapsed().as_secs_f64();

        println!(
            "{:>2} | 10^{:<7} | {:>20} | {:>8.3} s",
            i, i, res, elapsed
        );
    }
}
