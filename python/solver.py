"""Reusable search algorithms for heuristic contest problems."""

from __future__ import annotations

import heapq
import math
import random
import time
from typing import Callable, Iterable, List, Optional, Tuple, TypeVar


class State:
    """Base interface for a problem state.

    Implement ``clone`` and ``next_states`` in a subclass. ``next_states`` must
    return new states without mutating the source state. An optional boolean
    ``finished`` attribute marks terminal states.
    """

    def clone(self) -> "State":
        raise NotImplementedError

    def next_states(self) -> Iterable["State"]:
        raise NotImplementedError


StateT = TypeVar("StateT", bound=State)
Evaluator = Callable[[StateT], float]


class Solver:
    """Run several search algorithms against the same state/evaluation logic.

    Search methods update ``answer`` in place and deliberately return ``None``.
    """

    def __init__(
        self,
        initial_state: StateT,
        evaluate: Evaluator[StateT],
        maximize: bool = True,
        seed: Optional[int] = None,
    ) -> None:
        self._initial_state = initial_state.clone()
        self._evaluate = evaluate
        self.maximize = maximize
        self._random = random.Random(seed)
        self.answer: StateT = self._initial_state
        self.answer_score = evaluate(self.answer)

    def _is_better(self, score: float, other_score: float) -> bool:
        if self.maximize:
            return score > other_score
        return score < other_score

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
        """Repeatedly take the best next state, for at most ``steps`` turns."""
        if steps < 0:
            raise ValueError("steps must be non-negative")
        current, _ = self._reset_answer()
        evaluate = self._evaluate
        for _ in range(steps):
            if getattr(current, "finished", False):
                break
            candidates = list(current.next_states())
            if not candidates:
                break
            best_state = candidates[0]
            best_score = evaluate(best_state)
            for candidate in candidates[1:]:
                score = evaluate(candidate)
                if self._is_better(score, best_score):
                    best_state, best_score = candidate, score
            current, current_score = best_state, best_score
            self._record(current, current_score)

    def beam_search(self, steps: int, width: int = 100) -> None:
        """Expand up to ``steps`` layers, retaining at most ``width`` states."""
        if steps < 0:
            raise ValueError("steps must be non-negative")
        if width <= 0:
            raise ValueError("width must be positive")

        initial, initial_score = self._reset_answer()
        beam: List[Tuple[float, StateT]] = [(initial_score, initial)]
        evaluate = self._evaluate
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
                    candidate_score = evaluate(candidate)
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
        """Run simulated annealing until the time limit expires."""
        if start_temp <= 0 or end_temp <= 0:
            raise ValueError("temperatures must be positive")
        deadline = self._deadline(time_limit_ms)
        current, current_score = self._reset_answer()
        start_time = time.perf_counter()
        evaluate = self._evaluate
        rand = self._random.random
        exp = math.exp

        while time.perf_counter() < deadline:
            if getattr(current, "finished", False):
                break
            candidates = list(current.next_states())
            if not candidates:
                break
            candidate = candidates[self._random.randrange(len(candidates))]
            candidate_score = evaluate(candidate)
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
        """Repeatedly roll out random paths from the initial state."""
        if max_steps is not None and max_steps < 0:
            raise ValueError("max_steps must be non-negative")
        deadline = self._deadline(time_limit_ms)
        self._reset_answer()

        while time.perf_counter() < deadline:
            state = self._initial_state.clone()
            final_state, score, completed = self._rollout(state, deadline, max_steps)
            if not completed:
                break
            self._record(final_state, score)

    def monte_carlo(
        self,
        time_limit_ms: float = 1950,
        playouts_per_candidate: int = 30,
        max_steps: Optional[int] = None,
    ) -> None:
        """Choose each move by averaging random playout scores."""
        if playouts_per_candidate <= 0:
            raise ValueError("playouts_per_candidate must be positive")
        if max_steps is not None and max_steps < 0:
            raise ValueError("max_steps must be non-negative")
        deadline = self._deadline(time_limit_ms)
        current, _ = self._reset_answer()
        evaluate = self._evaluate

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
                    playout_state = candidate.clone()
                    _, score, completed = self._rollout(
                        playout_state, deadline, max_steps
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
            self._record(current, evaluate(current))

    def _rollout(
        self,
        state: StateT,
        deadline: float,
        max_steps: Optional[int],
    ) -> Tuple[StateT, float, bool]:
        evaluate = self._evaluate
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
        return state, evaluate(state), True

    def sa(
        self,
        time_limit_ms: float = 1950,
        start_temp: float = 1000.0,
        end_temp: float = 10.0,
    ) -> None:
        """Short alias for :meth:`simulated_annealing`."""
        self.simulated_annealing(time_limit_ms, start_temp, end_temp)
