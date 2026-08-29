// Beam Search Library

const bool MAXIMIZE = true;
// true: 大きい方を残す
// false: 小さい方を残す
const int BEAM_WIDTH = 100;  // ビーム幅（適宜調整してください）

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
