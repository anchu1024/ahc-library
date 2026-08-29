// Greedy Library

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
