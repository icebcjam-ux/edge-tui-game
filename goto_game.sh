#!/bin/bash

# 自動確保 tmux 開啟滑鼠支援（可滾動、可直接用滑鼠點擊切換視窗）
if ! grep -q "set -g mouse on" ~/.tmux.conf 2>/dev/null; then
    echo "set -g mouse on" >> ~/.tmux.conf
    tmux source-file ~/.tmux.conf 2>/dev/null
fi

# 定義直向全開邏輯
spawn_vertical() {
    tmux kill-session -t game_cluster 2>/dev/null
    tmux new-session -d -s game_cluster
    tmux send-keys -t game_cluster "cd ~/MultiNode-DevHub/edge-tui-game" C-m

    tmux split-window -h -t game_cluster
    tmux send-keys -t game_cluster "cd ~/MultiNode-DevHub/mounts_game/red" C-m

    tmux split-window -h -t game_cluster
    tmux send-keys -t game_cluster "ssh -t zero2w 'cd ~/GAME && exec bash -l'" C-m

    tmux split-window -h -t game_cluster
    tmux send-keys -t game_cluster "ssh -t lcd 'cd ~/GAME && exec bash -l'" C-m

    tmux split-window -h -t game_cluster
    tmux send-keys -t game_cluster "ssh -t white 'cd ~/GAME && exec bash -l'" C-m

    tmux select-layout -t game_cluster even-horizontal
    tmux attach-session -t game_cluster
}

# 定義十字切割 (2x2 / 網格) 邏輯
spawn_tiled() {
    tmux kill-session -t game_cluster 2>/dev/null
    tmux new-session -d -s game_cluster
    tmux send-keys -t game_cluster "cd ~/MultiNode-DevHub/edge-tui-game" C-m

    tmux split-window -h -t game_cluster
    tmux send-keys -t game_cluster "ssh -t zero2w 'cd ~/GAME && exec bash -l'" C-m

    tmux split-window -v -t game_cluster:0.0
    tmux send-keys -t game_cluster "ssh -t lcd 'cd ~/GAME && exec bash -l'" C-m

    tmux split-window -v -t game_cluster:0.1
    tmux send-keys -t game_cluster "ssh -t white 'cd ~/GAME && exec bash -l'" C-m

    tmux select-layout -t game_cluster tiled
    tmux attach-session -t game_cluster

    # 關鍵修正：加 TMUX= 強制無視外層嵌套
    TMUX= tmux attach-session -t game_cluster
}

# 判斷是否直接傳入參數（例如 ./goto_game.sh zero2w）
target="$1"

# 若沒傳參數，則跳出選單讓使用者挑選
if [ -z "$target" ]; then
    PS3="請選擇要切換的設備或模式 (輸入數字): "
    options=(
        "all (十字切割)"
        "all (直向切割)"
        "red"
        "lcd"
        "white"
        "zero2w"
        "退出"
    )
    select opt in "${options[@]}"; do
        case $opt in
            "all (十字切割)") target="tiled"; break ;;
            "all (直向切割)") target="vertical"; break ;;
            "red") target="red"; break ;;
            "lcd") target="lcd"; break ;;
            "white") target="white"; break ;;
            "zero2w") target="zero2w"; break ;;
            "退出") exit 0 ;;
            *) echo "無效選項，請重新選擇。";;
        esac
    done
fi

# 執行對應動作
case "$target" in
    vertical)
        spawn_vertical
        ;;
    tiled)
        spawn_tiled
        ;;
    red)
        cd ~/MultiNode-DevHub/mounts_game/red && exec bash -l
        ;;
    lcd)
        ssh -t lcd "cd ~/GAME && exec bash -l"
        ;;
    white)
        ssh -t white "cd ~/GAME && exec bash -l"
        ;;
    zero2w)
        ssh -t zero2w "cd ~/GAME && exec bash -l"
        ;;
    *)
        echo "使用方式: $0 [選項]"
        echo "或直接執行 $0 進入互動選單"
        exit 1
        ;;
esac