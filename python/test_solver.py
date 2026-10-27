from pathlib import Path
import subprocess
import sys
import unittest

from python import Solver, State


class NumberState(State):
    def __init__(self, value=0):
        self.value = value
        self.finished = value == 0 or value >= 4

    def clone(self):
        return NumberState(self.value)

    def next_states(self):
        if self.finished:
            return ()
        return (NumberState(self.value + 1), NumberState(self.value - 1))


def evaluate(state):
    return state.value


class SolverTests(unittest.TestCase):
    def test_unimplemented_submission_template_fails_with_actionable_message(self):
        template = Path(__file__).with_name("submit.py")
        result = subprocess.run(
            [sys.executable, str(template)],
            capture_output=True,
            check=False,
        )
        self.assertNotEqual(result.returncode, 0)
        self.assertIn(b"read_input()", result.stderr)

    def test_greedy_and_beam_search_update_answer_reference(self):
        solver = Solver(NumberState(1), evaluate, seed=1)

        self.assertIsNone(solver.greedy(10))
        self.assertEqual(solver.answer.value, 4)
        self.assertEqual(solver.answer_score, 4)

        self.assertIsNone(solver.beam_search(10, width=3))
        self.assertEqual(solver.answer.value, 4)

    def test_minimization(self):
        solver = Solver(NumberState(2), evaluate, maximize=False, seed=1)
        solver.greedy(10)
        self.assertEqual(solver.answer.value, 0)

    def test_rollout_algorithms_update_answer(self):
        annealing = Solver(NumberState(1), evaluate, seed=1)
        annealing.simulated_annealing(time_limit_ms=20)
        self.assertIn(annealing.answer.value, (1, 2, 3, 4))

        fast_random = Solver(NumberState(1), evaluate, seed=1)
        fast_random.fast_random(time_limit_ms=20)
        self.assertEqual(fast_random.answer.value, 4)

        monte_carlo = Solver(NumberState(1), evaluate, seed=1)
        monte_carlo.monte_carlo(time_limit_ms=20, playouts_per_candidate=8)
        self.assertEqual(monte_carlo.answer.value, 4)

    def test_timed_algorithms_accept_zero_budget(self):
        solver = Solver(NumberState(1), evaluate, seed=1)
        self.assertIsNone(solver.simulated_annealing(time_limit_ms=0))
        self.assertEqual(solver.answer.value, 1)
        self.assertIsNone(solver.fast_random(time_limit_ms=0))
        self.assertEqual(solver.answer.value, 1)
        self.assertIsNone(solver.monte_carlo(time_limit_ms=0))
        self.assertEqual(solver.answer.value, 1)

    def test_invalid_search_parameters_are_rejected(self):
        solver = Solver(NumberState(), evaluate)
        with self.assertRaises(ValueError):
            solver.beam_search(1, width=0)
        with self.assertRaises(ValueError):
            solver.monte_carlo(playouts_per_candidate=0)
        with self.assertRaises(ValueError):
            solver.fast_random(max_steps=-1)


if __name__ == "__main__":
    unittest.main()
