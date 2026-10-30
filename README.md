# ahc-library

AtCoder Heuristic Contest 向けの探索ライブラリです。C++ と Python の提出用テンプレート、
C++ 用コンポーネントを提供し、Python では共通の `Solver` から複数の探索方法を
利用できます。

## C++ の提出用ファイル

AHC の提出には [`submit.cpp`](./submit.cpp) を1ファイルで提出できます。C++20 以降を
想定し、特定問題の入力例・状態例・評価例は入れていません。ファイル冒頭の
「問題ごとの設定」と「問題ごとの実装」を問題に合わせて編集します。共通探索ライブラリは
編集不要です。

| 設定 | 内容 |
|---|---|
| `SEARCH_METHOD` | `"greedy"` / `"beam_search"` / `"simulated_annealing"` / `"fast_random"` / `"monte_carlo"` |
| `MAXIMIZE` | `true` は評価値最大化、`false` は最小化 |
| `RANDOM_SEED` | `nullopt` は未固定、整数を指定すると再現可能 |
| `SEARCH_STEPS` | Greedy・ビームサーチの最大手数 |
| `BEAM_WIDTH` | ビームサーチで各層に残す状態数 |
| `TIME_LIMIT_MS` | 時間制限付き探索の時間（ミリ秒） |
| `START_TEMPERATURE`, `END_TEMPERATURE` | 焼きなましの開始・終了温度 |
| `PLAYOUTS_PER_CANDIDATE` | モンテカルロで候補ごとに行うロールアウト数 |
| `LOG_LEVEL` | `none` / `error` / `warning` / `info` / `trace`。`none` はログを完全に無効化 |

`State` には問題に必要な型・フィールドを自由に定義できます。探索ライブラリは
状態の中身を参照せず、通常のコピーコンストラクタで状態を複製します。候補生成は探索方法に
よって使い分けます。

### Stateに入れるもの・避けるもの

盤面・位置・操作列など、**探索候補ごとに変化するデータ**を `State` に持たせます。
入力全体や大きな距離表など、全候補で共通のデータは `State` ごとに複製せず、
`ProblemData` と `problem_data` にまとめ、探索中は読み取り専用として扱ってください。
Stateのコピーが壊れるため、生ポインタや参照を安易に持たせないでください。共有データを
参照する必要がある場合は、寿命を保証できる設計（共有オブジェクトへの `shared_ptr<const T>`
など）を使います。

また、Stateに標準入出力・ログなどの副作用、乱数エンジン、タイマーを持たせるのは避けます。
候補生成は親Stateを変更せず、候補同士が独立するようにします。スコアをStateにキャッシュ
する場合は、すべての遷移・近傍変更で値が正しく保たれる必要があります。

| 関数 | 使用する探索方法 | 役割 |
|---|---|---|
| `next_states() const` | 貪欲法、ビームサーチ、ランダム探索、モンテカルロ | 現在状態から先に進む構築候補を列挙 |
| `neighbor_states() const` | 焼きなまし法 | 現在の解を少し変更した近傍候補を列挙 |

どちらの関数も現在状態を変更せず、候補ごとに独立して変更できる状態を返してください。
使わない方は初期状態のままで構いません。選択した探索方法が使う候補生成関数を実装して
ください。
特に焼きなまし法では `neighbor_states()` が必要です。構築途中の状態から次の手を列挙する
`next_states()` だけでは、焼きなまし法の近傍探索にはなりません。

探索中に終端を明示する場合は `bool finished` または `bool isFinish` を状態に追加して
`true` にできます。これらのフィールドを持たない場合は、構築候補が空であることを終端と
して扱います。`read_input()`、`evaluate(const State&)`、`write_answer(const State&)`
には問題固有の入力・評価・出力を実装してください。評価関数は状態のスコアを返します。

```cpp
struct State {
    vector<int> actions;
    int position = 0;

    // 構築型探索で使う。焼きなまし法では使わない。
    vector<State> next_states() const {
        vector<State> result;
        // 問題に応じてコピーした候補状態を追加する
        return result;
    }

    // 焼きなまし法で使う。現在の解の近傍状態を返す。
    vector<State> neighbor_states() const {
        vector<State> result;
        // 現在の解を少し変更した候補を追加する
        return result;
    }
};
```

`State` 内に共有する読み取り専用データや独自の型を持たせることもできます。候補ごとに
独立させる必要がある変更可能なデータは、候補生成時に適切にコピーしてください。
探索ライブラリは評価関数を各候補につき一度呼び、その評価値を探索中の状態と一緒に
保持します。初期状態も最初に一度だけ評価し、初期化や候補選択のために同じ状態を
再評価しません。モンテカルロの各プレイアウトでは、別々に到達した終端状態を評価します。
未実装の `evaluate()` は非数を返すため、共通探索処理が問題固有の実装を促すエラーを出します。
候補生成関数の初期候補が空の場合は、初期状態が意図的に終端である可能性を考慮して警告を
出します。入力・出力関数の本体もコメントを参考に実装してください。

診断ログはすべて標準エラー出力へ出し、標準出力には `write_answer()` の回答だけを
出します。既定の `none` は本番向けで、ログを出さずログ文字列も生成しません。
調査時に `error` / `warning` / `info` に設定すると、設定や探索の状況・終了理由を記録し、
問題固有関数の例外や未対応設定はエラーとして記録します。
`LOG_LEVEL = LogLevel::trace` にすると、各候補の生成数と評価値も記録できます。
ログ有効時は出力量に応じて実行時間が増えるため、提出時は `none` に設定してください。

### 補助関数・前処理を書く場所と実行順

問題固有コードは「共有データ・補助関数」から「`write_answer()`」までに書きます。
`State` のメソッドから使う補助関数は `State` より前に、`State` を引数に取る補助関数は
`State` の後に置いてください。任意の前処理は `preprocess(const State&)` に実装します。
ここにはBFSに限らず、問題に必要な前計算・表作成・データ検査などを自由に書けます。
前処理が不要なら空のままで構いません。`main()`から自動で呼び出されるため、
呼び出し側のコードを自分で追加する必要はありません。

実行順は `read_input()` → `preprocess(initial_state)` → 探索器の作成と初期評価 →
選択した探索方法 → `write_answer(answer)` です。`read_input()` で読み込んだ共有入力や
前処理結果は `problem_data` に格納し、`State` の候補生成や `evaluate()` から参照できます。
前処理中に例外が起きた場合は、探索を始める前にエラーとして記録されます。

```cpp
// ProblemDataには、全候補で共有する入力・前計算結果を追加する
struct ProblemData {
    vector<int> prepared_values;
};
ProblemData problem_data;

// Stateの定義後に置く。前計算の種類や内容は問題に合わせて自由に実装する
void preprocess(const State& initial_state) {
    problem_data.prepared_values.clear();
    // initial_stateやproblem_dataの入力を使って任意の準備処理を行う
}
```

## Python の提出用ファイル

AHC の提出には [`/submit.py`](/C:/Users/anchu/Desktop/codes/ahc-library/submit.py)
を1ファイルで提出できます。標準ライブラリ以外への依存はありません。特定問題の
入力例・状態例・評価例は入れていません。ファイル冒頭の「問題ごとの設定・実装」に、
入出力、実装する関数、探索方法の選択、設定値を一覧にしています。探索方法の選択は
`SEARCH_METHOD` の値を書き換えるだけです。共通探索ライブラリは編集不要です。

冒頭の設定値の意味は次のとおりです。

| 設定 | 内容 |
|---|---|
| `SEARCH_METHOD` | `"greedy"` / `"beam_search"` / `"simulated_annealing"` / `"fast_random"` / `"monte_carlo"` |
| `MAXIMIZE` | `True` は評価値最大化、`False` は最小化 |
| `RANDOM_SEED` | 乱数シード。再現性が不要なら `None` |
| `SEARCH_STEPS` | Greedy・ビームサーチの最大手数 |
| `BEAM_WIDTH` | ビームサーチで各層に残す状態数 |
| `TIME_LIMIT_MS` | 時間制限付き探索の時間（ミリ秒） |
| `START_TEMPERATURE`, `END_TEMPERATURE` | 焼きなましの開始・終了温度 |
| `PLAYOUTS_PER_CANDIDATE` | モンテカルロで候補ごとに行うロールアウト数 |

### 問題ごとに実装する箇所

1. **`State` のフィールド**
   1つの探索状態を再現するために必要な値を持たせます。問題入力のうち遷移・評価で
   参照する値も、状態に含めるか、評価関数が参照できる形で渡します。全候補に共通な
   読み取り専用データは複製時に共有し、候補ごとに変わるデータだけコピーすると効率的です。

2. **`read_input() -> State`**
   問題の入力形式を読み取り、初期状態を返します。トークン区切り、行区切り、複数テスト
   ケースなど、入力仕様に合ったパーサーをここに実装します。入力値を探索中に必要と
   する場合は、状態に保持するか、評価関数が利用できるようにしてください。

3. **`State.clone() -> State`**
   探索候補を作るための状態コピーを返します。探索ライブラリはコピーの中身を知りません。
   ミュータブルなリスト・配列などは候補間で共有せず、必要な範囲をコピーしてください。
   コピーが不完全だと、ある候補への変更が別候補や親状態に漏れます。

4. **`State.next_states() -> Iterable[State]`**
   現在状態から可能な次状態を列挙します。元状態は変更せず、各候補は独立して変更できる
   状態にしてください。合法手の生成や差分スコアの更新など、問題固有の遷移処理はここに
   実装します。合法手がない状態は空の iterable を返せます。終端を明示したい場合は
   状態に `finished = True` を設定します（省略時は未終了として扱われます）。

5. **`evaluate(state) -> float`**
   探索中に状態の良さを比較する評価関数です。スコアが大きいほど良いなら `MAXIMIZE = True`、
   小さいほど良いなら `False` にします。問題の目的関数だけでなく、必要な見込み・制約・
   残り手数などのヒューリスティックはここで組み合わせます。ライブラリ側は評価値を比較
   するだけで、問題固有のパラメータや評価の意味は解釈しません。計算量が実行時間に直結
   するため、可能なら状態にスコアを保持して差分更新してください。

6. **`write_answer(answer) -> None`**
   `solver.answer` を問題の指定する出力形式で出力します。最終状態から必要な操作列を復元
   する場合もここに実装します。標準出力にはジャッジが要求する形式だけを出してください。

### 探索の設定

`SEARCH_METHOD` で `greedy`、`beam_search`、`simulated_annealing`、`fast_random`、
`monte_carlo` のいずれかを選びます。`SEARCH_STEPS`、`BEAM_WIDTH`、`TIME_LIMIT_MS`、
温度、プレイアウト数などは探索方法の制御値です。問題固有の評価パラメータと混同せず、
評価に必要な問題情報は状態または評価関数側で管理してください。探索後の解は
`write_answer(solver.answer)` に渡されます。

ビームサーチは `evaluate` の値で各層の候補を順位付けします。Greedy は次の1手の比較、
焼きなましは近傍候補のランダム選択と温度による受理、ランダム探索とモンテカルロは
`next_states()` からのランダムなロールアウトを使います。探索手法を選ぶだけで問題の
遷移・評価・出力が自動生成されるわけではありません。特に焼きなましを使う場合は、
`next_states()` が現在解の近傍を適切に生成するようにしてください。

未実装の関数は明示的に `NotImplementedError` を出すため、そのままでは問題解答として
動きません。問題固有の6項目を実装してから提出してください。

### Python ライブラリ API

提出ファイルと分けて開発したい場合は、[`python/solver.py`](/C:/Users/anchu/Desktop/codes/ahc-library/python/solver.py)
を `python` パッケージから利用できます。

```python
from python import Solver, State

solver = Solver(initial_state, evaluate, maximize=True, seed=42)
solver.greedy(steps=100)
solver.beam_search(steps=100, width=100)
solver.simulated_annealing(time_limit_ms=1950, start_temp=1000, end_temp=10)
solver.fast_random(time_limit_ms=1950)
solver.monte_carlo(time_limit_ms=1950, playouts_per_candidate=30)
```

各探索メソッドは `None` を返し、最良状態を `solver.answer`、その評価値を
`solver.answer_score` に格納します。最大化・最小化は `Solver` の `maximize` 引数で
切り替えられます。乱数シードは `seed` 引数で指定できます。時間、幅、反復回数、
温度などの探索パラメータは各メソッドの引数で調整します。

従来の `python/beam_search.py` などの単体テンプレートも引き続き利用できます。
