# AHC Library made by anchu1024
# URL: https://github.com/anchu1024/ahc-library
"""AHC提出用の単一ファイル探索テンプレート。"""

from __future__ import annotations

import heapq
import math
import random
import time
from typing import Callable, Iterable, List, Optional, Tuple, TypeVar


# ============================================================================
# 問題ごとの設定・実装（提出時はこの冒頭部分を編集）
#
# 入力 : read_input()       -> 初期状態を返す
# 状態 : State              -> 問題固有の状態、clone()、next_states()を実装
# 評価 : evaluate(state)    -> 状態の良さを数値で返す
# 出力 : write_answer(state)-> ジャッジ指定の形式で解を出力
#
# 探索方法は SEARCH_METHOD で選択:
#   "greedy" / "beam_search" / "simulated_annealing"
#   "fast_random" / "monte_carlo"
#
# 調整値:
#   MAXIMIZE                 True: 最大化 / False: 最小化
#   RANDOM_SEED              乱数シード（Noneなら未固定）
#   SEARCH_STEPS             Greedy・ビームサーチの最大手数
#   BEAM_WIDTH               ビームサーチの保持状態数
#   TIME_LIMIT_MS            時間制限付き探索の実行時間（ms）
#   START_TEMPERATURE        焼きなまし開始温度
#   END_TEMPERATURE          焼きなまし終了温度
#   PLAYOUTS_PER_CANDIDATE   モンテカルロの候補ごとの試行回数
# ============================================================================
MAXIMIZE = True
RANDOM_SEED = None
SEARCH_METHOD = "beam_search"
SEARCH_STEPS = 100
BEAM_WIDTH = 100
TIME_LIMIT_MS = 1950
START_TEMPERATURE = 1000.0
END_TEMPERATURE = 10.0
PLAYOUTS_PER_CANDIDATE = 30


class State:
    """問題固有の状態データを持ち、以下の2メソッドを実装する。"""

    def clone(self) -> "State":
        """独立して変更できる状態コピーを返す。不変データは共有してよい。"""
        raise NotImplementedError("State.clone()を実装してください")

    def next_states(self) -> Iterable["State"]:
        """自状態を変更せず、合法な次状態を列挙して返す。"""
        raise NotImplementedError("State.next_states()を実装してください")


def read_input() -> State:
    """問題の入力を読み、初期状態を返す。"""
    raise NotImplementedError("read_input()を問題に合わせて実装してください")


def evaluate(state: State) -> float:
    """探索中の状態評価値を返す。"""
    raise NotImplementedError(
        f"{type(state).__name__}用のevaluate()を実装してください"
    )


def write_answer(answer: State) -> None:
    """解を問題指定の出力形式で標準出力へ出す。"""
    raise NotImplementedError(
        f"{type(answer).__name__}用のwrite_answer()を実装してください"
    )


# ============================================================================
# 共通探索ライブラリ（問題ごとの変更は不要）
# ============================================================================
StateT = TypeVar("StateT", bound=State)
Evaluator = Callable[[StateT], float]


class Solver:
    """共通の状態・評価関数を使う探索アルゴリズム群。"""

    def __init__(
        self,
        initial_state: StateT,
        evaluator: Evaluator[StateT],
        maximize: bool = True,
        seed: Optional[int] = None,
    ) -> None:
        self._initial_state = initial_state.clone()
        self._evaluate = evaluator
        self.maximize = maximize
        self._random = random.Random(seed)
        self.answer: StateT = self._initial_state
        self.answer_score = evaluator(self.answer)

    def _is_better(self, score: float, other_score: float) -> bool:
        return score > other_score if self.maximize else score < other_score

    def _reset_answer(self) -> Tuple[StateT, float]:
        self.answer = self._initial_state.clone()
        self.answer_score = self._evaluate(self.answer)
        return self.answer, self.answer_score

    def _record(self, state: StateT, score: float) -> None:
        if self._is_better(score, self.answer_score):
            self.answer = state
            self.answer_score = score

    @staticmethod
    def _deadline(time_limit_ms: float) -> float:
        if time_limit_ms < 0:
            raise ValueError("time_limit_ms must be non-negative")
        return time.perf_counter() + time_limit_ms / 1000.0

    def _rank_key(self, score: float) -> float:
        return score if self.maximize else -score

    def greedy(self, steps: int) -> None:
        if steps < 0:
            raise ValueError("steps must be non-negative")
        current, _ = self._reset_answer()
        evaluate_state = self._evaluate
        for _ in range(steps):
            if getattr(current, "finished", False):
                break
            candidates = list(current.next_states())
            if not candidates:
                break
            best_state = candidates[0]
            best_score = evaluate_state(best_state)
            for candidate in candidates[1:]:
                score = evaluate_state(candidate)
                if self._is_better(score, best_score):
                    best_state, best_score = candidate, score
            current = best_state
            self._record(current, best_score)

    def beam_search(self, steps: int, width: int = 100) -> None:
        if steps < 0:
            raise ValueError("steps must be non-negative")
        if width <= 0:
            raise ValueError("width must be positive")
        initial, initial_score = self._reset_answer()
        beam: List[Tuple[float, StateT]] = [(initial_score, initial)]
        evaluate_state = self._evaluate
        rank_key = self._rank_key

        for _ in range(steps):
            next_beam: List[Tuple[float, StateT]] = []
            expanded = False
            for score, state in beam:
                if getattr(state, "finished", False):
                    next_beam.append((score, state))
                    continue
                candidates = list(state.next_states())
                if not candidates:
                    next_beam.append((score, state))
                    continue
                expanded = True
                for candidate in candidates:
                    candidate_score = evaluate_state(candidate)
                    next_beam.append((candidate_score, candidate))
                    self._record(candidate, candidate_score)
            if not expanded:
                break
            beam = heapq.nlargest(width, next_beam, key=lambda item: rank_key(item[0]))

    def simulated_annealing(
        self,
        time_limit_ms: float = 1950,
        start_temp: float = 1000.0,
        end_temp: float = 10.0,
    ) -> None:
        if start_temp <= 0 or end_temp <= 0:
            raise ValueError("temperatures must be positive")
        deadline = self._deadline(time_limit_ms)
        current, current_score = self._reset_answer()
        start_time = time.perf_counter()
        rand = self._random.random
        exp = math.exp

        while time.perf_counter() < deadline:
            if getattr(current, "finished", False):
                break
            candidates = list(current.next_states())
            if not candidates:
                break
            candidate = candidates[self._random.randrange(len(candidates))]
            candidate_score = self._evaluate(candidate)
            elapsed = time.perf_counter() - start_time
            duration = deadline - start_time
            progress = min(1.0, elapsed / duration) if duration > 0 else 1.0
            temperature = start_temp + (end_temp - start_temp) * progress
            difference = candidate_score - current_score
            if not self.maximize:
                difference = -difference
            if difference > 0 or rand() < exp(difference / temperature):
                current, current_score = candidate, candidate_score
                self._record(current, current_score)

    def fast_random(
        self,
        time_limit_ms: float = 1950,
        max_steps: Optional[int] = None,
    ) -> None:
        if max_steps is not None and max_steps < 0:
            raise ValueError("max_steps must be non-negative")
        deadline = self._deadline(time_limit_ms)
        self._reset_answer()
        while time.perf_counter() < deadline:
            state, score, completed = self._rollout(
                self._initial_state.clone(), deadline, max_steps
            )
            if not completed:
                break
            self._record(state, score)

    def monte_carlo(
        self,
        time_limit_ms: float = 1950,
        playouts_per_candidate: int = 30,
        max_steps: Optional[int] = None,
    ) -> None:
        if playouts_per_candidate <= 0:
            raise ValueError("playouts_per_candidate must be positive")
        if max_steps is not None and max_steps < 0:
            raise ValueError("max_steps must be non-negative")
        deadline = self._deadline(time_limit_ms)
        current, _ = self._reset_answer()

        while time.perf_counter() < deadline:
            if getattr(current, "finished", False):
                break
            candidates = list(current.next_states())
            if not candidates:
                break
            best_state: Optional[StateT] = None
            best_average = 0.0
            for candidate in candidates:
                total = 0.0
                actual_playouts = 0
                for _ in range(playouts_per_candidate):
                    if time.perf_counter() >= deadline:
                        break
                    _, score, completed = self._rollout(
                        candidate.clone(), deadline, max_steps
                    )
                    if not completed:
                        break
                    total += score
                    actual_playouts += 1
                if actual_playouts == 0:
                    break
                average = total / actual_playouts
                if best_state is None or self._is_better(average, best_average):
                    best_state, best_average = candidate, average
                if time.perf_counter() >= deadline:
                    break
            if best_state is None:
                break
            current = best_state
            self._record(current, self._evaluate(current))

    def _rollout(
        self,
        state: StateT,
        deadline: float,
        max_steps: Optional[int],
    ) -> Tuple[StateT, float, bool]:
        steps = 0
        while not getattr(state, "finished", False):
            if time.perf_counter() >= deadline:
                return state, 0.0, False
            if max_steps is not None and steps >= max_steps:
                break
            candidates = list(state.next_states())
            if not candidates:
                break
            state = candidates[self._random.randrange(len(candidates))]
            steps += 1
        return state, self._evaluate(state), True

    def sa(
        self,
        time_limit_ms: float = 1950,
        start_temp: float = 1000.0,
        end_temp: float = 10.0,
    ) -> None:
        self.simulated_annealing(time_limit_ms, start_temp, end_temp)


def main() -> None:
    solver = Solver(read_input(), evaluate, maximize=MAXIMIZE, seed=RANDOM_SEED)
    if SEARCH_METHOD == "greedy":
        solver.greedy(SEARCH_STEPS)
    elif SEARCH_METHOD == "beam_search":
        solver.beam_search(SEARCH_STEPS, width=BEAM_WIDTH)
    elif SEARCH_METHOD == "simulated_annealing":
        solver.simulated_annealing(
            time_limit_ms=TIME_LIMIT_MS,
            start_temp=START_TEMPERATURE,
            end_temp=END_TEMPERATURE,
        )
    elif SEARCH_METHOD == "fast_random":
        solver.fast_random(time_limit_ms=TIME_LIMIT_MS)
    elif SEARCH_METHOD == "monte_carlo":
        solver.monte_carlo(
            time_limit_ms=TIME_LIMIT_MS,
            playouts_per_candidate=PLAYOUTS_PER_CANDIDATE,
        )
    else:
        raise ValueError(f"未対応の探索方法です: {SEARCH_METHOD}")
    write_answer(solver.answer)


if __name__ == "__main__":
    main()
