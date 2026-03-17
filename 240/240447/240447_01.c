#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#define BASE 1000000000U

typedef struct {
  uint32_t *d;
  int len;
  int cap;
} Big;

static void bi_init(Big *b){ b->d = NULL; b->len = 0; b->cap = 0; }
static void bi_free(Big *b){ free(b->d); b->d = NULL; b->len = 0; b->cap = 0; }
static void bi_reserve(Big *b, int cap){
  if (cap <= b->cap) return;
  int ncap = b->cap == 0 ? 2 : b->cap;
  while (ncap < cap) ncap *= 2;
  b->d = (uint32_t*)realloc(b->d, (size_t)ncap * sizeof(uint32_t));
  b->cap = ncap;
}
static void bi_set_ui(Big *b, uint32_t v){
  if (v == 0){ b->len = 0; return; }
  bi_reserve(b, 1);
  b->d[0] = v;
  b->len = 1;
}
static void bi_add(Big *dst, const Big *src){
  if (src->len == 0) return;
  int max = dst->len > src->len ? dst->len : src->len;
  bi_reserve(dst, max + 1);
  uint64_t carry = 0;
  int i = 0;
  for (; i < max; i++){
    uint64_t a = (i < dst->len) ? dst->d[i] : 0;
    uint64_t b = (i < src->len) ? src->d[i] : 0;
    uint64_t s = a + b + carry;
    dst->d[i] = (uint32_t)(s % BASE);
    carry = s / BASE;
  }
  if (carry){
    dst->d[i++] = (uint32_t)carry;
  }
  dst->len = i;
}
static void bi_add_ui(Big *dst, uint32_t v){
  if (v == 0) return;
  bi_reserve(dst, dst->len + 1);
  uint64_t carry = v;
  int i = 0;
  while (carry && i < dst->len){
    uint64_t s = (uint64_t)dst->d[i] + carry;
    dst->d[i] = (uint32_t)(s % BASE);
    carry = s / BASE;
    i++;
  }
  if (carry){
    dst->d[dst->len++] = (uint32_t)carry;
  }
}
static void bi_to_str(const Big *b, char *buf, size_t buflen){
  if (b->len == 0){
    snprintf(buf, buflen, "0");
    return;
  }
  int i = b->len - 1;
  int n = snprintf(buf, buflen, "%u", b->d[i]);
  for (i = i - 1; i >= 0 && (size_t)n < buflen; i--){
    n += snprintf(buf + n, buflen - (size_t)n, "%09u", b->d[i]);
  }
}

typedef struct {
  int *idx;   // s2 indices
  Big *val;   // values
  int *pos;   // pos[s2] = index+1 or 0
  int len;
  int cap;
} Row;

static void row_init(Row *r, int n_max){
  r->idx = NULL; r->val = NULL; r->len = 0; r->cap = 0;
  r->pos = (int*)calloc((size_t)(n_max + 1), sizeof(int));
}
static void row_free(Row *r){
  if (r->val){
    for (int i = 0; i < r->len; i++) bi_free(&r->val[i]);
  }
  free(r->idx);
  free(r->val);
  free(r->pos);
  r->idx = NULL; r->val = NULL; r->pos = NULL; r->len = r->cap = 0;
}
static void row_reserve(Row *r, int cap){
  if (cap <= r->cap) return;
  int ncap = r->cap == 0 ? 4 : r->cap;
  while (ncap < cap) ncap *= 2;
  r->idx = (int*)realloc(r->idx, (size_t)ncap * sizeof(int));
  r->val = (Big*)realloc(r->val, (size_t)ncap * sizeof(Big));
  for (int i = r->cap; i < ncap; i++) bi_init(&r->val[i]);
  r->cap = ncap;
}
static void row_clear(Row *r){
  for (int i = 0; i < r->len; i++){
    int s2 = r->idx[i];
    r->pos[s2] = 0;
    r->val[i].len = 0;
  }
  r->len = 0;
}
static void row_add(Row *r, int s2, const Big *add){
  int p = r->pos[s2];
  if (p == 0){
    row_reserve(r, r->len + 1);
    int idx = r->len++;
    r->idx[idx] = s2;
    r->pos[s2] = idx + 1;
    bi_add(&r->val[idx], add);
  } else {
    bi_add(&r->val[p - 1], add);
  }
}

int main(int argc, char **argv){
  int n_max = 2000;
  const char *out_path = "b240447.txt";
  if (argc >= 2) n_max = atoi(argv[1]);
  if (argc >= 3) out_path = argv[2];

  Row *dp = (Row*)malloc((size_t)(n_max + 1) * sizeof(Row));
  Row *new_dp = (Row*)malloc((size_t)(n_max + 1) * sizeof(Row));
  for (int i = 0; i <= n_max; i++){
    row_init(&dp[i], n_max);
    row_init(&new_dp[i], n_max);
  }
  // dp[0][0] = 1
  row_reserve(&dp[0], 1);
  dp[0].idx[0] = 0;
  dp[0].pos[0] = 1;
  dp[0].len = 1;
  bi_set_ui(&dp[0].val[0], 1);

  int *cb = (int*)malloc((size_t)(n_max + 1) * sizeof(int));

  FILE *fp = fopen(out_path, "w");
  if (!fp){
    fprintf(stderr, "failed to open %s\n", out_path);
    return 1;
  }
  // Line-buffered so early results appear quickly.
  setvbuf(fp, NULL, _IOLBF, 0);
  char *buf = (char*)malloc(100000);

  // n = 0 is already finalized before any parts are processed.
  {
    Row *r0 = &dp[0];
    int pos0 = r0->pos[0];
    if (pos0 == 0){
      fprintf(fp, "0 0\n");
    } else {
      bi_to_str(&r0->val[pos0 - 1], buf, 100000);
      fprintf(fp, "0 %s\n", buf);
    }
  }

  int log_step = 100;
  if (argc >= 4) log_step = atoi(argv[3]);

  clock_t t0 = clock();
  clock_t t_prev = t0;
  for (int p = 1; p <= n_max; p++){
    // build contributions for multiplicity >= 2: cb list of m*p
    int cb_len = 0;
    int m = 2;
    while (m * p <= n_max){
      cb[cb_len++] = m * p;
      m++;
    }

    // clear new_dp rows used last time
    for (int i = 0; i <= n_max; i++){
      if (new_dp[i].len > 0) row_clear(&new_dp[i]);
    }

    int max_s1 = p * (p + 1) / 2;
    if (max_s1 > n_max) max_s1 = n_max;

    for (int s1 = 0; s1 <= max_s1; s1++){
      Row *r = &dp[s1];
      if (r->len == 0) continue;
      for (int k = 0; k < r->len; k++){
        int s2 = r->idx[k];
        Big *count = &r->val[k];

        // multiplicity 0
        row_add(&new_dp[s1], s2, count);

        // multiplicity 1
        if (s1 + p <= n_max) row_add(&new_dp[s1 + p], s2, count);

        // multiplicity >= 2
        for (int i = 0; i < cb_len; i++){
          int ns2 = s2 + cb[i];
          if (ns2 <= n_max) row_add(&new_dp[s1], ns2, count);
          else break;
        }
      }
    }

    Row *tmp = dp; dp = new_dp; new_dp = tmp;

    // After finishing part p, values for n = p are finalized.
    Row *r = &dp[p];
    int pos = r->pos[p];
    if (pos == 0){
      fprintf(fp, "%d 0\n", p);
    } else {
      bi_to_str(&r->val[pos - 1], buf, 100000);
      fprintf(fp, "%d %s\n", p, buf);
    }

    if (log_step > 0 && p % log_step == 0){
      long long cells = 0;
      for (int i = 0; i <= n_max; i++) cells += dp[i].len;
      clock_t t_now = clock();
      double total_s = (double)(t_now - t0) / CLOCKS_PER_SEC;
      double step_s = (double)(t_now - t_prev) / CLOCKS_PER_SEC;
      t_prev = t_now;
      fprintf(stderr, "p=%d cells=%lld step_s=%.2f total_s=%.2f\n", p, cells, step_s, total_s);
    }
  }
  free(buf);
  fclose(fp);

  free(cb);
  for (int i = 0; i <= n_max; i++){
    row_free(&dp[i]);
    row_free(&new_dp[i]);
  }
  free(dp);
  free(new_dp);
  return 0;
}
