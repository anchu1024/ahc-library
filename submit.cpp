// AHC Library made by anchu1024
// URL: https://github.com/anchu1024/ahc-library
// AHC提出用の単一ファイルテンプレート。

#include <bits/stdc++.h>
using namespace std;

// ============================================================================
// 問題ごとの設定
// ============================================================================
// SEARCH_METHOD: "greedy", "beam_search", "simulated_annealing",
//                "fast_random", "monte_carlo" から選択
// MAXIMIZE: true は最大化、false は最小化
// RANDOM_SEED: nullopt は乱数固定なし、整数を指定すると再現可能
// LOG_LEVEL: none / error / warning / info / trace（noneは本番向けログ無効）
constexpr bool MAXIMIZE = true;
constexpr string_view SEARCH_METHOD = "beam_search";
constexpr int SEARCH_STEPS = 100;
constexpr int BEAM_WIDTH = 100;
constexpr double TIME_LIMIT_MS = 1950.0;
constexpr double START_TEMPERATURE = 1000.0;
constexpr double END_TEMPERATURE = 10.0;
constexpr int PLAYOUTS_PER_CANDIDATE = 30;
const optional<uint64_t> RANDOM_SEED = nullopt;
enum class LogLevel { none, error, warning, info, trace };
constexpr LogLevel LOG_LEVEL = LogLevel::none;

// ============================================================================
// 問題ごとの実装
// ============================================================================
// 【Stateに入れるもの・避けるもの】
// - Stateごとに変化する盤面・位置・操作列などを入れます。
// - 全Stateで共通の大きな入力データや前計算表は、Stateごとにコピーせず、
//   下の共有データ領域に置きます。Stateから参照するデータは探索中に変更しません。
// - 入出力、ログ、乱数エンジン、タイマーなどの副作用や実行環境依存の値は
//   Stateに持たせません。候補生成は親Stateを変更せず、独立した候補を返します。
// - Stateは探索ライブラリからコピーされます。生ポインタや参照を持たせる場合は
//   コピー後も有効であることを保証してください。通常は値、または共有データへの
//   const参照を使います。
// -
// スコアをStateにキャッシュする場合は、全遷移・近傍変更で必ず整合させてください。
//
// 【共有データ・補助関数（Stateの定義より前）】
// 入力全体や全State共通の前計算結果をここに定義します。
// Stateのメソッドから使う補助関数もここに置いてください。
struct ProblemData {
    // 例: 入力グラフ、距離表、探索用の定数など
};
ProblemData problem_data;

// Stateのメソッドより前に定義が必要な補助関数をここに書きます。

struct State {
    // Stateごとに変化する問題固有のフィールドを定義してください。

    // 貪欲法・ビームサーチ・ランダム探索・モンテカルロ用の「構築候補」。
    // 現在状態から可能な次状態を列挙し、現在状態自体は変更しないでください。
    vector<State> next_states() const { return {}; }

    // 焼きなまし法専用の「近傍候補」。
    // 現在の解を少し変更した候補を列挙し、現在状態自体は変更しないでください。
    vector<State> neighbor_states() const { return {}; }
};

// 【Stateを使う補助関数・前処理】
// Stateを引数に取る補助関数や任意の前処理をここに書きます。
// 前処理は入力読込後、探索器の作成（初期Stateの評価）より前に一度だけ呼ばれます。
// 必要なければ空のままで構いません。BFSに限らず、距離表・候補表・DP表など
// 問題に応じた共有データをproblem_dataに作成できます。
// main()側から自動で呼ぶため、呼び出しコードを追加する必要はありません。
void preprocess(const State&) {}

State read_input() {
    // 問題の入力を読み、初期状態を返してください。
    // 共通データをproblem_dataに読み込んでも構いません。
    return {};
}

double evaluate(const State&) {
    // 問題に合わせた評価値を返してください。
    return numeric_limits<double>::quiet_NaN();
}

void write_answer(const State&) {
    // ジャッジ指定の形式で回答を出力してください。
}

// ============================================================================
// 共通探索ライブラリ（以下は問題ごとの変更不要）
// ============================================================================
namespace ahc {

using Clock = chrono::steady_clock;

inline string_view log_level_name(LogLevel level) {
    switch (level) {
        case LogLevel::none:
            return "なし";
        case LogLevel::error:
            return "エラー";
        case LogLevel::warning:
            return "警告";
        case LogLevel::info:
            return "情報";
        case LogLevel::trace:
            return "詳細";
    }
    return "不明";
}

inline void log(LogLevel level, string_view area, string_view message) {
    if constexpr (LOG_LEVEL == LogLevel::none) return;
    if (static_cast<int>(level) > static_cast<int>(LOG_LEVEL)) return;
    cerr << '[' << log_level_name(level) << "][" << area << "] " << message
         << '\n';
}

template <class Formatter>
void trace(string_view area, Formatter&& formatter) {
    if constexpr (LOG_LEVEL == LogLevel::trace) {
        log(LogLevel::trace, area, forward<Formatter>(formatter)());
    }
}

template <class T, class = void>
struct HasFinishedMember : false_type {};

template <class T>
struct HasFinishedMember<T, void_t<decltype(declval<const T&>().finished)>>
    : true_type {};

template <class T, class = void>
struct HasIsFinishMember : false_type {};

template <class T>
struct HasIsFinishMember<T, void_t<decltype(declval<const T&>().isFinish)>>
    : true_type {};

template <class T>
bool is_finished(const T& state) {
    if constexpr (HasFinishedMember<T>::value) {
        return static_cast<bool>(state.finished);
    } else if constexpr (HasIsFinishMember<T>::value) {
        return static_cast<bool>(state.isFinish);
    } else {
        return false;
    }
}

template <class StateT, class Evaluator>
class Solver {
   private:
    StateT initial_state_;
    Evaluator evaluator_;
    bool maximize_;
    optional<uint64_t> seed_;
    mt19937_64 random_;
    string_view algorithm_ = "初期化";
    double initial_score_;
    Clock::time_point run_start_ = Clock::now();
    size_t evaluation_count_ = 0;
    size_t candidate_count_ = 0;
    size_t iteration_count_ = 0;

   public:
    StateT answer;
    double answer_score;

    Solver(const StateT& initial_state, Evaluator evaluator,
           bool maximize = true, optional<uint64_t> seed = nullopt)
        : initial_state_(initial_state),
          evaluator_(move(evaluator)),
          maximize_(maximize),
          seed_(seed),
          random_(seed_ ? *seed_ : random_device{}()),
          initial_score_(checked_evaluate(initial_state_, "初期評価", 0)),
          answer(initial_state_),
          answer_score(initial_score_) {}

    void greedy(int steps) {
        if (steps < 0) throw invalid_argument("stepsは0以上にしてください");
        begin_run("貪欲法");
        reset_answer();
        StateT current = initial_state_;
        bool first_expansion = true;
        string_view stop_reason = "指定手数に到達";

        for (int step = 0; step < steps && !is_finished(current); ++step) {
            iteration_count_ = static_cast<size_t>(step + 1);
            vector<StateT> candidates = generate_candidates(
                "構築候補", step, [&] { return current.next_states(); });
            if (candidates.empty()) {
                warn_if_initial_candidates_empty(first_expansion,
                                                 "next_states()");
                stop_reason = "候補がありません";
                break;
            }
            first_expansion = false;

            auto best = candidates.begin();
            double best_score = checked_evaluate(*best, "候補評価", step);
            for (auto it = next(candidates.begin()); it != candidates.end();
                 ++it) {
                const double score = checked_evaluate(*it, "候補評価", step);
                if (is_better(score, best_score)) {
                    best = it;
                    best_score = score;
                }
            }
            current = move(*best);
            record(current, best_score);
        }
        if (is_finished(current)) stop_reason = "終端状態に到達";
        end_run(stop_reason);
    }

    void beam_search(int steps, int width = 100) {
        if (steps < 0) throw invalid_argument("stepsは0以上にしてください");
        if (width <= 0) throw invalid_argument("widthは1以上にしてください");
        begin_run("ビームサーチ");
        reset_answer();

        vector<pair<double, StateT>> beam;
        beam.emplace_back(answer_score, answer);
        bool first_expansion = true;
        string_view stop_reason = "指定手数に到達";
        for (int step = 0; step < steps; ++step) {
            iteration_count_ = static_cast<size_t>(step + 1);
            vector<pair<double, StateT>> next_beam;
            bool expanded = false;
            bool had_unfinished_state = false;
            for (const auto& [score, state] : beam) {
                if (is_finished(state)) {
                    next_beam.emplace_back(score, state);
                    continue;
                }
                had_unfinished_state = true;
                vector<StateT> candidates = generate_candidates(
                    "構築候補", step, [&] { return state.next_states(); });
                if (candidates.empty()) {
                    if (first_expansion) {
                        warn_if_initial_candidates_empty(first_expansion,
                                                         "next_states()");
                    }
                    next_beam.emplace_back(score, state);
                    continue;
                }
                first_expansion = false;
                expanded = true;
                for (StateT& candidate : candidates) {
                    const double candidate_score =
                        checked_evaluate(candidate, "候補評価", step);
                    next_beam.emplace_back(candidate_score, move(candidate));
                    record(next_beam.back().second, candidate_score);
                }
            }
            if (!expanded) {
                stop_reason = had_unfinished_state
                                  ? "展開可能な状態がありません"
                                  : "全状態が終端です";
                break;
            }

            sort(next_beam.begin(), next_beam.end(),
                 [this](const auto& a, const auto& b) {
                     return is_better(a.first, b.first);
                 });
            if (static_cast<int>(next_beam.size()) > width) {
                next_beam.resize(width);
            }
            beam = move(next_beam);
        }
        end_run(stop_reason);
    }

    void simulated_annealing(double time_limit_ms = 1950.0,
                             double start_temp = 1000.0,
                             double end_temp = 10.0) {
        if (!isfinite(start_temp) || !isfinite(end_temp) || start_temp <= 0 ||
            end_temp <= 0) {
            throw invalid_argument("温度は正の値にしてください");
        }
        begin_run("焼きなまし法");
        const auto start = Clock::now();
        const auto deadline = make_deadline(time_limit_ms);
        reset_answer();
        StateT current = initial_state_;
        double current_score = answer_score;
        bool first_expansion = true;
        string_view stop_reason = "時間制限に到達";

        while (Clock::now() < deadline && !is_finished(current)) {
            ++iteration_count_;
            vector<StateT> candidates =
                generate_candidates("近傍候補", iteration_count_ - 1,
                                    [&] { return current.neighbor_states(); });
            if (candidates.empty()) {
                warn_if_initial_candidates_empty(first_expansion,
                                                 "neighbor_states()");
                stop_reason = "近傍候補がありません";
                break;
            }
            first_expansion = false;
            StateT candidate =
                move(candidates[uniform_index(candidates.size())]);
            const double candidate_score =
                checked_evaluate(candidate, "近傍評価", iteration_count_ - 1);
            const double elapsed = elapsed_ms(start);
            const double duration = max(0.0, time_limit_ms);
            const double progress =
                duration > 0 ? min(1.0, elapsed / duration) : 1.0;
            const double temperature =
                start_temp + (end_temp - start_temp) * progress;
            double difference = candidate_score - current_score;
            if (!maximize_) difference = -difference;

            if (difference > 0 ||
                uniform_real_distribution<double>(0.0, 1.0)(random_) <
                    exp(difference / temperature)) {
                current = move(candidate);
                current_score = candidate_score;
                record(current, current_score);
                trace(algorithm_, [&] {
                    return string("近傍を受理しました（手数=") +
                           to_string(iteration_count_) + "）";
                });
            } else {
                trace(algorithm_, [&] {
                    return string("近傍を棄却しました（手数=") +
                           to_string(iteration_count_) + "）";
                });
            }
        }
        if (is_finished(current)) stop_reason = "終端状態に到達";
        end_run(stop_reason);
    }

    void fast_random(double time_limit_ms = 1950.0) {
        const auto deadline = make_deadline(time_limit_ms);
        begin_run("ランダム探索");
        reset_answer();
        bool first_expansion = true;
        string_view stop_reason = "時間制限に到達";
        while (Clock::now() < deadline) {
            StateT state = initial_state_;
            bool changed = false;
            while (!is_finished(state)) {
                if (Clock::now() >= deadline) break;
                vector<StateT> candidates =
                    generate_candidates("構築候補", iteration_count_,
                                        [&] { return state.next_states(); });
                if (candidates.empty()) {
                    warn_if_initial_candidates_empty(first_expansion,
                                                     "next_states()");
                    stop_reason = "候補がありません";
                    break;
                }
                first_expansion = false;
                state = move(candidates[uniform_index(candidates.size())]);
                changed = true;
                ++iteration_count_;
            }
            if (Clock::now() >= deadline) break;
            const double state_score =
                changed ? checked_evaluate(state, "ロールアウト評価",
                                           iteration_count_)
                        : initial_score_;
            record(state, state_score);
            ++iteration_count_;
        }
        end_run(stop_reason);
    }

    void monte_carlo(double time_limit_ms = 1950.0,
                     int playouts_per_candidate = 30) {
        if (playouts_per_candidate <= 0) {
            throw invalid_argument(
                "playouts_per_candidateは1以上にしてください");
        }
        const auto deadline = make_deadline(time_limit_ms);
        begin_run("モンテカルロ法");
        reset_answer();
        StateT current = initial_state_;
        bool first_expansion = true;
        string_view stop_reason = "時間制限に到達";

        while (Clock::now() < deadline && !is_finished(current)) {
            ++iteration_count_;
            vector<StateT> candidates =
                generate_candidates("構築候補", iteration_count_ - 1,
                                    [&] { return current.next_states(); });
            if (candidates.empty()) {
                warn_if_initial_candidates_empty(first_expansion,
                                                 "next_states()");
                stop_reason = "候補がありません";
                break;
            }
            first_expansion = false;
            optional<StateT> best_state;
            double best_average = 0;
            double best_state_score = 0;

            for (StateT& candidate : candidates) {
                const double candidate_score = checked_evaluate(
                    candidate, "候補評価", iteration_count_ - 1);
                long double total = 0;
                int actual_playouts = 0;
                for (int i = 0; i < playouts_per_candidate; ++i) {
                    if (Clock::now() >= deadline) break;
                    auto [final_score, completed] = rollout(
                        candidate, candidate_score, deadline, iteration_count_);
                    if (!completed) break;
                    total += final_score;
                    ++actual_playouts;
                }
                if (actual_playouts == 0) break;
                const double average =
                    static_cast<double>(total / actual_playouts);
                if (!isfinite(average)) {
                    throw runtime_error(
                        "モンテカルロの平均評価値が有限値ではありません");
                }
                if (!best_state || is_better(average, best_average)) {
                    best_state = candidate;
                    best_average = average;
                    best_state_score = candidate_score;
                }
                if (Clock::now() >= deadline) break;
            }

            if (!best_state) break;
            current = move(*best_state);
            record(current, best_state_score);
        }
        if (is_finished(current)) stop_reason = "終端状態に到達";
        end_run(stop_reason);
    }

   private:
    void begin_run(string_view algorithm) {
        algorithm_ = algorithm;
        if constexpr (LOG_LEVEL >= LogLevel::info) {
            run_start_ = Clock::now();
            evaluation_count_ = 1;
            candidate_count_ = 0;
            iteration_count_ = 0;
            ostringstream message;
            message << "探索を開始します（最大化=" << boolalpha << maximize_
                    << "、初期評価値=" << initial_score_ << "、乱数シード=";
            if (seed_) {
                message << *seed_;
            } else {
                message << "未固定";
            }
            message << "）";
            log(LogLevel::info, algorithm_, message.str());
        }
    }

    void end_run(string_view reason) {
        if constexpr (LOG_LEVEL >= LogLevel::info) {
            ostringstream message;
            message << "探索を終了します（理由=" << reason
                    << "、反復=" << iteration_count_
                    << "、生成候補=" << candidate_count_
                    << "、評価回数=" << evaluation_count_
                    << "、最良評価値=" << setprecision(17) << answer_score
                    << "、経過時間=" << elapsed_ms(run_start_) << "ms）";
            log(LogLevel::info, algorithm_, message.str());
        }
    }

    double checked_evaluate(const StateT& state, string_view phase,
                            size_t step) {
        if constexpr (LOG_LEVEL >= LogLevel::info) {
            ++evaluation_count_;
        }
        double score = 0;
        try {
            score = evaluator_(state);
        } catch (const exception& error) {
            log_callback_error(phase, step, error.what());
            throw;
        } catch (...) {
            log_callback_error(phase, step, "不明な例外");
            throw;
        }
        if (!isfinite(score)) {
            ostringstream message;
            message << phase << "（手数=" << step
                    << "）が有限のスコアを返しませんでした。evaluate()"
                       "を確認してください。";
            log(LogLevel::error, algorithm_, message.str());
            throw runtime_error(message.str());
        }
        log_score(phase, step, score);
        return score;
    }

    template <class Generator>
    vector<StateT> generate_candidates(string_view phase, size_t step,
                                       Generator&& generator) {
        vector<StateT> candidates;
        try {
            candidates = forward<Generator>(generator)();
        } catch (const exception& error) {
            log_callback_error(phase, step, error.what());
            throw;
        } catch (...) {
            log_callback_error(phase, step, "不明な例外");
            throw;
        }
        if constexpr (LOG_LEVEL >= LogLevel::info) {
            candidate_count_ += candidates.size();
        }
        trace(algorithm_, [&] {
            ostringstream message;
            message << phase << "を生成しました（手数=" << step
                    << "、候補数=" << candidates.size() << "）";
            return message.str();
        });
        return candidates;
    }

    void log_score(string_view phase, size_t step, double score) const {
        trace(algorithm_, [&] {
            ostringstream message;
            message << phase << "を評価しました（手数=" << step
                    << "、評価値=" << setprecision(17) << score << "）";
            return message.str();
        });
    }

    void log_callback_error(string_view phase, size_t step,
                            string_view detail) const {
        if constexpr (LOG_LEVEL >= LogLevel::error) {
            ostringstream message;
            message << phase << "で例外が発生しました（手数=" << step
                    << "）: " << detail;
            log(LogLevel::error, algorithm_, message.str());
        }
    }

    static void warn_if_initial_candidates_empty(bool& first_expansion,
                                                 const char* generator_name) {
        if (!first_expansion) return;
        first_expansion = false;
        if constexpr (LOG_LEVEL >= LogLevel::warning) {
            ostringstream message;
            message << "初期状態の " << generator_name
                    << " が空です。初期状態が終端でなければ候補生成を確認してく"
                       "ださい。";
            log(LogLevel::warning, "候補生成", message.str());
        }
    }

    bool is_better(double score, double other_score) const {
        return maximize_ ? score > other_score : score < other_score;
    }

    void reset_answer() {
        answer = initial_state_;
        answer_score = initial_score_;
    }

    void record(const StateT& state, double score) {
        if (is_better(score, answer_score)) {
            answer = state;
            answer_score = score;
            trace(algorithm_, [&] {
                ostringstream message;
                message << "最良解を更新しました（評価値=" << setprecision(17)
                        << score << "）";
                return message.str();
            });
        }
    }

    static auto make_deadline(double time_limit_ms) {
        if (!isfinite(time_limit_ms) || time_limit_ms < 0) {
            throw invalid_argument("time_limit_msは有限の0以上にしてください");
        }
        const auto now = Clock::now();
        const double max_time_ms =
            chrono::duration<double, milli>(Clock::time_point::max() - now)
                .count();
        if (time_limit_ms > max_time_ms) {
            throw invalid_argument(
                "time_limit_msが表現可能な時間範囲を超えています");
        }
        return now + chrono::duration_cast<Clock::duration>(
                         chrono::duration<double, milli>(time_limit_ms));
    }

    static double elapsed_ms(Clock::time_point start) {
        return chrono::duration<double, milli>(Clock::now() - start).count();
    }

    size_t uniform_index(size_t size) {
        return uniform_int_distribution<size_t>(0, size - 1)(random_);
    }

    pair<double, bool> rollout(const StateT& initial, double initial_score,
                               Clock::time_point deadline, size_t step) {
        StateT state = initial;
        bool changed = false;
        while (!is_finished(state)) {
            if (Clock::now() >= deadline) return {0.0, false};
            vector<StateT> candidates = generate_candidates(
                "ロールアウト候補", step, [&] { return state.next_states(); });
            if (candidates.empty()) {
                if (!changed) {
                    log(LogLevel::trace, algorithm_,
                        "ロールアウト開始状態が終端でした");
                }
                return {changed ? checked_evaluate(state, "終端状態評価", step)
                                : initial_score,
                        true};
            }
            state = move(candidates[uniform_index(candidates.size())]);
            changed = true;
        }
        return {changed ? checked_evaluate(state, "終端状態評価", step)
                        : initial_score,
                true};
    }
};

}  // namespace ahc

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    string_view stage = "初期化";
    try {
        if constexpr (LOG_LEVEL != LogLevel::none) {
            ostringstream settings;
            settings << "探索方法=" << SEARCH_METHOD
                     << "、目的=" << (MAXIMIZE ? "最大化" : "最小化")
                     << "、ログレベル=" << ahc::log_level_name(LOG_LEVEL)
                     << "、最大手数=" << SEARCH_STEPS
                     << "、ビーム幅=" << BEAM_WIDTH
                     << "、制限時間=" << TIME_LIMIT_MS << "ms"
                     << "、温度=" << START_TEMPERATURE << "->"
                     << END_TEMPERATURE
                     << "、候補ごとの試行回数=" << PLAYOUTS_PER_CANDIDATE
                     << "、乱数シード=";
            if (RANDOM_SEED) {
                settings << *RANDOM_SEED;
            } else {
                settings << "未固定";
            }
            ahc::log(LogLevel::info, "実行", settings.str());
        }

        stage = "入力読み込み";
        if constexpr (LOG_LEVEL >= LogLevel::info) {
            ahc::log(LogLevel::info, stage, "入力を読み込みます");
        }
        State initial_state = read_input();
        if constexpr (LOG_LEVEL >= LogLevel::info) {
            ahc::log(LogLevel::info, stage, "入力を読み込みました");
        }

        stage = "問題固有の前処理";
        if constexpr (LOG_LEVEL >= LogLevel::info) {
            ahc::log(LogLevel::info, stage, "前処理を開始します");
        }
        preprocess(initial_state);
        if constexpr (LOG_LEVEL >= LogLevel::info) {
            ahc::log(LogLevel::info, stage, "前処理が完了しました");
        }

        stage = "初期化・初期評価";
        ahc::Solver<State, decltype(&evaluate)> solver(initial_state, evaluate,
                                                       MAXIMIZE, RANDOM_SEED);

        stage = "探索";
        if (SEARCH_METHOD == "greedy") {
            solver.greedy(SEARCH_STEPS);
        } else if (SEARCH_METHOD == "beam_search") {
            solver.beam_search(SEARCH_STEPS, BEAM_WIDTH);
        } else if (SEARCH_METHOD == "simulated_annealing") {
            solver.simulated_annealing(TIME_LIMIT_MS, START_TEMPERATURE,
                                       END_TEMPERATURE);
        } else if (SEARCH_METHOD == "fast_random") {
            solver.fast_random(TIME_LIMIT_MS);
        } else if (SEARCH_METHOD == "monte_carlo") {
            solver.monte_carlo(TIME_LIMIT_MS, PLAYOUTS_PER_CANDIDATE);
        } else {
            throw invalid_argument("未対応のSEARCH_METHODです");
        }

        stage = "回答出力";
        if constexpr (LOG_LEVEL >= LogLevel::info) {
            ahc::log(LogLevel::info, stage, "回答を標準出力へ出力します");
        }
        write_answer(solver.answer);
        if (!cout) {
            throw runtime_error("回答の標準出力に失敗しました");
        }
        if constexpr (LOG_LEVEL >= LogLevel::info) {
            ahc::log(LogLevel::info, stage, "回答を出力しました");
        }
    } catch (const exception& error) {
        if constexpr (LOG_LEVEL >= LogLevel::error) {
            ahc::log(LogLevel::error, stage, error.what());
        }
        return 1;
    } catch (...) {
        if constexpr (LOG_LEVEL >= LogLevel::error) {
            ahc::log(LogLevel::error, stage, "原因不明の例外が発生しました");
        }
        return 1;
    }
    return 0;
}
