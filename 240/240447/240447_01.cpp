#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include <algorithm>
#include <chrono>
#include <iostream>
using namespace std;

static const uint32_t BASE = 1000000000U;

struct Big {
  vector<uint32_t> d; // little-endian base 1e9
  void clear(){ d.clear(); }
  bool empty() const { return d.empty(); }
};

static inline void add_big(Big &dst, const Big &src){
  if (src.d.empty()) return;
  size_t n = max(dst.d.size(), src.d.size());
  if (dst.d.size() < n) dst.d.resize(n, 0);
  uint64_t carry = 0;
  for (size_t i = 0; i < n; ++i){
    uint64_t a = dst.d[i];
    uint64_t b = (i < src.d.size()) ? src.d[i] : 0;
    uint64_t s = a + b + carry;
    dst.d[i] = (uint32_t)(s % BASE);
    carry = s / BASE;
  }
  if (carry) dst.d.push_back((uint32_t)carry);
}

static inline void set_ui(Big &dst, uint32_t v){
  dst.d.clear();
  if (v) dst.d.push_back(v);
}

static inline string to_string_big(const Big &b){
  if (b.d.empty()) return "0";
  string s = std::to_string(b.d.back());
  char buf[16];
  for (int i = (int)b.d.size() - 2; i >= 0; --i){
    snprintf(buf, sizeof(buf), "%09u", b.d[i]);
    s += buf;
  }
  return s;
}

struct Row {
  vector<int> idx;   // s2 list
  vector<Big> val;   // values aligned to idx
  vector<int> pos;   // pos[s2] = index+1 or 0

  Row() {}
  Row(int n_max){ init(n_max); }

  void init(int n_max){
    idx.clear();
    val.clear();
    pos.assign(n_max + 1, 0);
  }

  void clear(){
    for (int s2 : idx){
      pos[s2] = 0;
    }
    idx.clear();
    val.clear();
  }

  void add(int s2, const Big &count){
    int p = pos[s2];
    if (p == 0){
      pos[s2] = (int)idx.size() + 1;
      idx.push_back(s2);
      val.emplace_back();
      add_big(val.back(), count);
    } else {
      add_big(val[p - 1], count);
    }
  }

  size_t size() const { return idx.size(); }
};

vector<string> all_a(int n_max, int log_step, FILE *fp_out){
  int size = n_max + 1;
  vector<Row> dp(size), ndp(size);
  for (int i = 0; i <= n_max; ++i){
    dp[i].init(n_max);
    ndp[i].init(n_max);
  }

  // dp[0][0] = 1
  dp[0].idx.push_back(0);
  dp[0].val.emplace_back();
  set_ui(dp[0].val[0], 1);
  dp[0].pos[0] = 1;

  vector<int> cb;
  cb.reserve(n_max);

  auto t0 = chrono::steady_clock::now();
  auto t_prev = t0;

  // n = 0 is already finalized
  if (fp_out) fprintf(fp_out, "0 1\n");

  for (int p = 1; p <= n_max; ++p){
    // build contributions for multiplicity >= 2: cb list of m*p
    cb.clear();
    for (int m = 2, v = 2 * p; v <= n_max; ++m, v += p) cb.push_back(v);

    // clear ndp rows (only those used in dp)
    for (int i = 0; i <= n_max; ++i){
      if (!ndp[i].idx.empty()) ndp[i].clear();
    }

    int max_s1 = p * (p + 1) / 2;
    if (max_s1 > n_max) max_s1 = n_max;

    for (int s1 = 0; s1 <= max_s1; ++s1){
      Row &r = dp[s1];
      if (r.idx.empty()) continue;
      for (size_t k = 0; k < r.idx.size(); ++k){
        int s2 = r.idx[k];
        const Big &count = r.val[k];

        // multiplicity 0
        ndp[s1].add(s2, count);

        // multiplicity 1
        if (s1 + p <= n_max) ndp[s1 + p].add(s2, count);

        // multiplicity >= 2
        for (size_t i = 0; i < cb.size(); ++i){
          int ns2 = s2 + cb[i];
          if (ns2 <= n_max) ndp[s1].add(ns2, count);
          else break;
        }
      }
    }

    dp.swap(ndp);

    // a(p) is finalized after processing part p
    if (fp_out){
      Row &r = dp[p];
      int pos = r.pos[p];
      if (pos == 0) fprintf(fp_out, "%d 0\n", p);
      else {
        string s = to_string_big(r.val[pos - 1]);
        fprintf(fp_out, "%d %s\n", p, s.c_str());
      }
    }

    if (log_step > 0 && p % log_step == 0){
      long long cells = 0;
      for (int i = 0; i <= n_max; ++i) cells += (long long)dp[i].size();
      auto t_now = chrono::steady_clock::now();
      double step_s = chrono::duration<double>(t_now - t_prev).count();
      double total_s = chrono::duration<double>(t_now - t0).count();
      t_prev = t_now;
      cerr << "p=" << p << " cells=" << cells << " step_s=" << step_s << " total_s=" << total_s << "\n";
    }
  }

  vector<string> res(size);
  for (int n = 0; n <= n_max; ++n){
    Row &r = dp[n];
    int p = r.pos[n];
    if (p == 0) res[n] = "0";
    else res[n] = to_string_big(r.val[p - 1]);
  }
  return res;
}

int main(int argc, char **argv){
  int n_max = 50;
  const char *out_path = "b240447.txt";
  int log_step = 0;
  if (argc >= 2) n_max = atoi(argv[1]);
  if (argc >= 3) out_path = argv[2];
  if (argc >= 4) log_step = atoi(argv[3]);

  FILE *fp = fopen(out_path, "w");
  if (!fp){
    fprintf(stderr, "failed to open %s\n", out_path);
    return 1;
  }
  setvbuf(fp, NULL, _IOLBF, 0);

  auto a = all_a(n_max, log_step, fp);
  // (a is returned in case you want to use it in-process)
  fclose(fp);
  return 0;
}
