// Fast Random Library

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
    return res;
}

ll calculate_score(const State& state) {
    // ここでスコアを計算してください
    return 0;  // 仮の戻り値
}

// 標準のrand()より高速な乱数生成器 (XorShift)
uint32_t xor_shift() {
    static uint32_t y = 2463534242;
    y = y ^ (y << 13);
    y = y ^ (y >> 17);
    return y = y ^ (y << 5);
}

State fast_random(const State& begin) {
    State overall_best_state = begin;
    // ベストスコアの初期化（最大化なら極小値、最小化なら極大値）
    overall_best_state.score = MAXIMIZE ? -2e18 : 2e18;

    // 制限時間いっぱいまでシミュレーション（プレイアウト）を繰り返す
    while (timer.time() < 1950) {
        State current_state = begin;

        // 1回のランダム探索を最後までやり切る
        while (!current_state.isFinish) {
            // ループ内でも時間チェック（重い処理の場合は必須）
            if (timer.time() >= 1950) return overall_best_state;

            vector<State> next_states = get_candidate(current_state);
            if (next_states.empty()) {
                break;
            }

            // 候補の中から完全にランダムで1つ選んで遷移する
            int r = xor_shift() % next_states.size();
            current_state = next_states[r];
        }

        // 終端まで行った（または候補が無くなった）状態のスコアを計算
        current_state.score = calculate_score(current_state);

        // ベストスコアの更新
        if (MAXIMIZE) {
            if (current_state.score > overall_best_state.score) {
                overall_best_state = current_state;
            }
        } else {
            if (current_state.score < overall_best_state.score) {
                overall_best_state = current_state;
            }
        }
    }

    return overall_best_state;
}
