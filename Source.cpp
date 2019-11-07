#include <graphics.h>
#include <ctime>
#include <conio.h>
#include <iostream>
#include <mmsystem.h>
#include <Windows.h>
#include <fstream>

#pragma comment(lib, "Winmm.lib")
using namespace std;
const int delta[24][2] = { { 1,1 },{ 0,1 },{ -1,1 },{ -1,0 },
{ -1,-1 },{ 0,-1 },{ 1,-1 },{ 1,0 },
{ 2,0 },{ 2,1 },{ 2,2 },{ 1,2 },
{ 0,2 },{ -1,2 },{ -2,2 },{ -2,1 },
{ -2,0 },{ -2,-1 },{ -2,-2 },{ -1,-2 },
{ 0,-2 },{ 1,-2 },{ 2,-2 },{ 2,-1 } };

int currBotColor, usercolor;
int gridInfo[7][7] = { 0 }; // 先x后y，记录棋盘状态
IMAGE img[8];				// 保存图片
int blackPieceCount = 0, whitePieceCount = 0;
#define WINDOWS_X 260	//窗口大小X
#define WINDOWS_Y 260	//窗口大小Y
#define BBLACK 60		//空格大小
int load = 0;

void print(void)	// 画棋盘
{
	int x, y;
	for (x = 0; x < 7; x++)
		for (y = 0; y < 7; y++)
			switch (gridInfo[x][y])
			{
			case 0:
				putimage(37 * y, 37 * x, &img[0]);
				break;
			case 1:
				putimage(37 * y, 37 * x, &img[1]);
				blackPieceCount++;
				break;
			case -1:
				putimage(37 * y, 37 * x, &img[2]);
				whitePieceCount++;
				break;
			}

}
int inMap(int x, int y)
{
	if (x < 0 || x > 6 || y < 0 || y > 6)
		return 0;
	return 1;
}

// 向Direction方向改动坐标，并返回是否越界
int MoveStep(int &x, int &y, int Direction)
{
	x = x + delta[Direction][0];
	y = y + delta[Direction][1];
	return inMap(x, y);
}

// 在坐标处落子，检查是否合法并膜拟落子
bool ProcStep(int x0, int y0, int x1, int y1, int color)
{
	if (color == 0)
		return false;
	if (x1 == -1) // 无路可走，跳过此回合
		return true;
	if (!inMap(x0, y0) || !inMap(x1, y1)) // 超出边界
		return false;
	if (gridInfo[x0][y0] != color)
		return false;
	int dx, dy, x, y, currCount = 0, dir;
	int effectivePoints[8][2];
	dx = abs((x0 - x1)), dy = abs((y0 - y1));
	if ((dx == 0 && dy == 0) || dx > 2 || dy > 2) // 保证不会移动到原来位置，而且移动始终在5×5区域内
		return false;
	if (gridInfo[x1][y1] != 0) // 保证移动到的位置为空
		return false;
	if (dx == 2 || dy == 2) // 如果走的是5×5的外围，则不是复制粘贴
		gridInfo[x0][y0] = 0;
	else
	{
		if (color == 1)
			blackPieceCount++;
		else
			whitePieceCount++;
	}

	gridInfo[x1][y1] = color;
	for (dir = 0; dir < 8; dir++) // 影响邻近8个位置
	{
		x = x1 + delta[dir][0];
		y = y1 + delta[dir][1];
		if (!inMap(x, y))
			continue;
		if (gridInfo[x][y] == -color)
		{
			effectivePoints[currCount][0] = x;
			effectivePoints[currCount][1] = y;
			currCount++;
			gridInfo[x][y] = color;
		}
	}
	if (currCount != 0)
	{
		if (color == 1)
		{
			blackPieceCount += currCount;
			whitePieceCount -= currCount;
		}
		else
		{
			whitePieceCount += currCount;
			blackPieceCount -= currCount;
		}
	}
	return true;
}


bool end(void)	// 弹出对话框
{
	HWND wnd = GetHWnd();
	int key;
	load = 0;
	char str[50];
	if (blackPieceCount == whitePieceCount)
		strcpy_s(str, "Tie\n Again?");
	else if (blackPieceCount > whitePieceCount)
		strcpy_s(str, "Black Wins\n Again?");
	else
		strcpy_s(str, "White Wins\n Again?");
	if (blackPieceCount == whitePieceCount)
		key = MessageBox(wnd, str, "Tie", MB_YESNO | MB_ICONQUESTION);
	else if (blackPieceCount > whitePieceCount)
		key = MessageBox(wnd,str,"Black Wins",MB_YESNO | MB_ICONQUESTION);
	else
		key = MessageBox(wnd, str, "White Wins", MB_YESNO | MB_ICONQUESTION);
	if (key == IDYES)//再来就再来呗
		return true;
	else
		return false;
}
bool begin(void)	// 开始选棋子颜色用
{
	HWND wnd = GetHWnd();
	int key;
	char str[50];
	strcpy_s(str, "Do You want to Be Black?");
	key= MessageBox(wnd, str, "选择颜色", MB_YESNO | MB_ICONQUESTION);
	if (key == IDYES)
		return true;
	else
		return false;
}
//接下来为botzone时间
int CanMove(int color)
{
	int x0 = 0, x1 = 0, y0 = 0, y1 = 0, dir = 0;
	int posCount = 0;
	for (y0 = 0; y0 < 7; y0++)
	{
		for (x0 = 0; x0 < 7; x0++)
		{
			if (gridInfo[x0][y0] != color)
				continue;
			for (dir = 0; dir < 24; dir++)
			{
				x1 = x0 + delta[dir][0];
				y1 = y0 + delta[dir][1];
				if (!inMap(x1, y1))
					continue;
				if (gridInfo[x1][y1] != 0)
					continue;
				posCount++;
			}
		}
	}
	if (posCount)
		return 1;
	else
		return 0;
}
int Evaluate()
{//评估函数
	double a = 0;
	int posCount = 0, stable = 0;
	for (int y0 = 0; y0 < 7; y0++)
	{
		for (int x0 = 0; x0 < 7; x0++)
		{
			int count = 0;
			if (gridInfo[x0][y0] != currBotColor)
				continue;
			for (int dir = 0; dir < 24; dir++)
			{
				int x1 = x0 + delta[dir][0];
				int y1 = y0 + delta[dir][1];
				if (!inMap(x1, y1))
					continue;
				if (gridInfo[x1][y1] != 0)
					continue;
				if (dir < 8)
					count++;
				posCount++;
			}
			if (count == 0)
				stable++;
		}
	}
	if (currBotColor == -1)
		a = ((whitePieceCount - blackPieceCount) + posCount*0.1 + stable * 1.5);
	else
		a = ((blackPieceCount - whitePieceCount) + posCount*0.1 + stable * 1.5);
	return a;
}
int min1(int depth, int alpha, int beta);
int max1(int depth, int alpha, int beta)
{//极大极小搜索
	if (depth <= 0)
		return Evaluate();
	int x0 = 0, x1 = 0, y0 = 0, y1 = 0, dir = 0;
	int beginPos[1000][2] = { 0 }, possiblePos[1000][2] = { 0 };
	int posCount = 0;
	for (y0 = 0; y0 < 7; y0++)
	{
		for (x0 = 0; x0 < 7; x0++)
		{
			if (gridInfo[x0][y0] != currBotColor)
				continue;
			for (dir = 0; dir < 24; dir++)
			{
				x1 = x0 + delta[dir][0];
				y1 = y0 + delta[dir][1];
				if (!inMap(x1, y1))
					continue;
				if (gridInfo[x1][y1] != 0)
					continue;
				beginPos[posCount][0] = x0;
				beginPos[posCount][1] = y0;
				possiblePos[posCount][0] = x1;
				possiblePos[posCount][1] = y1;
				posCount++;
			}
		}
	}
	if (posCount == 0)
		if (!CanMove(-currBotColor))
			return Evaluate();
	int val = 0;
	int temp1[7][7], temp2 = blackPieceCount, temp3 = whitePieceCount;
	for (int i = 0; i < 7; i++)
		for (int j = 0; j < 7; j++)
			temp1[i][j] = gridInfo[i][j];
	for (int j = 0; j < posCount; j++)
	{
		if (ProcStep(beginPos[j][0], beginPos[j][1], possiblePos[j][0], possiblePos[j][1], currBotColor))
			val = min1(depth - 1, alpha, beta);
		if (val > alpha)
		{
			if (val >= beta)
				return val;
			alpha = val;
		}
		for (int i = 0; i < 7; i++)
			for (int j = 0; j < 7; j++)
				gridInfo[i][j] = temp1[i][j];
		blackPieceCount = temp2;
		whitePieceCount = temp3;
	}
	return alpha;
}
int min1(int depth, int alpha, int beta)
{
	if (depth <= 0)
		return Evaluate();
	int x0 = 0, x1 = 0, y0 = 0, y1 = 0, dir = 0;
	int beginPos[1000][2] = { 0 }, possiblePos[1000][2] = { 0 };
	int posCount = 0;
	for (y0 = 0; y0 < 7; y0++)
	{
		for (x0 = 0; x0 < 7; x0++)
		{
			if (gridInfo[x0][y0] != -currBotColor)
				continue;
			for (dir = 0; dir < 24; dir++)
			{
				x1 = x0 + delta[dir][0];
				y1 = y0 + delta[dir][1];
				if (!inMap(x1, y1))
					continue;
				if (gridInfo[x1][y1] != 0)
					continue;
				beginPos[posCount][0] = x0;
				beginPos[posCount][1] = y0;
				possiblePos[posCount][0] = x1;
				possiblePos[posCount][1] = y1;
				posCount++;
			}
		}
	}
	if (posCount == 0)
		if (!CanMove(-currBotColor))
			return Evaluate();
	int val = 0;
	int temp1[7][7], temp2 = blackPieceCount, temp3 = whitePieceCount;
	for (int i = 0; i < 7; i++)
		for (int j = 0; j < 7; j++)
			temp1[i][j] = gridInfo[i][j];
	for (int j = 0; j < posCount; j++)
	{
		if (ProcStep(beginPos[j][0], beginPos[j][1], possiblePos[j][0], possiblePos[j][1], -currBotColor))
			val = max1(depth - 1, alpha, beta);
		if (val < beta)
		{
			if (val <= alpha)
				return val;
			beta = val;
		}
		for (int i = 0; i < 7; i++)
			for (int j = 0; j < 7; j++)
				gridInfo[i][j] = temp1[i][j];
		blackPieceCount = temp2;
		whitePieceCount = temp3;
	}
	return beta;
}
int best_move = 0;
int best_val = -1111111;
int minmax(int depth, int alpha, int beta)
{
	int x0 = 0, x1 = 0, y0 = 0, y1 = 0, dir = 0;
	int beginPos[1000][2] = { 0 }, possiblePos[1000][2] = { 0 };
	int posCount = 0;
	for (y0 = 0; y0 < 7; y0++)
	{
		for (x0 = 0; x0 < 7; x0++)
		{
			if (gridInfo[x0][y0] != currBotColor)
				continue;
			for (dir = 0; dir < 24; dir++)
			{
				x1 = x0 + delta[dir][0];
				y1 = y0 + delta[dir][1];
				if (!inMap(x1, y1))
					continue;
				if (gridInfo[x1][y1] != 0)
					continue;
				beginPos[posCount][0] = x0;
				beginPos[posCount][1] = y0;
				possiblePos[posCount][0] = x1;
				possiblePos[posCount][1] = y1;
				posCount++;
			}
		}
	}
	int val = 0;
	int temp1[7][7], temp2 = blackPieceCount, temp3 = whitePieceCount;
	for (int i = 0; i < 7; i++)
		for (int j = 0; j < 7; j++)
			temp1[i][j] = gridInfo[i][j];
	for (int j = 0; j < posCount; j++)
	{
		if (ProcStep(beginPos[j][0], beginPos[j][1], possiblePos[j][0], possiblePos[j][1], currBotColor))
			val = min1(depth - 1, alpha, beta);
		if (val > alpha)
		{
			if (val >= beta)
				return val;
			alpha = val;
			best_move = j;
		}
		for (int i = 0; i < 7; i++)
			for (int j = 0; j < 7; j++)
				gridInfo[i][j] = temp1[i][j];
		blackPieceCount = temp2;
		whitePieceCount = temp3;
	}
	return best_move;
}
//botzone部分结束
void save()
{
	ofstream fout("save/save");
	fout << usercolor << ' ';
	for (int i = 0; i < 7; i++)
		for (int j = 0; j < 7; j++)
			if (gridInfo[i][j] == usercolor)
				fout << i << ' ' << j << ' ';
	fout << 8<<' '<<8<<' ';
	fout <<currBotColor << ' ';
	for (int i = 0; i < 7; i++)
		for (int j = 0; j < 7; j++)
			if (gridInfo[i][j] == currBotColor)
				fout << i << ' ' << j << ' ';
	fout.close();
}
void print1(int x, int y)
{
	int dir = 0;
	for (dir = 0; dir < 24; dir++)
	{
		int x1 = x + delta[dir][0], y1 = y + delta[dir][1];
		if (!inMap(x1, y1))
			continue;
		if (gridInfo[x1][y1] != 0)
			continue;
		if (dir < 8)
			putimage(37 * y1, 37 * x1, &img[7]);
		else if (dir >= 8)
			putimage(37 * y1, 37 * x1, &img[6]);
	}
}
void play(void)			// 游戏过程
{
	MOUSEMSG m1, m2;
	int flag = 0;
	if (usercolor == 1)
		flag = 1;
	while (CanMove(usercolor) && CanMove(currBotColor))
	{
		int x = 0, y = 0, x2 = 0, y2 = 0;
		int startX, startY, resultX, resultY;
		if (CanMove(currBotColor) && flag == 0)
		{
			int beginPos[1000][2] = { 0 }, possiblePos[1000][2] = { 0 }, posCount = 0, choice, dir;
			int x0, y0, x1, y1;
			for (y0 = 0; y0 < 7; y0++)
			{
				for (x0 = 0; x0 < 7; x0++)
				{
					if (gridInfo[x0][y0] != currBotColor)
						continue;
					for (dir = 0; dir < 24; dir++)
					{
						x1 = x0 + delta[dir][0];
						y1 = y0 + delta[dir][1];
						if (!inMap(x1, y1))
							continue;
						if (gridInfo[x1][y1] != 0)
							continue;
						beginPos[posCount][0] = x0;
						beginPos[posCount][1] = y0;
						possiblePos[posCount][0] = x1;
						possiblePos[posCount][1] = y1;
						posCount++;
					}
				}
			}
			minmax(4, -1000000, 10000);
			startX = beginPos[best_move][0];
			startY = beginPos[best_move][1];
			resultX = possiblePos[best_move][0];
			resultY = possiblePos[best_move][1];
			Sleep(299);
			if (currBotColor == 1)
				putimage(startY * 37, startX * 37, &img[3]);
			else
				putimage(startY * 37, startX * 37, &img[4]);
			print1(startX, startY);
			Sleep(1000);
			ProcStep(startX, startY, resultX, resultY, currBotColor);
			mciSendString("play  音乐\\下子.wma from 0", NULL, 0, NULL);
			print();
			flag = 1;
			continue;
		}
		if (CanMove(usercolor) && flag == 1)										// 如果玩家有下子位置							
		{
			while (true)
			{
				m1 = GetMouseMsg();							// 获取鼠标消息
				if (m1.mkLButton)
				{
					x = (m1.y) / 37;
					y = (m1.x) / 37;
					FlushMouseMsgBuffer();
					if (gridInfo[x][y] == usercolor)							// 如果当前位置有效
					{
						print1(x, y);
						if (usercolor == 1)
							putimage(37 * y, 37 * x, &img[3]);//标识选取点
						else
							putimage(37 * y, 37 * x, &img[4]);
						while (true)
						{
							m2 = GetMouseMsg();
							if (m2.mkLButton)
							{
								x2 = (m2.y) / 37;
								y2 = (m2.x) / 37;
								if (ProcStep(x, y, x2, y2, usercolor))
								{
									mciSendString("play  音乐\\下子.wma from 0", NULL, 0, NULL);
									print();
									flag = 0;
									break;
									//break;
								}
							}
							else if (m2.mkRButton)
							{
								print();
								if (usercolor == 1)
								{
									putimage(37 * y, 37 * x, &img[1]);//取消标识
									putimage(37 * resultY, 37 * resultX, &img[2]);
								}
								else
								{
									putimage(37 * y, 37 * x, &img[2]);
									putimage(37 * resultY, 37 * resultX, &img[1]);
								}
								break;
							}
						}
						if (flag == 1)
							continue;
						else
							break;
					}
					else if (m1.y > 260 || m1.x > 260)
					{
						HWND wnd1 = GetHWnd();
						int key;
						key = MessageBox(wnd1, "存盘吗？", "咦？", MB_YESNO | MB_ICONQUESTION);
						if (key == IDYES)
							save();
						else
							continue;
						HWND wnd2 = GetHWnd();
						int key1;
						key1 = MessageBox(wnd2, "继续嘛？", "存盘结束！", MB_YESNO | MB_ICONQUESTION);
						if (key1 == IDYES)
							continue;
						else
							exit(1);
					}
					else
						continue;
				}
			}
		}
	}
	if (CanMove(usercolor))
	{
		if (usercolor == 1)
			blackPieceCount += (49 - whitePieceCount - blackPieceCount);
		else
			whitePieceCount += (49 - whitePieceCount - blackPieceCount);
	 }
	else if (CanMove(currBotColor))
	{
		if (currBotColor == 1)
			blackPieceCount += (49 - whitePieceCount - blackPieceCount);
		else
			whitePieceCount += (49 - whitePieceCount - blackPieceCount);
	}
	if (usercolor == 1)
	{
		if (blackPieceCount > whitePieceCount)
			mciSendString("play  音乐\\胜利.wma from 0", NULL, 0, NULL);
		else if (blackPieceCount < whitePieceCount)
			mciSendString("play  音乐\\失败.wma from 0", NULL, 0, NULL);
	}
	else
	{
		if (blackPieceCount < whitePieceCount)
			mciSendString("play  音乐\\胜利.wma from 0", NULL, 0, NULL);
		else if (blackPieceCount > whitePieceCount)
			mciSendString("play  音乐\\失败.wma from 0", NULL, 0, NULL);
	}
}
void menu()
{
	IMAGE background;
	loadimage(&background, "图片\\蛤.jpg");
	putimage(0, 0, &background);
	MOUSEMSG m1;
	RECT r1 = { WINDOWS_X / 2 - BBLACK,WINDOWS_Y / 3+30,WINDOWS_X / 2 + BBLACK ,WINDOWS_Y / 3 + BBLACK / 2+30 };
	rectangle(WINDOWS_X / 2 - BBLACK, WINDOWS_Y / 3+30, WINDOWS_X / 2 + BBLACK, WINDOWS_Y / 3 + BBLACK / 2+30);
	drawtext("新游戏", &r1, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

	RECT r2 = { WINDOWS_X / 2 - BBLACK,WINDOWS_Y / 3 + BBLACK / 2 + 40,WINDOWS_X / 2 + BBLACK,WINDOWS_Y / 3 + BBLACK + 40 };
	rectangle(WINDOWS_X / 2 - BBLACK, WINDOWS_Y / 3 + BBLACK / 2 + 40, WINDOWS_X / 2 + BBLACK, WINDOWS_Y / 3 + BBLACK + 40);
	drawtext("载入游戏", &r2, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

	RECT r3 = { WINDOWS_X / 2 - BBLACK,WINDOWS_Y / 3 + BBLACK + 50,WINDOWS_X / 2 + BBLACK,(int)(WINDOWS_Y / 3 + BBLACK*1.5 + 50) };
	rectangle(WINDOWS_X / 2 - BBLACK, WINDOWS_Y / 3 + BBLACK + 50, WINDOWS_X / 2 + BBLACK, (int)(WINDOWS_Y / 3 + BBLACK*1.5 + 50));
	drawtext("退出游戏", &r3, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	while (true)
	{
		m1 = GetMouseMsg();							// 获取鼠标消息
		if (m1.mkLButton)
		{
			if (m1.x > WINDOWS_X / 2 - BBLACK && m1.x<WINDOWS_X / 2 + BBLACK && m1.y>WINDOWS_Y / 3 && m1.y < WINDOWS_Y / 3 + BBLACK / 2)
				return;
			else if (m1.x > WINDOWS_X / 2 - BBLACK && m1.x<WINDOWS_X / 2 + BBLACK && m1.y>WINDOWS_Y / 3 + BBLACK / 2 + 10 && m1.y < WINDOWS_Y / 3 + BBLACK + 10)
			{
				ifstream fin("save/save");
				if (!fin)
				{
					HWND wnd = GetHWnd();
					MessageBox(wnd,"目前没有存盘可以继续哦QAQ ", "载入失败", MB_OK | MB_ICONHAND);
					continue;
				}
				fin >> usercolor;
				int x = 0, y = 0;
				while (x != 8 && y != 8)
				{
					fin >> x >> y;
					gridInfo[x][y] = usercolor;
				}
				fin >> currBotColor;
				while (fin >> x >> y)
					gridInfo[x][y] = currBotColor;
				load = 1;
				fin.close();
				return;
			}
			else if (m1.x > WINDOWS_X / 2 - BBLACK && m1.x<WINDOWS_X / 2 + BBLACK && m1.y>WINDOWS_Y / 3 + BBLACK*1.5 / 2 + 20 && m1.y < WINDOWS_Y / 3 + BBLACK*1.5 + 20)
				exit(1);
		}
	}
}
// 主函数
void prime()
{
	//初始化棋盘
	for (int i = 0; i < 7; i++)
		for (int j = 0; j < 7; j++)
			gridInfo[i][j] = 0;
	gridInfo[0][0] = gridInfo[6][6] = 1;
	gridInfo[6][0] = gridInfo[0][6] = -1;
}
int main()
{
	// 加载图片
	loadimage(&img[0], "图片\\空位.bmp");
	loadimage(&img[1], "图片\\黑子.bmp");
	loadimage(&img[2], "图片\\白子.bmp");
	loadimage(&img[3], "图片\\黑子1.bmp");
	loadimage(&img[4], "图片\\白子1.bmp");
	loadimage(&img[5], "图片\\存盘.bmp");
	loadimage(&img[6], "图片\\可行1.bmp");
	loadimage(&img[7], "图片\\可行2.bmp");

	 //加载音乐
	mciSendString("open 音乐\\背景音乐.wma", NULL, 0, NULL);
	mciSendString("open 音乐\\和局.wma", NULL, 0, NULL);
	mciSendString("open 音乐\\胜利.wma", NULL, 0, NULL);
	mciSendString("open 音乐\\失败.wma", NULL, 0, NULL);
	mciSendString("open 音乐\\下子.wma", NULL, 0, NULL);

	// 初始化棋盘
	initgraph(260, 305);
	IMAGE board;
	loadimage(&board, "图片\\棋盘.png");
	mciSendString("play 音乐\\背景音乐.mp3 from 5 repeat", NULL, 0, NULL);
	menu();
	do
	{
		putimage(0,260, &img[5]);
		SetWindowText(GetHWnd(), "同化棋");
		if (!load)
		{
			if (begin())
				usercolor = 1;
			else
				usercolor = -1;
			currBotColor = -usercolor;
			prime();
			print();
		}
		else
			print();
		play();
	} while (end());
	// 关闭音乐
	mciSendString("close  音乐\\背景音乐.mp3", NULL, 0, NULL);
	mciSendString("close  音乐\\和局.wma", NULL, 0, NULL);
	mciSendString("close  音乐\\胜利.wma", NULL, 0, NULL);
	mciSendString("close  音乐\\失败.wma", NULL, 0, NULL);
	mciSendString("close  音乐\\下子.wma", NULL, 0, NULL);
	closegraph();
	return 0;
}
