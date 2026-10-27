# ahc-library

AtCoder Heuristic Contest 向けの探索ライブラリです。C++ 用コンポーネントと
Python 用テンプレートに加え、Python では共通の `Solver` から複数の探索方法を
利用できます。

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
