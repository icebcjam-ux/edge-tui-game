#!/bin/bash

spawn_tiled() {
    tmux kill-session -t game_cluster 2>/dev/null
    tmux new-session -d -s game_cluster

    # 切分並套用 1:2 比例 (左 33%, 右 67%)
    tmux split-window -h -t game_cluster
    tmux set-window-option -t game_cluster main-pane-width 33%
    tmux select-layout -t game_cluster main-vertical

    # 指令發送
    tmux send-keys -t game_cluster.0 "cd ~/MultiNode-DevHub/edge-tui-game" C-m
    tmux send-keys -t game_cluster.1 "cd ~/MultiNode-DevHub/mounts_game/red" C-m

    # 關鍵修正：加 TMUX= 強制無視外層嵌套
    TMUX= tmux attach-session -t game_cluster
}

# 選單邏輯
echo "1) all (十字切割)"
echo "2) all (直向切割)"
echo "3) red"
echo "4) lcd"
echo "5) white"
echo "6) zero2w"
echo "7) 退出"
read -p "請選擇要切換的設備或模式 (輸入數字): " choice

case $choice in
    1) spawn_tiled ;;
    7) exit 0 ;;
    *) echo "無效選項" ;;
esac