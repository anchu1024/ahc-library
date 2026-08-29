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

    Timer() { this->init(); }

    void init() { start = chrono::steady_clock::now(); }

    ll time() {
        auto current = chrono::steady_clock::now();
        return chrono::duration_cast<chrono::milliseconds>(current - start)
            .count();
    }
};
Timer timer;

// ここにグローバル変数を置く (例: const int N;)

const bool MAXIMIZE = true;
// true: 大きい方を残す
// false: 小さい方を残す

struct State {
    ll score;
    bool isFinish;
    // ここに必要な要素を入れて
};

vector<State> get_candidate(const State& state) {
    vector<State> res;
    // ここで次の候補のStateを作成して返す。
}

ll calculate_score(const State& state) {
    // ここでスコアを計算してください
}

State greedy(State& begin) {
    State best_state = begin;
    do {
        if (timer.time() >= 1950) return best_state;
        vector<State> next = get_candidate(best_state);
        if (next.empty()) {
            return best_state;
        }
        for (State& state : next) {
            state.score = calculate_score(state);
        }
        sort(next.begin(), next.end(), [&](State& a, State& b) {
            if (MAXIMIZE)
                return a.score > b.score;
            else
                return a.score < b.score;
        });
        best_state = next[0];
    } while (!best_state.isFinish);
    return best_state;
}

void input() {
    // ここで入力を読み込みましょう。
}

void output(State& ans) {
    // ここで出力する。
}

State init() {
    State s;
    s.score = 0;
    s.isFinish = false;
    return s;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    timer.init();

    input();

    State begin = init();

    // ここでGreedyやらなんやらする
    State ans = greedy(begin);

    output(ans);

    return 0;
}
