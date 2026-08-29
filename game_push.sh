
#!/bin/bash
# Usage: ./game_push.sh <檔名1> [檔名2 ...]
# 例如：
#   ./game_push.sh bot_agent             (直接推送編譯好的 binary)
#   ./game_push.sh bot_agent.cpp protocol.h (推送指定的多個原始碼檔案)
#   要進入 edge-tui-game 資料夾後再執行此腳本，確保檔案路徑要進入
#   指令式../game_push.sh bot_agent

# 1. 檢查是否帶入必要參數
if [ $# -eq 0 ]; then
    echo "❌ 錯誤：未指定要推送的檔案！"
    echo "💡 用法：./game_push.sh <檔名1> [檔名2 ...]"
    echo "   範例：./game_push.sh bot_agent"
    exit 1
fi

# 2. 定義目標節點清單
NODES=(
    "lcd@lcd"
    "white@white"
    "zero2w@zero2w"
)

# 3. 逐一推送到各節點
for NODE in "${NODES[@]}"; do
    echo "----------------------------------------"
    echo "正在傳送 $@ 至 ${NODE}:~/GAME/ ..."
    
    # $@ 代表傳入的所有參數（可一次傳多個檔案）
    scp "$@" "${NODE}:~/GAME/"
    
    if [ $? -eq 0 ]; then
        echo "✅ ${NODE} 傳送成功！"
    else
        echo "❌ 傳送至 ${NODE} 失敗，請檢查網路連線或 SSH 設定！"
        
    fi
done

echo "----------------------------------------"
echo "🎉 所有節點處理完成！"

#推送時注意事項：
# ==============================================================================
# 專案推送與遠端自動編譯腳本 (game_push.sh)
#
# 【跨平台架構限制與注意事項】：
# 如果在 Pi 5 (ARM64 / 64-bit OS) 上編譯，產出的 binary 檔推送到 32-bit OS 
# 的舊款樹（例如跑 32-bit Raspbian 的 Zero W / 3B）時，執行可能會跳出：
# "Exec format error"
# 
# 【最佳解決方案】：
# 直接推送原始碼（.cpp / .h），並透過 SSH command 讓各節點在本地自行用 gcc/g++ 進行編譯！
# ==============================================================================
# 錯誤訊息範例：
#  scp: stat local "xxx": No such file or directory
# echo "❌ ${NODE} 正確的路徑是：cd edge-tui-game"
# echo "❌ ${NODE} 在執行指令：../game_push.sh bot_agent"
# ==============================================================================