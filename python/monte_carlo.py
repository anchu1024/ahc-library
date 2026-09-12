import sys
import time
import random

MAX = True
PLAY_CNT = 30  # 1候補あたりのプレイアウト回数（問題依存）



# --- Input ---
def read_input():
    data = sys.stdin.read().strip().split()
    return data



# --- State ---
class State:
    __slots__ = ("s", "f", "d")
    # __slots__ により Python のオブジェクトが高速化される（超重要）

    def __init__(self, data=None):
        self.s = 0
        self.f = False
        self.d = data  # 問題依存の状態


    def clone(self):
        # deepcopy は絶対に使わない
        new = State(self.d)
        new.s = self.s
        new.f = self.f
        return new



# --- Candidate Generation ---
def gen_cand(st):
    """
    state から次の候補 State を複数生成して返す
    """
    res = []
    # 問題依存
    return res



# --- Score Calculation ---
def calculate(st):
    """
    終局時のスコアを計算する。
    差分更新できるなら絶対に差分更新する。
    """
    s = 0
    # 問題依存
    return s



# --- Random Next State ---
def gen_rand_next(st):
    """
    プレイアウト用のランダム遷移
    """
    n_st = gen_cand(st)
    if not n_st:
        end = st.clone()
        end.f = True
        return end

    # random.choice は C 実装なので高速
    return random.choice(n_st)



# --- Playout ---
def playout(st):
    """
    終局までランダムに進めて最終スコアを返す
    """
    # ローカル変数化で高速化
    nex = gen_rand_next
    calc = calculate

    while not st.f:
        st = nex(st)

    return calc(st)



# --- Monte Carlo ---
def monte_carlo(begin, time_limit_ms=1950):
    start = time.time()
    limit = time_limit_ms / 1000.0

    cur = begin.clone()

    # ローカル変数化（高速化のため）
    get_cand = gen_cand
    play = playout
    rand = random.random

    while True:
        now = time.time() - start
        if now >= limit:
            return cur

        n_st = get_cand(cur)
        if not n_st:
            return cur

        # ベスト候補
        best_s = -1e18 if MAX else 1e18
        best = n_st[0]

        for st in n_st:
            s_sum = 0
            actual = 0

            # プレイアウトを複数回実行
            for _ in range(PLAY_CNT):
                if time.time() - start >= limit:
                    break
                s_sum += playout(st.clone())
                actual += 1

            if actual == 0:
                break

            avg_s = s_sum / actual

            if MAX:
                if avg_s > best_s:
                    best_s = avg_s
                    best = st
            else:
                if avg_s < best_s:
                    best_s = avg_s
                    best = st

        cur = best

        if cur.f:
            return cur



# --- Output ---
def output(st):
    # 問題依存
    print("")



def solve():
    data = read_input()
    i_st = State(data)

    best = monte_carlo(i_st)

    output(best)



if __name__ == "__main__":
    solve()