int cols, rows;
getmaxyx(stdstd, rows, cols); // ncurses 取得尺寸
bool is_phone = (cols < 50);
int col_spacing = is_phone ? 8 : 16; // 手機縮小欄位間距
