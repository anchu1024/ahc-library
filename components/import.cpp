#pragma GCC optimize("O3")
#pragma GCC optimize("unroll-loops")
#pragma GCC target("avx2,bmi,bmi2,lzcnt,popcnt")

#include <bits/stdc++.h>
using namespace std;
using ll = long long;
using ull = unsigned long long;
using pii = pair<int, int>;
using pll = pair<ll, ll>;
using vi = vector<int>;
using vll = vector<ll>;
using vvi = vector<vi>;
using vvll = vector<vll>;

constexpr int MOD = 998244353;
const int MOD2 = 1000000007;
constexpr ll INF = 1e18;
constexpr ll dx[4] = {1, 0, -1, 0};
constexpr ll dy[4] = {0, 1, 0, -1};
// 斜め移動を許可されている場合
// constexpr ll dx[8] = {1,1,0,-1,-1,-1,0,1};
// constexpr ll dy[8] = {0,1,1,1,0,-1,-1,-1};

// 時間管理
struct Timer {
    chrono::steady_clock::time_point start;

    Timer() { start = chrono::steady_clock::now(); }

    ll time() {
        auto current = chrono::steady_clock::now();
        return chrono::duration_cast<chrono::milliseconds>(current - start)
            .count();
    }
};
