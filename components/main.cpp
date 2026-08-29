// 入力変数はグローバル化しておきましょう

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

// ここにいろいろな処理を入れる。

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
