#!/bin/bash

# 自動切換到 push.sh 所在的根目錄 (MultiNode-DevHub)
cd "$(dirname "$0")"

# 預設 Commit 訊息，也可自訂：./push.sh "更新說明"
MSG=${1:-"update: auto commit and push hub code and mounts"}

# 1. 自動產生/確保 edge-tui-game 的 .gitignore
mkdir -p edge-tui-game
cat << 'EOF' > edge-tui-game/.gitignore
server
bot_agent
*.o
EOF

# 2. 自動產生/確保 mounts_game 的 .gitignore
mkdir -p mounts_game
cat << 'EOF' > mounts_game/.gitignore
server
bot_agent
*.o
EOF

# 進行全域 Git 加入與提交
git add .

# 只有在真的有檔案變更時才 commit，避免產生一堆空的 commit 紀錄
if ! git diff-index --quiet HEAD --; then
    git commit -m "$MSG"
else
    echo "沒有偵測到程式碼變更，跳過 Commit。"
fi

# 嘗試 Push，如果不成功（有衝突或遠端較新）就直接停止，絕不強制蓋碼
if git push -u origin main; then
    echo "----------------------------------------"
    echo " Success: Code & Mounts pushed to GitHub!"
    echo "----------------------------------------"
else
    echo "----------------------------------------"
    echo " [警告] Push 被拒絕！請手動檢查 GitHub 與本地狀態！"
    echo "----------------------------------------"
fi