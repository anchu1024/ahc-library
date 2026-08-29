// Simulated Annealing Library

const bool MAXIMIZE = true;
// true: 大きい方を残す
// false: 小さい方を残す

struct State {
    ll score;
    // ここに必要な要素を入れて
};

// 0.0 ~ 1.0の乱数を生成する関数 (XorShift等の高速な乱数生成器の使用を推奨)
double rand01() {
    static mt19937 mt(893);  // <random> ヘッダが必要
    uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(mt);
}

State get_next_state(const State& state) {
    State next = state;
    // ここで状態をランダムに「少しだけ」変化させる（近傍遷移）
    // 例: next.array[rand() % N] = rand_val;
    return next;
}

ll calculate_score(const State& state) {
    // ここでスコアを計算してください
    // （状態全体を再計算するより、差分だけ計算して更新する方が高速です）
}

State simulated_annealing(State& begin) {
    State current_state = begin;
    current_state.score = calculate_score(current_state);
    State best_state = current_state;

    // --- ハイパーパラメータ（問題に合わせて必ず調整してください） ---
    double start_temp = 1000.0;  // 序盤の温度（スコアの変動幅の目安）
    double end_temp = 10.0;      // 終盤の温度
    double time_limit = 1950.0;  // 制限時間(ms)
    // -------------------------------------------------------------

    int iter_count = 0;

    while (true) {
        // 毎回時間を取ると重いので、一定回数ごとに時間チェックするのもおすすめ
        double current_time = timer.time();
        if (current_time >= time_limit) break;

        // 1. 次の候補（近傍）を1つ生成
        State next_state = get_next_state(current_state);
        next_state.score = calculate_score(next_state);

        // 2. 現在の温度を計算（線形に下げる例）
        double temp =
            start_temp + (end_temp - start_temp) * (current_time / time_limit);

        // 3. スコアの差分を計算
        ll score_diff = next_state.score - current_state.score;
        if (!MAXIMIZE)
            score_diff = -score_diff;  // 最小化問題の場合は符号を反転

        // 4. 遷移確率の計算と受理判定
        // スコアが良くなった場合は確実（prob > 1.0）に遷移する
        // 悪くなった場合でも、確率的に遷移する（局所解からの脱出）
        double prob = exp(score_diff / temp);

        if (prob > rand01()) {
            current_state = next_state;

            // ベストスコアの更新判定
            bool is_best_updated =
                MAXIMIZE ? (current_state.score > best_state.score)
                         : (current_state.score < best_state.score);
            if (is_best_updated) {
                best_state = current_state;
            }
        }

        iter_count++;
    }

    // cerr << "Iter: " << iter_count << endl; // 実行回数の確認用
    return best_state;
}
