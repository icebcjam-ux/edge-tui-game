#!/bin/bash
# Usage: ./push.sh [target_name]
# 例如：
#   ./push.sh              (預設編譯並推送 bot_agent)
#   ./push.sh my_test_bot  (編譯 my_test_bot.cpp 並推送名為 my_test_bot 的執行檔)

# 1. 取得第一個參數，若沒填則預設為 "bot_agent"
TARGET_NAME="${1:-bot_agent}"
CPP_FILE="${TARGET_NAME}.cpp"

echo "=== 準備處理標的：${TARGET_NAME} (${CPP_FILE}) ==="

# 檢查 .cpp 原始碼檔是否存在
if [ ! -f "$CPP_FILE" ]; then
    echo "錯誤：找不到原始檔 ${CPP_FILE}！"
    exit 1
fi

# 2. 定義目標節點清單 (格式: "user@hostname")
NODES=(
    "lcd@lcd"
    "white@white"
    "zero2w@zero2w"
)

# 3. 逐一推送到各節點並觸發遠端本地編譯 (避免 64-bit / 32-bit Exec format error)
for NODE in "${NODES[@]}"; do
    echo "----------------------------------------"
    echo "正在傳送原始碼與標頭檔至 ${NODE} ..."
    
    # 同步原始碼與協定檔到遠端 ~/GAME/ 資料夾
    scp "$CPP_FILE" protocol.h "${NODE}:~/GAME/"
    
    if [ $? -eq 0 ]; then
        echo "傳送成功，開始讓 ${NODE} 進行本地編譯..."
        # 透過 SSH 指令，在遠端執行本地編譯
        ssh "$NODE" "g++ -O2 ~/GAME/${CPP_FILE} -o ~/GAME/${TARGET_NAME}"
        
        if [ $? -eq 0 ]; then
            echo "✅ ${NODE} 編譯完成！產出執行檔: ~/GAME/${TARGET_NAME}"
        else
            echo "❌ ${NODE} 編譯失敗，請檢查遠端環境或語法！"
        fi
    else
        echo "❌ 傳送至 ${NODE} 失敗，請確認網路連線或 SSH 設定！"
    fi
done

echo "----------------------------------------"
echo "🎉 所有節點處理完成！"
echo "=============================================================================="
echo "專案推送與遠端自動編譯腳本 (push.sh)"

echo "【跨平台架構限制與注意事項】："
echo " 如果在 Pi 5 (ARM64 / 64-bit OS) 上編譯，產出的 binary 檔推送到 32-bit OS "
echo " 的舊款樹（例如跑 32-bit Raspbian 的 Zero W / 3B）時，執行可能會跳出："
echo " \"Exec format error\""
echo ""
echo " 【最佳解決方案】："
echo " 直接推送原始碼（.cpp / .h），並透過 SSH command 讓各節點在本地自行用 gcc/g++ 進行編譯！"
echo " =============================================================================="