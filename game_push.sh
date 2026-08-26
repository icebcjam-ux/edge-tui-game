#!/bin/bash
# 自動同步編譯好的 bot_agent 到所有算力節點

# 先在本機編譯
g++ -O2 bot_agent.cpp -o bot_agent

if [ $? -eq 0 ]; then
    echo "=== 編譯成功，開始推送至各節點 ==="
    
    # 依序推送到目標主機與對應資料夾
    scp bot_agent white@white:~/GAME/
    scp bot_agent zero2w@zero2w:~/GAME/
    scp bot_agent lcd@lcd:~/GAME/

    echo "=== 所有節點推送完成 ==="
else
    echo "編譯失敗，請檢查原始碼！"
fi