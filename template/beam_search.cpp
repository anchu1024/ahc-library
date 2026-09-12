// greedyのテンプレートの中で定数部分にBEAM_WIDTHを追加して、greedy関数をbeam_search関数に置き換えるだけでOK

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

const int BEAM_WIDTH = 100;

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

State beam_search(State& begin) {
    vector<State> current_states;
    current_states.push_back(begin);

    State best_state = begin;

    while (true) {
        // タイムリミットチェック
        if (timer.time() >= 1950) return best_state;

        vector<State> next_states;
        bool all_finished = true;

        // 現在保持している上位の状態すべてから、次の候補を展開する
        for (const State& state : current_states) {
            // すでに終了条件を満たしている状態はそのまま次ターンへ持ち越す
            if (state.isFinish) {
                next_states.push_back(state);
                continue;
            }

            all_finished = false;

            vector<State> candidates = get_candidate(state);
            for (State& next_state : candidates) {
                next_state.score = calculate_score(next_state);
                next_states.push_back(next_state);
            }
        }

        // 全ての状態が終了した、もしくは次の候補が存在しない場合は終了
        if (next_states.empty() || all_finished) {
            break;
        }

        // 次の候補をすべてスコア順にソートする
        sort(next_states.begin(), next_states.end(),
             [&](const State& a, const State& b) {
                 if (MAXIMIZE)
                     return a.score > b.score;
                 else
                     return a.score < b.score;
             });

        // ビーム幅 (BEAM_WIDTH) の分だけ上位を残す
        current_states.clear();
        int next_size = min((int)next_states.size(), BEAM_WIDTH);
        for (int i = 0; i < next_size; ++i) {
            current_states.push_back(next_states[i]);
        }

        // 暫定1位を記録しておく（一番スコアが良いものが先頭に来ている）
        best_state = current_states[0];
    }

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
    State ans = beam_search(begin);

    output(ans);

    return 0;
}
