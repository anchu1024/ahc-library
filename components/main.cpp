// 入力変数はグローバル化しておきましょう

void input() {
    // ここで入力を読み込みましょう。
}

void output(State& ans) {
    // ここで出力する。
}

State init() {
    // ここで初期ステートを生成する
}

// ここにいろいろなコードを挟む Greedyなど

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    Timer timer;  // ここでタイマーを初期化

    input();

    State begin = init();

    // ここでGreedyやらなんやらする
    State ans = greedy(begin);

    output(ans);

    return 0;
}
