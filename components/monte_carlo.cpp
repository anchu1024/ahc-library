// Monte Carlo Library

const bool MAXIMIZE = true;
// true: 大きい方を残す
// false: 小さい方を残す

// 1候補あたりの基本プレイアウト数 (実行時間制約に合わせて調整してください)
const int PLAYOUT_COUNT = 30;

struct State {
    ll score;
    bool isFinish;
    // ここに必要な要素を入れて
};

vector<State> get_candidate(const State& state) {
    vector<State> res;
    // ここで次の候補のStateを作成して返す。
    return res;
}

ll calculate_score(const State& state) {
    // ここで終局時のスコアを計算してください
    return 0;
}

// プレイアウト用にランダムな次の状態を1つ返す関数
State get_random_next(const State& state) {
    vector<State> next_states = get_candidate(state);
    if (next_states.empty()) {
        State end_state = state;
        end_state.isFinish = true;
        return end_state;
    }
    // 乱数生成（より高速なXorShift等への置き換えを推奨します）
    static std::mt19937 mt(1333);
    int idx = mt() % next_states.size();
    return next_states[idx];
}

// 終局までランダムに手を選び続け、最終スコアを返す関数
ll playout(State state) {
    while (!state.isFinish) {
        state = get_random_next(state);
    }
    return calculate_score(state);
}

State monte_carlo(State& begin) {
    State current_state = begin;

    do {
        if (timer.time() >= 1950) return current_state;

        vector<State> next = get_candidate(current_state);
        if (next.empty()) {
            return current_state;
        }

        double best_eval = MAXIMIZE ? -1e18 : 1e18;
        State best_state = next[0];

        for (State& state : next) {
            ll sum_score = 0;
            int actual_playouts = 0;

            // 各候補に対して複数回プレイアウトを実行
            for (int i = 0; i < PLAYOUT_COUNT; ++i) {
                if (timer.time() >= 1950) break;  // 時間切れ対策
                sum_score += playout(state);
                actual_playouts++;
            }

            if (actual_playouts == 0) break;

            // プレイアウトの平均スコアを算出
            double avg_score = (double)sum_score / actual_playouts;

            // 評価値が良ければベスト状態を更新
            if (MAXIMIZE) {
                if (avg_score > best_eval) {
                    best_eval = avg_score;
                    best_state = state;
                }
            } else {
                if (avg_score < best_eval) {
                    best_eval = avg_score;
                    best_state = state;
                }
            }
        }

        current_state = best_state;

    } while (!current_state.isFinish);

    return current_state;
}
