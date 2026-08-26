#!/bin/bash

# 定義遠端裝置資訊
# 格式："地名|IP|遠端帳號|遠端GAME路徑|本地掛載點"
MOUNTS=(
    "Lcd|192.168.1.121|lcd|/home/lcd/GAME|/home/red/MultiNode-DevHub/mounts_game/lcd"
    "Zero 2w|192.168.1.104|zero2w|/home/zero2w/GAME|/home/red/MultiNode-DevHub/mounts_game/zero2"
    "White|192.168.1.116|white|/home/white/GAME|/home/red/MultiNode-DevHub/mounts_game/white"
)

# 1. 處理本地端 Red 的 GAME 目錄
RED_GAME_PATH="/home/red/MultiNode-DevHub/mounts_game/red"
mkdir -p "$RED_GAME_PATH"

# 2. 進行遠端 SSHFS 掛載
for target in "${MOUNTS[@]}"; do
    IFS="|" read -r NAME IP USER REMOTE_PATH LOCAL_PATH <<< "$target"

    # 確保本地資料夾存在
    mkdir -p "$LOCAL_PATH"

    # 檢查是否已經掛載，若未掛載才進行 SSHFS 掛載
    if mountpoint -q "$LOCAL_PATH"; then
        echo "[$NAME] 已經掛載於 $LOCAL_PATH，跳過。"
    else
        echo "[$NAME] 正在掛載至 $LOCAL_PATH..."
        sshfs "${USER}@${IP}:${REMOTE_PATH}" "$LOCAL_PATH" -o reconnect,ServerAliveInterval=15,ServerAliveCountMax=3

        if [ $? -eq 0 ]; then
            echo "[$NAME] 掛載成功！"
        else
            echo "[$NAME] 掛載失敗，請檢查遠端是否有 /home/$USER/GAME 目錄或 SSH 連線。"
        fi
    fi
done