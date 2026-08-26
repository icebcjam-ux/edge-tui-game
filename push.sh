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
git commit -m "$MSG"
git push -u origin main

echo "----------------------------------------"
echo " Success: Code & Mounts pushed to GitHub!"
echo "----------------------------------------"