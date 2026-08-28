#!/bin/bash

# 定義遠端裝置資訊
# 格式: "地名|IP|遠端帳號|遠端GAME路徑|本地掛載點"
MOUNTS=(
    "Lcd|192.168.1.121|lcd|/home/lcd/GAME|/home/red/MultiNode-DevHub/mounts_game/lcdGAME"
    "Zero 2w|192.168.1.104|zero2w|/home/zero2w/GAME|/home/red/MultiNode-DevHub/mounts_game/zero2wGAME"
    "White|192.168.1.116|white|/home/white/GAME|/home/red/MultiNode-DevHub/mounts_game/whiteGAME"
)

# 1. 處理本地端 Red 的 GAME 目錄
RED_GAME_PATH="/home/red/MultiNode-DevHub/mounts_game/red"
mkdir -p "$RED_GAME_PATH"

# 2. 進行遠端 SSHFS 自動切換（已掛載則卸載，未掛載則掛載）
for target in "${MOUNTS[@]}"; do
    IFS="|" read -r NAME IP USER REMOTE_PATH LOCAL_PATH <<< "$target"

    # 確保本地資料夾存在
    mkdir -p "$LOCAL_PATH"

    if mountpoint -q "$LOCAL_PATH"; then
        # --- 已掛載：執行卸載 ---
        echo "[$NAME] 目前已掛載，正在卸載 $LOCAL_PATH..."
        fusermount -u "$LOCAL_PATH"
        
        if [ $? -eq 0 ]; then
            echo "[$NAME] 卸載成功！"
        else
            echo "[$NAME] 卸載失敗，請檢查是否有程序正在使用該目錄。"
        fi
    else
        # --- 未掛載：執行掛載 ---
        echo "[$NAME] 未掛載，正在掛載至 $LOCAL_PATH..."
        sshfs "${USER}@${IP}:${REMOTE_PATH}" "$LOCAL_PATH" -o reconnect,ServerAliveInterval=15,ServerAliveCountMax=3

        if [ $? -eq 0 ]; then
            echo "[$NAME] 掛載成功！"
        else
            echo "[$NAME] 掛載失敗，請檢查遠端是否有 $REMOTE_PATH 目錄或 SSH 連線。"
        fi
    fi
done