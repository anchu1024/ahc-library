import sys
import time
import heapq

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
        # 必要なら浅いコピーにする
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



# --- Beam Step ---
def beam_step(beams, width):
    """
    beams: 現在の状態リスト
    width: 次のステップで保持する幅
    """
    n_st = [] # 次の状態

    for st in beams:
        for cand in gen_cand(st):
            s = calc(cand)
            n_st.append((s, cand))

    # スコアが高い順に beam_width 個だけ残す
    if MAX:
        n_st.sort(key=lambda x: -x[0])
    else:
        n_st.sort()
   
    n_beam = [st for s, st in n_st[:width]] # 次のビーム

    return n_beam



# --- Output ---
def output(st):
    # 問題依存
    print("")



# --- Main ---
def solve():
    data = read_input()

    # 初期状態
    i_st = State(data)

    # Beam の幅（問題依存）
    width = 10

    # Beam の初期化
    b = [i_st]

    # 問題依存のループ回数
    for _ in range(100):
        b = beam_step(b, width)

    # 最終的に最も良い状態を選ぶ
    st = max(b, key=lambda s: calc(s))

    output(b)



if __name__ == "__main__":
    solve()