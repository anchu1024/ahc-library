import sys
import time
import random
import math

MAX = True  # True: 大きい方を残す / False: 小さい方を残す



# --- Input ---
def read_input():
    data = sys.stdin.read().strip().split()
    return data



# --- State ---
class State:
    __slots__ = ("s", "f", "d")  
    # __slots__ を使うと Python のオブジェクトが高速化される（超重要）

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



# --- Next State (近傍生成) ---
def gen_cand(st):
    """
    状態を「少しだけ」変化させる。
    Python ではここが一番重くなるので、軽量化が命。
    """
    new = st.clone()
    # 問題依存の近傍操作を書く
    # 例: new.data[i] ^= 1 など
    return new



# --- Score Calculation ---
def calc(st):
    """
    スコア計算は O(1) に近づける。
    差分更新できるなら絶対に差分更新する。
    """
    s = 0
    # 問題依存
    return s



# --- Simulated Annealing ---
def sa(begin, time_limit_ms=1950):
    start = time.time()
    limit = time_limit_ms / 1000.0

    cur = begin.clone()
    cur.s = calc(cur)

    best = cur.clone()

    # --- Hyperparameters ---
    s_temp = 1000.0
    e_temp = 10.0

    # Python SA の高速化のための工夫
    rand = random.random  # ローカル変数化で高速化
    exp = math.exp        # ローカル化で高速化

    it_cnt = 0

    while True:
        now = time.time() - start
        if now >= limit:
            break

        # 温度を線形に下げる
        temp = s_temp + (e_temp - s_temp) * (now / limit)

        # 近傍生成
        n_st = gen_cand(cur)
        n_st.s = calc(n_st)

        # スコア差分
        diff = n_st.s - cur.s
        if not MAX:
            diff = -diff

        # 遷移確率
        # diff > 0 なら必ず遷移
        if diff > 0 or rand() < exp(diff / temp):
            cur = n_st

            # ベスト更新
            if MAX:
                if cur.s > best.s:
                    best = cur.clone()
            else:
                if cur.s < best.s:
                    best = cur.clone()

        it_cnt += 1


    # print("Iter:", iter_count)
    return best



# --- Output ---
def output(st):
    # 問題依存
    print("")



def solve():
    data = read_input()
    i_st = State(data)

    best = sa(i_st)

    output(best)



if __name__ == "__main__":
    solve()