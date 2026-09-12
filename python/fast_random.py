import sys
import time
import random

MAX = True  # True: 大きい方を残す / False: 小さい方を残す



# --- Input ---
def read_input():
    data = sys.stdin.read().strip().split()
    return data



# --- State ---
class State:
    def __init__(self):
        self.s = 0
        self.f = False # 終了判定
        # 問題依存の状態をここに入れる


    def clone(self):
        # deepcopy は遅いので必要な部分だけコピーする
        new = State()
        # new.xxx = self.xxx など必要なものだけコピー
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
def calc(st):
    """
    終端状態のスコアを計算する
    """
    s = 0
    # 問題依存
    return s



# --- Fast Random (Random Playout) ---
def fast_random(begin, time_limit_ms=1950):
    """
    begin_state からランダムプレイアウトを繰り返し、
    最良の状態を返す。
    """
    s_time = time.time()
    limit = time_limit_ms / 1000.0

    # 初期ベスト
    best = begin.clone()
    best.s = -2e18 if MAX else 2e18

    while time.time() - s_time < limit:
        cur = begin.clone()

        # ランダムに最後まで進める
        while not cur.f:
            if time.time() - s_time >= limit:
                return best

            n_st = gen_cand(cur)
            if not n_st:
                break

            # 完全ランダムで1つ選ぶ
            cur = random.choice(n_st)


        # 終端状態のスコア計算
        cur.s = calc(cur)

        # ベスト更新
        if MAX:
            if cur.s > best.s:
                best = cur
        else:
            if cur.s < best.s:
                best = cur

    return best



# --- Output ---
def output(st):
    # 問題依存
    print("")



def solve():
    data = read_input()
    i_st = State(data)

    best = fast_random(i_st)

    output(best)



if __name__ == "__main__":
    solve()