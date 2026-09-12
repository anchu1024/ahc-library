import sys
import time

# --- フラグなどの設定変数 ---
MAX = True



# --- Input ---
def read_input():
    data = sys.stdin.read().strip().split()
    return data



# --- State ---
class State:
    def __init__(self, data):
        self.data = data
        # 問題依存の状態をここに


    def clone(self):
        return State(self.data)



# --- Evaluation ---
def calc(st):
    s = 0
    # 問題依存
    return s



# --- Neighbor ---
def gen_cand(st):
    cand = []
    # 問題依存
    return cand



# --- Greedy Step ---
def greedy(st):
    best_s = -10**18
    if not MAX:
        best_s = -best_s
    best = None
    for cand in gen_cand(st):
        s = calc(cand)
        if (MAX and s > best_s) or (not MAX and s < best_s):
            best_s = s
            best = cand
    return best



# --- Output ---
def output(state):
    # 問題依存
    print("")



# --- Main ---
def solve():
    data = read_input()
    st = State(data)

    # 問題依存のループ回数
    for _ in range(100):
        st = greedy(st)

    output(st)



if __name__ == "__main__":
    solve()