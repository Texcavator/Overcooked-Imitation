#include <windows.h>
#include <sstream> 
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <vector>
#include <string>
#include <mmsystem.h>
#include <cstdlib>
#include <conio.h>
#include <thread>
#include <dshow.h>
#include <commctrl.h>
#include <map>

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "msimg32.lib")
#pragma comment(lib, "strmiids.lib")
#pragma comment(lib, "quartz.lib")
#pragma comment(lib, "comctl32.lib")

STARTUPINFO si;

const char* ffmpeg_cmd = "D:\\ffmpeg-7.0.1-full_build\\bin\\ffmpeg.exe -f gdigrab -i desktop -pix_fmt yuv420p -y out.mp4";

PROCESS_INFORMATION pi;

// 创建管道用于通信
HANDLE hPipeRead, hPipeWrite;
SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };


// 界面
#define FIRST_COVER 0	// 封面
#define MENU 1			// 菜单
#define GAME_GOING 2	// 游戏中
#define GAME_OVER 3		// 游戏结算
#define MAP_CREATE 4	// 地图编辑

// 方向
#define Q 0
#define H 1
#define Z 2
#define Y 3

#define PII std::pair<int, int>		

// 动作
#define STAND 0
#define GO 1
#define CUT 2

// 屏幕大小
#define BIG 2048
#define SMALL 1440

// 游戏状态
#define PRE1 0		// 菜谱
#define PRE2 1		// 预备
#define PRE3 2		// 开始
#define GOING 3		// 正在进行
#define OVER 4		// 时间到

//全局变量声明
HDC		hdc, mdc, bufdc;
HWND	hwnd;
RECT	clientRect;
DWORD	tPre, tNow, last_time, cur_time, t1, t2, start_time;
HPEN hp;
HBRUSH hbr;
double x, y; // 鼠标实时位置

HDC		hdcmem;
HBITMAP hbm_bk[6], hbm_per[2][4][3], hbm_zj[6], hbm_ml[2], hbm_num[10], hbm_table[2], hbm_menu[7], hbm_click[8], hbm_plate, hbm_src[3], hbm_food[3][2],
	hbm_knife, hbm_error[3], hbm_success, hbm_lose, hbm_numres[10], hbm_gameover, hbm_star, hbm_num_s[10], hbm_bt[2][2], hbm_mc[5], hbm_cancel,
	hbm_map_start[2], hbm_dbl, hbm_cook[10], hbm_per_sle[2];

PII sz_per[2][4][3] = { {{{140, 180}, {140, 180}, {140, 180}},
	{{140, 180}, {140, 180}, {140, 180}},
	{{140, 180}, {140, 180}, {140, 180}},
	{{140, 180}, {140, 180}, {140, 180}}}, 
	{{{140, 180}, {140, 180}, {140, 180}},
	{{140, 180}, {140, 180}, {140, 180}},
	{{140, 180}, {140, 180}, {140, 180}},
	{{140, 180}, {140, 180}, {140, 180}}} };
PII sz_bk[6] = { {2160, 1214}, {100, 100}, {820, 1330}, {507, 1300}, {1448, 234} };
PII sz_zj[6] = { {1492, 903}, {599, 244}, {478, 288}, {380, 131}, {363, 136}, {756, 220} };
PII sz_ml[2] = { {140, 165}, {139, 168} };
PII sz_num[10] = { {65, 87}, {67, 87}, {68, 87}, {66, 87}, {63, 87}, {67, 87}, {57, 87}, {64, 87}, {60, 87}, {62, 87} };
PII sz_menu[7] = { {2160, 1214}, {2160, 1214}, {2160, 1214}, {2160, 1214}, {2160, 1214}, {2160, 1214}, {2160, 1214} };
PII sz_click[8] = { {316, 142}, {280, 108}, {260, 109}, {242, 103}, {224, 103}, {224, 472}, {2148, 490}, {364, 56} };
PII pos_click[8] = { {293, 120}, {508, 148}, {704, 158}, {884, 172}, {1048, 184}, {1048, 204}, {0, 310}, {0, 827} };
PII sz_plate = { 64, 64 };
PII sz_src[3] = { {0, 0}, { 40, 40 }, {40, 40} };
PII sz_food[3][2] = { {{0, 0}, {0, 0}}, {{40, 40}, {40, 40}}, {{40, 40}, {40, 40}} };
PII sz_knife = { 35, 50 };
PII sz_error[3] = { {121, 35}, {119, 38}, {197, 38} };
PII sz_success = { 122, 55 };
PII sz_lose = { 119, 59 };
PII sz_numres[10] = { {25, 34}, {23, 34}, {25, 34}, {25, 34}, {25, 34}, {24, 34}, {25, 34}, {24, 34}, {25, 34}, {26, 34} };
PII sz_gameover = { 2160, 1215 };
PII sz_star = { 100, 100 };
PII sz_num_s[10] = { {65, 92}, {54, 92}, {60, 92}, {62, 92}, {65, 92}, {64, 92}, {57, 92}, {61, 92}, {57, 92}, {57, 92} };
PII sz_bt[2] = { {299, 128}, {278, 128} };
PII sz_mc[5] = { {1440, 573}, {215, 251}, {215, 251}, {112, 56}, {112, 56} };
PII sz_cancel = { 70, 70 };
PII sz_map_start[2] = { {254, 114}, {254, 114} };
PII sz_dbl = { 106, 122 };
PII sz_cook[10] = { {1440, 573}, {132, 83}, {163, 88} };
PII sz_per_sle[2] = { {400, 360}, {400, 360} };

// 方块内是否有桌子
bool g_lst[4][9][13] = { { {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1}} };
// 方块内是否有盘子 有几个
int pt_lst[4][9][13] = { {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}} };
// 方块内是否有材料箱 什么材料
int box_lst [4] [9] [13] = { {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}} };
// 方块内是否有垃圾桶
bool trash_lst[4][9][13] = { {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}} };
// 方块内是否有传送带 哪个方向 1234前后左右
int belt_lst[4][9][13] = { {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}} };
// 方块内是否有碗柜
bool cupboard_lst[4][9][13] = { {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}} };
// 方块内是否有食物 0没拿 1生鱼2生虾 3熟鱼4熟虾 5装盘熟鱼6装盘熟虾
int food_lst [4] [9] [13] = { {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}} };
// 方块内是否有切菜台 是否被使用 非0说明有 2说明在使用
int cut_lst [4] [9] [13] = { {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0}} };

// 当前正在用的地图
bool g[9][13], trash[9][13], cupboard[9][13];
int pt[9][13], box[9][13], belt[9][13], food[9][13], cut[9][13];

int st_main = FIRST_COVER;							// 现在的界面
int mapId = 0;										// 正在用的地图编号
int cnt_per = 1;									// 游戏人数
int menu_idx = 0;									// 菜单播放动画
int menu_click = -1;								// 菜单点击效果
int menu_click_st = -1;								// 菜单是否被按下
bool quit = false;									// 离开按钮是否启用
int set = -1;										// 正在设置什么：0音量 1灵敏度 >=10说明按下了
PII buttonSet[2] = { {852, 470}, {852, 550} };
int st_game = PRE1;
bool tmp_st = false;
int table_len = 69;									// 桌子长度
int cut_time_len = 3000;							// 切菜时间
int over_time;										// 结束时间

int score = 0, time_have = 180000;
int currentVolume = 500; // 当前音量
int lastState = FIRST_COVER;  // 上一个状态
int per_cut_time[2]; // 开始切菜时间
int cnt_suc = 0, cnt_los = 0;
// 0鼠标不在按钮上 1鼠标在按钮上
bool res_button_st[2] = { 0, 0 };
// 我的地图界面点击情况 0鼠标不在按钮上 1鼠标在按钮上
bool map_select_st[3] = { 0, 0, 0 };
// 12材料箱 3垃圾桶 4切菜板 5碗柜 6盘子 78910传送带 11桌子
PII mouse_createmap = {0, 0};
int mouse_cancel = false; // 地图编辑取消 0鼠标不在上面 1鼠标在上面 2鼠标点击
int map_start_st = 0; // 地图编辑界面开始游戏
bool cook_sle[2] = { false, false }; // 选择厨师的三角形

struct person					// 人物
{
	double posx, posy;			// 实时位置
	int id;						// 编号
	// 方向和抬脚
	// way:0-前 1-后 2-左 3-右
	// idx:0-站 2-左抬 3-右抬
	int way = Q;
	int idx = 0;
	int st = STAND;				// 状态
	int velocity = 30;			// 速度
	int xx, yy;					// 所在格子编号
	int food = 0;				// 手里拿了什么东西 0没拿 1生鱼2生虾 3熟鱼4熟虾 5装盘熟鱼6装盘熟虾 7盘子
	int xxn, yyn;				// 面朝的格子编号
};

struct meal						// 订单
{
	int id;						// 种类
	DWORD time_start;			// 订单开始时间
	DWORD time_len = 60000;		// 订单持续时间
	bool st;					// 订单状态
};

struct Mess						// 事件
{
	int id;						// 种类
	DWORD time_start;			// 错误制造时间
	int posx, posy;				// 错误位置
};

struct person per[2];
std::vector<struct meal> ml;
std::vector<struct Mess> message;
std::vector<DWORD> update_plate;

RECT videoRect = { 50, 50, 450, 350 }; // 视频显示区域的位置和大小

HWND hProgressBar; // 全局变量，用于存储进度条控件的句柄
IGraphBuilder* pGraphBuilder = NULL;
IMediaControl* pMediaControl = NULL;
IMediaSeeking* pMediaSeeking = NULL;
IMediaEvent* pMediaEvent = NULL;

//全局函数声明
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
void				MyPaint(HDC hdc);

// 等比例缩放函数
void IsometricScalingByRyanzi(HBITMAP& hbm, char* name, int& wei, int& hei, int hy);
// 加载位图
void LoadBitmapInit();
// 透明贴图
BOOL MyTransparentBlt2(HDC hdcDest, int nXOriginDest, int nYOriginDest, int nWidthDest, int nHeightDest, HDC hdcSrc,
	int nXOriginSrc, int nYOriginSrc, int nWidthSrc, int nHeightSrc, UINT crTransparent);
// 游戏中绘制背景
void GamePaintBackground();
// 游戏中绘制订单
void GamePaintMeal();
// 游戏中绘制分数
void GamePaintScore();
// 游戏中绘制时间
void GamePaintTime();
// 调整音量
void SetVolume(int volume);
// 播放音乐
void PlayBackgroundMusic(const char* filepath);
// 停止播放
void StopBackgroundMusic();
// 更换音乐
void ChangeMusic(const char* filepath);
// 判断是否需要更换
void ChangeGameState(int newState);
// 绘制渐变矩形
void DrawGradientRectangle(HDC mdc, int left, int top, int len);
// 绘制桌子
void DrawTable(int left, int top);
// 更新人物所在方格
void CalculatePerPos(int k);
// 绘制食物
void DrawFood(int id, int x, int y);
// 绘制人物
void DrawPerson(int i);
// 绘制盘子
void DrawPlate(int x, int y);
// 绘制材料箱
void DrawSrcBox(int id, int x, int y);
// 绘制垃圾桶
void DrawTrashcan(int x, int y);
// 绘制传送带
void DrawConveyorbelt(int way, int x, int y);
// 绘制碗柜
void DrawCupBoard(int x, int y);
// 绘制切菜台
void DrawCutBoard(int x, int y, int knife);
// 错误 - 0没切 1没装盘 2没订单
void ErrorDispose(int id, int xx, int yy);
// 成功完成
void SuccessMeal(int posx, int posy);
// 丢分
void LoseScore(int posx, int posy);
// 送餐程序处理
void DealDelivery(int id, int fd);
// 切菜过程进度条
void DrawCutProgress(int id, int start, int now);
// 切菜
void Cutting(int id, int xx, int yy);
// 绘制消息
void DrawMessage();
// 更新碗柜上干净的盘子
void UpdateNewPlate(int i, int j);
// 结算界面星星
void DrawStar(int cnt); 
// 结算界面分数
void DrawScore(int score);
// 结算界面订单数
void DrawMealCnt(int cnt1, int cnt2);
// 结算界面下方按键
void DrawResButton();
// 视频进度更新
void UpdateProgressBar();
// 视频播放
void PlayVideo(HWND hwnd, LPCWSTR filename);
// 隐藏终端
void hideConsoleWindow();
// 桌子等物品
void DrawCube();
// 物品选种边框
void DrawBlame();
// 组件栏
void DrawSub();
// 画出已有地图
void DrawCurMap();
// 预设物品定位
void DrawNotDrop();
// 取消放置
void DrawCancel();
// 初始化地图
void initMap();

//****WinMain函数，程序入口点函数***********************
int APIENTRY WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
	MSG msg;

	MyRegisterClass(hInstance);

	// 初始化
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	PlayBackgroundMusic("music/Menu_Title_Screen.mp3");

	GetMessage(&msg, NULL, NULL, NULL); // 初始化msg      
	
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));
	CreatePipe(&hPipeRead, &hPipeWrite, &sa, 0);

	si.hStdInput = hPipeRead;
	si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
	si.dwFlags |= STARTF_USESTDHANDLES;
	
	// 游戏循环
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			DWORD tNow = GetTickCount();
			static DWORD tPre = tNow;
			if (tNow - tPre >= 100)
			{
				HDC hdc = GetDC(NULL);
				MyPaint(hdc);
				ReleaseDC(NULL, hdc);
				tPre = tNow;
			}
		}
	}

	return (int)msg.wParam;
}

//****设计一个窗口类****
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wcex.lpfnWndProc = (WNDPROC)WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = NULL;
	wcex.hCursor = NULL;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = "canvas";
	wcex.hIconSm = NULL;

	return RegisterClassEx(&wcex);
}

//****初始化函数****
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hwnd = CreateWindow("canvas", "OverCooked!", WS_OVERLAPPEDWINDOW,
		0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), NULL, NULL, hInstance, NULL);
	if (!hwnd)
	{
		return FALSE;
	}

	// 移除窗口边框和标题栏
	LONG style = GetWindowLong(hwnd, GWL_STYLE);
	style &= ~(WS_OVERLAPPEDWINDOW);
	style |= WS_POPUP;
	SetWindowLong(hwnd, GWL_STYLE, style);

	// 调整窗口大小和位置，使其覆盖整个屏幕
	SetWindowPos(hwnd, HWND_TOP, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), SWP_FRAMECHANGED);

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	////////////////////////////////////////
	GetClientRect(hwnd, &clientRect);
	HBITMAP fullmap;
	hdc = GetDC(hwnd);
	mdc = CreateCompatibleDC(hdc);
	bufdc = CreateCompatibleDC(hdc);

	//建立空的位图并置入mdc中
	fullmap = CreateCompatibleBitmap(hdc, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
	SelectObject(mdc, fullmap);

	srand(time(0));

	per[0].id = 0;
	per[0].posx = 400, per[0].posy = 300;
	per[1].id = 1;
	per[1].posx = 400, per[1].posy = 500;

	//载入各连续移动位图及背景图
	LoadBitmapInit();
	
	MyPaint(hdc);

	return TRUE;
}

//****自定义绘图函数****
void MyPaint(HDC hdc)
{
	hdc = GetDC(hwnd);
	ChangeGameState(st_main);
	lastState = st_main;

	hp = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
	hbr = CreateSolidBrush(RGB(0, 0, 0));
	SelectObject(mdc, hp);
	SelectObject(mdc, hbr);
	Rectangle(mdc, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
	DeleteObject(hp);
	DeleteObject(hbr);

	if (st_main == FIRST_COVER)
	{
		SelectObject(bufdc, hbm_bk[0]);
		BitBlt(mdc, 0, 50, sz_bk[0].first, sz_bk[0].second, bufdc, 0, 0, SRCCOPY);
	}
	else if (st_main == MENU)
	{
		SelectObject(bufdc, hbm_menu[menu_idx]);
		BitBlt(mdc, 0, 50, sz_menu[menu_idx].first, sz_menu[menu_idx].second, bufdc, 0, 0, SRCCOPY);
		if (cnt_per == 2)
		{
			SelectObject(bufdc, hbm_dbl);
			MyTransparentBlt2(mdc, 1070, 609, sz_dbl.first, sz_dbl.second, bufdc, 0, 0, sz_dbl.first, sz_dbl.second, RGB(144, 118, 44));
		}
		if (menu_click != -1)
		{
			if (menu_click == 1 && (menu_click_st == -1 || menu_click_st == 1))
			{
				if (menu_click_st == 1)
				{
					SelectObject(bufdc, hbm_mc[0]);
					MyTransparentBlt2(mdc, 0, 200, sz_mc[0].first, sz_mc[0].second, bufdc, 0, 0, sz_mc[0].first, sz_mc[0].second, RGB(135, 135, 135));
					SelectObject(bufdc, hbm_click[7]);
					MyTransparentBlt2(mdc, pos_click[7].first, pos_click[7].second, sz_click[7].first, sz_click[7].second, bufdc, 0, 0, sz_click[7].first, sz_click[7].second, RGB(100, 100, 100));
				
					int tmpx = 250;
					for (int i = 0; i < 3; i++)
					{
						if (!map_select_st[i])
						{
							SelectObject(bufdc, hbm_mc[1]);
							MyTransparentBlt2(mdc, tmpx, 370, sz_mc[1].first, sz_mc[1].second, bufdc, 0, 0, sz_mc[1].first, sz_mc[1].second, RGB(168, 214, 230));
							SelectObject(bufdc, hbm_mc[3]);
							MyTransparentBlt2(mdc, tmpx + 20, 470, sz_mc[3].first, sz_mc[3].second, bufdc, 0, 0, sz_mc[3].first, sz_mc[3].second, RGB(89, 162, 223));
						}
						else
						{
							SelectObject(bufdc, hbm_mc[2]);
							MyTransparentBlt2(mdc, tmpx, 370, sz_mc[2].first, sz_mc[2].second, bufdc, 0, 0, sz_mc[2].first, sz_mc[2].second, RGB(168, 214, 230));
							SelectObject(bufdc, hbm_mc[4]);
							MyTransparentBlt2(mdc, tmpx + 20, 470, sz_mc[4].first, sz_mc[4].second, bufdc, 0, 0, sz_mc[4].first, sz_mc[4].second, RGB(225, 151, 90));
						}
						SelectObject(bufdc, hbm_num[i + 1]);
						MyTransparentBlt2(mdc, tmpx + 130, 470, sz_num[i + 1].first, sz_num[i +1].second, bufdc, 0, 0, sz_num[i + 1].first, sz_num[i + 1].second, RGB(75, 104, 136));
						tmpx += 350;
					}
				}
				else
				{
					SelectObject(bufdc, hbm_click[1]);
					MyTransparentBlt2(mdc, pos_click[1].first, pos_click[1].second, sz_click[1].first, sz_click[1].second, bufdc, 0, 0, sz_click[1].first, sz_click[1].second, RGB(100, 100, 100));
				}
			}
			else if (menu_click == 2 && (menu_click_st == -1 || menu_click_st == 2))
			{
				if (menu_click_st == 2)
				{
					SelectObject(bufdc, hbm_cook[0]);
					MyTransparentBlt2(mdc, 0, 200, sz_cook[0].first, sz_cook[0].second, bufdc, 0, 0, sz_cook[0].first, sz_cook[0].second, RGB(135, 135, 135));
					SelectObject(bufdc, hbm_click[7]);
					MyTransparentBlt2(mdc, pos_click[7].first, pos_click[7].second, sz_click[7].first, sz_click[7].second, bufdc, 0, 0, sz_click[7].first, sz_click[7].second, RGB(100, 100, 100));
					if (cnt_per == 1)
					{
						SelectObject(bufdc, hbm_cook[1]);
						BitBlt(mdc, 440, 400, sz_cook[1].first, sz_cook[1].second, bufdc, 0, 0, SRCCOPY);
						SelectObject(bufdc, hbm_per_sle[per[0].id]);
						MyTransparentBlt2(mdc, 570, 320, sz_per_sle[per[0].id].first, sz_per_sle[per[0].id].second, bufdc, 0, 0, sz_per_sle[per[0].id].first, sz_per_sle[per[0].id].second, RGB(3, 169, 244));

						POINT pt1[3] = { {540, 570}, {540, 630}, {500, 600} };
						POINT pt2[3] = { {960, 570}, {960, 630}, {1000, 600} };
						hp = CreatePen(PS_SOLID, 3, RGB(26, 89, 115));
						hbr = CreateSolidBrush(RGB(47, 159, 205));
						SelectObject(mdc, hp);
						SelectObject(mdc, hbr);
						Polygon(mdc, pt1, 3);
						Polygon(mdc, pt2, 3);
						DeleteObject(hp);
						DeleteObject(hbr);
					}
					else if (cnt_per == 2)
					{
						SelectObject(bufdc, hbm_cook[1]);
						BitBlt(mdc, 140, 400, sz_cook[1].first, sz_cook[1].second, bufdc, 0, 0, SRCCOPY);
						SelectObject(bufdc, hbm_per_sle[per[0].id]);
						MyTransparentBlt2(mdc, 270, 320, sz_per_sle[per[0].id].first, sz_per_sle[per[0].id].second, bufdc, 0, 0, sz_per_sle[per[0].id].first, sz_per_sle[per[0].id].second, RGB(3, 169, 244));

						POINT pt1[3] = { {240, 570}, {240, 630}, {200, 600} };
						POINT pt2[3] = { {660, 570}, {660, 630}, {700, 600} };
						hp = CreatePen(PS_SOLID, 3, RGB(26, 89, 115));
						hbr = CreateSolidBrush(RGB(47, 159, 205));
						SelectObject(mdc, hp);
						SelectObject(mdc, hbr);
						Polygon(mdc, pt1, 3);
						Polygon(mdc, pt2, 3);
						DeleteObject(hp);
						DeleteObject(hbr);

						SelectObject(bufdc, hbm_cook[2]);
						BitBlt(mdc, 720, 400, sz_cook[2].first, sz_cook[2].second, bufdc, 0, 0, SRCCOPY);
						SelectObject(bufdc, hbm_per_sle[per[1].id]);
						MyTransparentBlt2(mdc, 850, 320, sz_per_sle[per[1].id].first, sz_per_sle[per[1].id].second, bufdc, 0, 0, sz_per_sle[per[1].id].first, sz_per_sle[per[1].id].second, RGB(3, 169, 244));

						POINT pt3[3] = { {820, 570}, {820, 630}, {780, 600} };
						POINT pt4[3] = { {1240, 570}, {1240, 630}, {1280, 600} };
						hp = CreatePen(PS_SOLID, 3, RGB(26, 89, 115));
						hbr = CreateSolidBrush(RGB(47, 159, 205));
						SelectObject(mdc, hp);
						SelectObject(mdc, hbr);
						Polygon(mdc, pt3, 3);
						Polygon(mdc, pt4, 3);
						DeleteObject(hp);
						DeleteObject(hbr);
					}
				}
				else
				{
					SelectObject(bufdc, hbm_click[2]);
					MyTransparentBlt2(mdc, pos_click[2].first, pos_click[2].second, sz_click[2].first, sz_click[2].second, bufdc, 0, 0, sz_click[2].first, sz_click[2].second, RGB(100, 100, 100));
				}
			}
			else if (menu_click == 3 && (menu_click_st == -1 || menu_click_st == 3))
			{
				if (menu_click_st == 3)
				{
					for (int i = 6; i <= 7; i++)
					{
						SelectObject(bufdc, hbm_click[i]);
						MyTransparentBlt2(mdc, pos_click[i].first, pos_click[i].second, sz_click[i].first, sz_click[i].second, bufdc, 0, 0, sz_click[i].first, sz_click[i].second, RGB(100, 100, 100));
					}
					hp = CreatePen(PS_SOLID, 3, RGB(255, 255, 255));
					hbr = CreateSolidBrush(RGB(48, 110, 155));
					SelectObject(mdc, hp);
					SelectObject(mdc, hbr);
					RoundRect(mdc, 550, 445, 1100, 495, 50, 50);
					RoundRect(mdc, 550, 525, 1100, 575, 50, 50);
					DeleteObject(hp);
					DeleteObject(hbr);
					hp = CreatePen(PS_SOLID, 6, RGB(122, 204, 245));
					hbr = CreateSolidBrush(RGB(255, 255, 255));
					SelectObject(mdc, hp);
					SelectObject(mdc, hbr);
					Ellipse(mdc, buttonSet[0].first - 27, buttonSet[0].second - 27, buttonSet[0].first + 27, buttonSet[0].second + 27);
					Ellipse(mdc, buttonSet[1].first - 27, buttonSet[1].second - 27, buttonSet[1].first + 27, buttonSet[1].second + 27);
					DeleteObject(hp);
					DeleteObject(hbr);
				}
				else
				{
					SelectObject(bufdc, hbm_click[3]);
					MyTransparentBlt2(mdc, pos_click[3].first, pos_click[3].second, sz_click[3].first, sz_click[3].second, bufdc, 0, 0, sz_click[3].first, sz_click[3].second, RGB(100, 100, 100));
				}
			}
			else if (menu_click == 4 && menu_click_st == -1)
			{
				SelectObject(bufdc, hbm_click[5]);
				MyTransparentBlt2(mdc, pos_click[5].first, pos_click[5].second, sz_click[5].first, sz_click[5].second, bufdc, 0, 0, sz_click[5].first, sz_click[5].second, RGB(100, 100, 100));
				SelectObject(bufdc, hbm_click[4]);
				MyTransparentBlt2(mdc, pos_click[4].first, pos_click[4].second, sz_click[4].first, sz_click[4].second, bufdc, 0, 0, sz_click[4].first, sz_click[4].second, RGB(100, 100, 100));
			}
			else if (menu_click_st == -1)
			{
				SelectObject(bufdc, hbm_click[menu_click]);
				BitBlt(mdc, pos_click[menu_click].first, pos_click[menu_click].second, sz_click[menu_click].first, sz_click[menu_click].second, bufdc, 0, 0, SRCCOPY);
			}
		}
		Sleep(200);
		menu_idx = (menu_idx + 1) % 7;
	}
	else if (st_main == GAME_GOING)
	{
		for (int i = 0; i < cnt_per; i++)
		{
			CalculatePerPos(i);
		}

		t2 = GetTickCount();
		cur_time = GetTickCount();
		if (!tmp_st) t1 = t2;
		if (st_game == PRE1 && t2 - t1 >= 2000)
		{
			initMap();
			st_game = PRE2;
			tmp_st = false;
		}
		else if (st_game == PRE2 && t2 - t1 >= 2000)
		{
			st_game = PRE3;
			tmp_st = false;
		}
		else if (st_game == PRE3 && t2 - t1 >= 2000)
		{
			st_game = GOING;
			start_time = GetTickCount();
			last_time = GetTickCount();

			
			hideConsoleWindow(); // 启动程序时隐藏控制台窗口
			si.dwFlags |= STARTF_USESHOWWINDOW;
			si.wShowWindow = SW_HIDE; // 确保子进程不显示窗口

			// 启动ffmpeg进程
			CreateProcess(NULL, (LPSTR)ffmpeg_cmd, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);

			// 关闭管道读句柄
			CloseHandle(hPipeRead);
		}

		// 背景
		GamePaintBackground();

		bool stPaintPer[2] = {false, false};
		for (int i = 0; i < 9; i++)
		{
			for (int j = 0; j < 13; j++)
			{
				for (int k = 0; k < cnt_per; k++)
				{
					if (stPaintPer[k]) continue;
					if ((int)((per[k].posy + sz_per[per[k].id][per[k].way][per[k].idx].second - 190) / table_len) == i)
					{
						stPaintPer[k] = true;
						DrawPerson(k);
						if (per[k].st == CUT) Cutting(k, per[k].xxn, per[k].yyn);
					}
				}
				if (g[i][j]) DrawTable(280 + j * (table_len + 1), 190 + i * (table_len + 1));
				if (box[i][j])
				{
					DrawTable(280 + j * (table_len + 1), 190 + i * (table_len + 1));
					DrawSrcBox(box[i][j], 280 + j * (table_len + 1), 190 + i * (table_len + 1));
				}
				if (trash[i][j])
				{
					DrawTable(280 + j * (table_len + 1), 190 + i * (table_len + 1));
					DrawTrashcan(280 + j * (table_len + 1), 190 + i * (table_len + 1));
				}
				if (i > 0 && belt[i][j] && belt[i - 1][j] == belt[i][j])
				{
					DrawTable(280 + j * (table_len + 1), 190 + (i - 1) * (table_len + 1));
					DrawTable(280 + j * (table_len + 1), 190 + i * (table_len + 1));
					DrawConveyorbelt(belt[i - 1][j], 280 + j * (table_len + 1), 190 + (i - 1) * (table_len + 1));
				}
				else if (j > 0 && belt[i][j - 1] && belt[i][j] == belt[i][j - 1])
				{
					DrawTable(280 + (j - 1) * (table_len + 1), 190 + i * (table_len + 1));
					DrawTable(280 + j * (table_len + 1), 190 + i * (table_len + 1));
					DrawConveyorbelt(belt[i][j - 1], 280 + (j - 1) * (table_len + 1), 190 + i * (table_len + 1));
				}
				if (cupboard[i][j])
				{
					DrawTable(280 + j * (table_len + 1), 190 + i * (table_len + 1));
					DrawCupBoard(280 + j * (table_len + 1), 190 + i * (table_len + 1));
					UpdateNewPlate(i, j);
				}
				if (pt[i][j])
				{
					for (int z = 0; z < pt[i][j]; z++)
					{
						DrawPlate(280 + j * (table_len + 1) + 3, 190 + i * (table_len + 1) + 3 - 3 * z);
					}
				}
				if (cut[i][j])
				{
					DrawTable(280 + j * (table_len + 1), 190 + i * (table_len + 1));
					DrawCutBoard(280 + j * (table_len + 1), 190 + i * (table_len + 1), cut[i][j]);
				}
				if (food[i][j])
				{
					int tmp = food[i][j];
					if (food[i][j] >= 5 && food[i][j] <= 6)
					{
						DrawPlate(280 + j * (table_len + 1) + 3, 190 + i * (table_len + 1) + 3);
						tmp -= 2;
					}
					DrawFood(tmp, 280 + j * (table_len + 1) + table_len / 2, 190 + i * (table_len + 1) + table_len / 2);
				}
			}
		}
		
		if (st_game == GOING)
		{
			GamePaintMeal();	// 订单栏
			GamePaintScore();	// 分数
			GamePaintTime();	// 时间
			DrawMessage();		// 绘制消息
		}

		if (st_game == PRE1 || st_game == PRE2 || st_game == PRE3)
		{
			SelectObject(bufdc, hbm_zj[st_game]);
			MyTransparentBlt2(mdc, (GetSystemMetrics(SM_CXSCREEN) - sz_zj[st_game].first) / 2, (GetSystemMetrics(SM_CYSCREEN) - sz_zj[st_game].second) / 2, sz_zj[st_game].first, sz_zj[st_game].second, bufdc, 0, 0, sz_zj[st_game].first, sz_zj[st_game].second, RGB(156, 125, 99));
			if (!tmp_st) t1 = GetTickCount();
			tmp_st = true;
		}

		if (st_game == OVER)
		{
			SelectObject(bufdc, hbm_zj[5]);
			MyTransparentBlt2(mdc, (GetSystemMetrics(SM_CXSCREEN) - sz_zj[5].first) / 2, (GetSystemMetrics(SM_CYSCREEN) - sz_zj[5].second) / 2, sz_zj[5].first, sz_zj[5].second, bufdc, 0, 0, sz_zj[5].first, sz_zj[5].second, RGB(156, 129, 99));
			
			if ((int)cur_time - over_time >= 1000)
			{
				st_main = GAME_OVER;
				st_game = PRE1;
				initMap();
			}
			// 向ffmpeg进程发送'q'命令以停止录制
			const char* stop_cmd = "q";
			DWORD written;
			if (!WriteFile(hPipeWrite, stop_cmd, strlen(stop_cmd), &written, NULL)) {
				std::cerr << "Failed to send stop command to ffmpeg." << std::endl;
			}

			// 关闭管道写句柄
			CloseHandle(hPipeWrite);

			// 等待ffmpeg进程完全终止
			WaitForSingleObject(pi.hProcess, INFINITE);

			// 关闭句柄
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
		}
	}
	else if (st_main == GAME_OVER)
	{
		SelectObject(bufdc, hbm_gameover);
		BitBlt(mdc, 0, 50, sz_gameover.first, sz_gameover.second, bufdc, 0, 0, SRCCOPY);
		if (score <= 100) DrawStar(1);
		else if (score <= 200) DrawStar(2);
		else DrawStar(3);
		DrawScore(score);
		DrawMealCnt(cnt_suc, cnt_los);
		DrawResButton();
	}
	else if (st_main == MAP_CREATE)
	{
		// 背景
		GamePaintBackground();
		// 组件栏
		DrawSub();
		// 画已有地图
		DrawCurMap();
		// 预设物品定位
		DrawNotDrop();
		// 取消放置
		DrawCancel();
		
		SelectObject(bufdc, hbm_map_start[map_start_st]);
		MyTransparentBlt2(mdc, 1180, 770, sz_map_start[map_start_st].first, sz_map_start[map_start_st].second, bufdc, 0, 0, sz_map_start[map_start_st].first, sz_map_start[map_start_st].second, RGB(255, 152, 0));
	}

	BitBlt(hdc, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), mdc, 0, 0, SRCCOPY);

	tPre = GetTickCount();         //记录此次绘图时间
}

//****消息处理函数***********************************
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_MOUSEMOVE:
	{
		x = LOWORD(lParam), y = HIWORD(lParam);

		// 菜单
		if (st_main == MENU)
		{
			if (menu_click_st == -1)
			{
				if (x >= 293 && y >= 120 && x <= 502 && y <= 211) menu_click = 0;
				else if (menu_click == 0) menu_click = -1;
				if (x >= 508 && y >= 149 && x <= 704 && y <= 240) menu_click = 1;
				else if (menu_click == 1) menu_click = -1;
				if (x >= 704 && y >= 158 && x <= 894 && y <= 248) menu_click = 2;
				else if (menu_click == 2) menu_click = -1;
				if (x >= 884 && y >= 177 && x <= 1029 && y <= 248) menu_click = 3;
				else if (menu_click == 3) menu_click = -1;
				if (x >= 1048 && y >= 184 && x <= 1183 && y <= 254) menu_click = 4;
				else if (menu_click == 4) menu_click = -1;
			}
			else
			{
				if (menu_click == 1)
				{
					if (x >= 250 && x <= 465 && y >= 370 && y <= 621) map_select_st[0] = true;
					else if (map_select_st[0]) map_select_st[0] = false;
					if (x >= 600 && x <= 815 && y >= 370 && y <= 621) map_select_st[1] = true;
					else if (map_select_st[1]) map_select_st[1] = false;
					if (x >= 950 && x <= 1165 && y >= 370 && y <= 621) map_select_st[2] = true;
					else if (map_select_st[2]) map_select_st[2] = false;
				}
				else if (menu_click == 2)
				{
					if (cnt_per == 1)
					{
						if ((x >= 500 && x <= 540 && y >= 570 && y <= 630) || (x >= 960 && x <= 1000 && y >= 570 && y <= 630)) cook_sle[0] = true;
						else if (cook_sle[0]) cook_sle[0] = false;
					}
					else if (cnt_per == 2)
					{
						if ((x >= 200 && x <= 240 && y >= 570 && y <= 630) || (x >= 660 && x <= 700 && y >= 570 && y <= 630)) cook_sle[0] = true;
						else if (cook_sle[0]) cook_sle[0] = false;

						if ((x >= 780 && x <= 820 && y >= 570 && y <= 630) || (x >= 1240 && x <= 1280 && y >= 570 && y <= 630)) cook_sle[1] = true;
						else if (cook_sle[1]) cook_sle[1] = false;
					}
				}
				else if (menu_click == 3)
				{
					if (set == -1)
					{
						if (x >= buttonSet[0].first - 27 && x <= buttonSet[0].first + 27 && y >= buttonSet[0].second - 27 && y <= buttonSet[0].second + 27) set = 0;
						else if (x >= buttonSet[1].first - 27 && x <= buttonSet[1].first + 27 && y >= buttonSet[1].second - 27 && y <= buttonSet[1].second + 27) set = 1;
					}
					else if (set >= 10)
					{
						int tmpx = x;
						tmpx = min(1100 - 27, tmpx);
						tmpx = max(550 + 27, tmpx);
						buttonSet[set % 10].first = tmpx;
						if (set % 10 == 0)
						{
							int newSize = 1.0 * 1000 * (tmpx - 550 - 54) / (550 - 54);
							SetVolume(newSize);
						}
						else if (set % 10 == 1)
						{
							int newSize = 1.0 * 40 * (tmpx - 550 - 54) / (550 - 54) + 10;
							for (int i = 0; i < 1; i++)
							{
								per[i].velocity = newSize;
							}
						}
					}
					else
					{
						if (!(x >= buttonSet[0].first - 27 && x <= buttonSet[0].first + 27 && y >= buttonSet[0].second - 27 && y <= buttonSet[0].second + 27) &&
							!(x >= buttonSet[1].first - 27 && x <= buttonSet[1].first + 27 && y >= buttonSet[1].second - 27 && y <= buttonSet[1].second + 27))
							set = -1;
					}
				}
			}
			if (x >= 90 && y >= 830 && x <= 155 && y <= 853) quit = true;
			else if (quit) quit = false;
		}
		else if (st_main == GAME_OVER)
		{
			if (x >= 310 && x <= 580 && y >= 680 && y <= 780) res_button_st[0] = true;
			else res_button_st[0] = false;
			if (x >= 910 && x <= 1170 && y >= 680 && y <= 780) res_button_st[1] = true;
			else res_button_st[1] = false;
		}
		else if (st_main == MAP_CREATE)
		{
			if (mouse_createmap.second == 0)
			{
				if (x >= 30 && x <= 30 + table_len + 1 && y >= 100 && y <= 100 + table_len + 1 + 13) mouse_createmap.first = 1;
				else if (mouse_createmap.first == 1) mouse_createmap.first = 0;
				if (x >= 150 && x <= 150 + table_len + 1 && y >= 100 && y <= 100 + table_len + 1 + 13) mouse_createmap.first = 2;
				else if (mouse_createmap.first == 2) mouse_createmap.first = 0;
				if (x >= 30 && x <= 30 + table_len + 1 && y >= 210 && y <= 210 + table_len + 1 + 13) mouse_createmap.first = 3;
				else if (mouse_createmap.first == 3) mouse_createmap.first = 0;
				if (x >= 150 && x <= 150 + table_len + 1 && y >= 210 && y <= 210 + table_len + 1 + 13) mouse_createmap.first = 4;
				else if (mouse_createmap.first == 4) mouse_createmap.first = 0;
				if (x >= 30 && x <= 30 + table_len + 1 && y >= 320 && y <= 320 + table_len + 1 + 13) mouse_createmap.first = 5;
				else if (mouse_createmap.first == 5) mouse_createmap.first = 0;
				if (x >= 150 && x <= 150 + table_len + 1 && y >= 320 && y <= 320 + table_len + 1 + 13) mouse_createmap.first = 6;
				else if (mouse_createmap.first == 6) mouse_createmap.first = 0;
				if (x >= 30 && x <= 30 + table_len + 1 && y >= 430 && y <= 430 + 2 * (table_len + 1)) mouse_createmap.first = 7;
				else if (mouse_createmap.first == 7) mouse_createmap.first = 0;
				if (x >= 150 && x <= 150 + table_len + 1 && y >= 430 && y <= 430 + 2 * (table_len + 1)) mouse_createmap.first = 8;
				else if (mouse_createmap.first == 8) mouse_createmap.first = 0;
				if (x >= 50 && x <= 50 + 2 * (table_len + 1) && y >= 590 && y <= 590 + table_len + 1) mouse_createmap.first = 9;
				else if (mouse_createmap.first == 9) mouse_createmap.first = 0;
				if (x >= 50 && x <= 50 + 2 * (table_len + 1) && y >= 680 && y <= 680 + table_len + 1) mouse_createmap.first = 10;
				else if (mouse_createmap.first == 10) mouse_createmap.first = 0;
				if (x >= 30 && x <= 30 + table_len + 1 && y >= 790 && y <= 790 + table_len + 1) mouse_createmap.first = 11;
				else if (mouse_createmap.first == 11) mouse_createmap.first = 0;
				if (x >= 280 && x <= 1190 && y >= 190 && y <= 820 && mouse_createmap.second == 0) mouse_cancel = 1;
				else if (mouse_cancel) mouse_cancel = 0;
				if (x >= 1180 && x <= 1434 && y >= 770 && y <= 884) map_start_st = 1;
				else if (map_start_st == 1) map_start_st = 0;
			}
		}
		return 0;
	}

	case WM_KEYDOWN:
	{
		if (GetAsyncKeyState(VK_ESCAPE))
		{
			exit(0);
		}
		if (st_main == FIRST_COVER)
		{
			if (GetAsyncKeyState(VK_SPACE) || GetAsyncKeyState('A'))
			{
				st_main = MENU;
			}
		}
		else if (st_main == MENU)
		{
			if (GetAsyncKeyState('A') || GetAsyncKeyState(VK_SPACE)) cnt_per = (cnt_per == 1 ? 2 : 1);
		}
		else if (st_main == GAME_GOING)
		{
			if (st_game == GOING)
			{
				// 第一个人
					// 人物行走
				if (GetAsyncKeyState('A'))
				{
					per[0].way = Z;
					per[0].st = GO;
				}
				else if (GetAsyncKeyState('S'))
				{
					per[0].way = Q;
					per[0].st = GO;
				}
				else if (GetAsyncKeyState('D'))
				{
					per[0].way = Y;
					per[0].st = GO;
				}
				else if (GetAsyncKeyState('W'))
				{
					per[0].way = H;
					per[0].st = GO;
				}

					// 拿东西/放东西
				if (GetAsyncKeyState('F'))
				{
					if (per[0].food) // 手里本来就拿了东西
					{
						if (!food[per[0].xxn][per[0].yyn] && !(per[1].xx == per[10].xxn && per[1].yy == per[0].yyn)) // 面前的格子没东西就把手里的东西丢到前面
						{
							if (trash[per[0].xxn][per[0].yyn]) // 面前是垃圾桶
							{
								if (per[0].food >= 5 && per[0].food <= 7) // 手里有盘子
								{
									per[0].food = 7; // 因为盘子不能丢掉
								}
								else // 手里没盘子
								{
									per[0].food = 0; // 直接丢掉即可
								}
							}
							else if (belt[per[0].xxn][per[0].yyn]) // 面前是传送带
							{
								DealDelivery(0, per[0].food);
							}
							else if (cut[per[0].xxn][per[0].yyn]) // 面前是切菜板
							{
								if (per[0].food >= 1 && per[0].food <= 4) // 手里没拿盘子
								{
									food[per[0].xxn][per[0].yyn] = per[0].food;
									per[0].food = 0;
								}
							}
							else if (!box[per[0].xxn][per[0].yyn] && !cupboard[per[0].xxn][per[0].yyn])
							{
								if (pt[per[0].xxn][per[0].yyn] && g[per[0].xxn][per[0].yyn]) // 面前有盘子放在桌子上
								{
									if (per[0].food >= 3 && per[0].food <= 4) // 手里是熟的食物
									{
										pt[per[0].xxn][per[0].yyn] = 0;
										food[per[0].xxn][per[0].yyn] = per[0].food + 2;
										per[0].food = 0;
									}
								}
								else
								{
									food[per[0].xxn][per[0].yyn] = per[0].food;
									per[0].food = 0;
								}
							}
						}
					}
					else // 手里是空的
					{
						if (cupboard[per[0].xxn][per[0].yyn]) // 面前是碗柜
						{
							if (pt[per[0].xxn][per[0].yyn]) // 有盘子
							{
								per[0].food = 7;
								pt[per[0].xxn][per[0].yyn]--;
							}
						}
						else if (food[per[0].xxn][per[0].yyn]) // 面前有食物
						{
							per[0].food = food[per[0].xxn][per[0].yyn];
							food[per[0].xxn][per[0].yyn] = 0;
						}
						else if (pt[per[0].xxn][per[0].yyn])
						{
							per[0].food = 7;
							pt[per[0].xxn][per[0].yyn]--;
						}
						else if (box[per[0].xxn][per[0].yyn]) // 面前是食材柜
						{
							per[0].food = box[per[0].xxn][per[0].yyn];
						}
					}
				}

					// 切菜
				if (GetAsyncKeyState('G'))
				{
					// 手里没拿东西 and 面前是切菜板 and 切菜板上有生的食物 才能切
					if (per[0].food == 0 && cut[per[0].xxn][per[0].yyn] && (food[per[0].xxn][per[0].yyn] == 1 || food[per[0].xxn][per[0].yyn] == 2)) 
					{
						Cutting(0, per[0].xxn, per[0].yyn);
					}
				}

				// 第二个人
				if (cnt_per == 2)
				{
					// 人物行走
					if (GetAsyncKeyState(VK_LEFT))
					{
						per[1].way = Z;
						per[1].st = GO;
					}
					else if (GetAsyncKeyState(VK_DOWN))
					{
						per[1].way = Q;
						per[1].st = GO;
					}
					else if (GetAsyncKeyState(VK_RIGHT))
					{
						per[1].way = Y;
						per[1].st = GO;
					}
					else if (GetAsyncKeyState(VK_UP))
					{
						per[1].way = H;
						per[1].st = GO;
					}

					// 拿东西/放东西
					if (GetAsyncKeyState('L'))
					{
						if (per[1].food) // 手里本来就拿了东西
						{
							if (!food[per[1].xxn][per[1].yyn] && !(per[0].xx == per[1].xxn && per[0].yy == per[1].yyn)) // 面前的格子没东西就把手里的东西丢到前面
							{
								if (trash[per[1].xxn][per[1].yyn]) // 面前是垃圾桶
								{
									if (per[1].food >= 5 && per[1].food <= 7) // 手里有盘子
									{
										per[1].food = 7; // 因为盘子不能丢掉
									}
									else // 手里没盘子
									{
										per[1].food = 0; // 直接丢掉即可
									}
								}
								else if (belt[per[1].xxn][per[1].yyn]) // 面前是传送带
								{
									DealDelivery(1, per[1].food);
								}
								else if (cut[per[1].xxn][per[1].yyn]) // 面前是切菜板
								{
									if (per[1].food >= 1 && per[1].food <= 4) // 手里没拿盘子
									{
										food[per[1].xxn][per[1].yyn] = per[1].food;
										per[1].food = 0;
									}
								}
								else if (!box[per[1].xxn][per[1].yyn] && !cupboard[per[1].xxn][per[1].yyn])
								{
									if (pt[per[1].xxn][per[1].yyn] && g[per[1].xxn][per[1].yyn]) // 面前有盘子放在桌子上
									{
										if (per[1].food >= 3 && per[1].food <= 4) // 手里是熟的食物
										{
											pt[per[1].xxn][per[1].yyn] = 0;
											food[per[1].xxn][per[1].yyn] = per[1].food + 2;
											per[1].food = 0;
										}
									}
									else
									{
										food[per[1].xxn][per[1].yyn] = per[1].food;
										per[1].food = 0;
									}
								}
							}
						}
						else // 手里是空的
						{
							if (cupboard[per[1].xxn][per[1].yyn]) // 面前是碗柜
							{
								if (pt[per[1].xxn][per[1].yyn]) // 有盘子
								{
									per[1].food = 7;
									pt[per[1].xxn][per[1].yyn]--;
								}
							}
							else if (food[per[1].xxn][per[1].yyn]) // 面前有食物
							{
								per[1].food = food[per[1].xxn][per[1].yyn];
								food[per[1].xxn][per[1].yyn] = 0;
							}
							else if (pt[per[1].xxn][per[1].yyn])
							{
								per[1].food = 7;
								pt[per[1].xxn][per[1].yyn]--;
							}
							else if (box[per[1].xxn][per[1].yyn]) // 面前是食材柜
							{
								per[1].food = box[per[1].xxn][per[1].yyn];
							}
						}

					}
					
					// 切菜
					if (GetAsyncKeyState('K'))
					{
						// 手里没拿东西 and 面前是切菜板 and 切菜板上有生的食物 才能切
						if (per[1].food == 0 && cut[per[1].xxn][per[1].yyn] && (food[per[1].xxn][per[1].yyn] == 1 || food[per[1].xxn][per[1].yyn] == 2))
						{
							Cutting(1, per[1].xxn, per[1].yyn);
						}
					}
				}
			}
		}
		return 0;
	}

	case WM_KEYUP:
	{
		if (st_main == GAME_GOING)
		{
			if (per[0].st == GO && !(GetAsyncKeyState('W') || GetAsyncKeyState('S') || GetAsyncKeyState('A') || GetAsyncKeyState('D')))
			{
				per[0].idx = 0;
				per[0].st = STAND;
			}

			if (per[1].st == GO && !(GetAsyncKeyState(VK_UP) || GetAsyncKeyState(VK_DOWN) || GetAsyncKeyState(VK_LEFT) || GetAsyncKeyState(VK_RIGHT)))
			{
				per[1].idx = 0;
				per[1].st = STAND;
			}
		}
		
		return 0;
	}

	case WM_LBUTTONDOWN:
	{
		if (st_main == MENU)
		{
			if (menu_click == 0) st_main = GAME_GOING;
			else if (menu_click == 1)
			{
				menu_click_st = 1;
				for (int i = 0; i < 3; i++)
				{
					if (map_select_st[i])
					{
						mapId = i + 1;
						st_main = MAP_CREATE;
					}
				}
			}
			else if (menu_click == 2)
			{
				menu_click_st = 2;
				if (cnt_per == 1)
				{
					if (cook_sle[0]) per[0].id = (per[0].id == 0 ? 1 : 0);
				}
				else
				{
					if (cook_sle[0]) per[0].id = (per[0].id == 0 ? 1 : 0);
					if (cook_sle[1]) per[1].id = (per[1].id == 0 ? 1 : 0);
				}
			}
			else if (menu_click == 3)
			{
				menu_click_st = 3;
				if (set != -1)
				{
					set += 10;
				}
			}
			if (quit)
			{
				if (menu_click_st == -1)
					st_main = FIRST_COVER;
				else if (menu_click_st == 1 || menu_click_st == 2 || menu_click_st == 3)
				{
					menu_click_st = -1;
					menu_click = -1;
				}
			}
		}
		else if (st_main == GAME_OVER)
		{
			if (res_button_st[0])
			{
				PlayVideo(hwnd, L"D:/000AAATexcavator/windows/overcooked/overcooked/out.mp4");
				res_button_st[0] = false;
			}
			else if (res_button_st[1])
			{
				st_main = MENU;
			}
		}
		else if (st_main == MAP_CREATE)
		{
			if (x >= 0 && x <= 260 && y >= 50 && y <= 890)
			{
				if (mouse_createmap.first != 0 && mouse_createmap.second != 1) mouse_createmap.second = 1;
			}
			else if (x >= 280 && x <= 1190 && y >= 190 && y <= 820)
			{
				if (mouse_createmap.first != 0 && mouse_createmap.second == 1) mouse_createmap.second = 2;
				if (mouse_cancel == 1) mouse_cancel = 2;
			}
			if (map_start_st == 1) st_main = GAME_GOING;
		}
		
		return 0;
	}

	case WM_LBUTTONUP:
	{
		if (st_main == MENU)
		{
			if (menu_click == 3)
			{
				set = -1;
			}
		}
		return 0;
	}

	case WM_RBUTTONDOWN:
	{
		mouse_createmap.first = mouse_createmap.second = 0;
	}

	case WM_MOUSEWHEEL:
	{
		return 0;
	}

	case WM_DESTROY:
	{
		DeleteDC(mdc);
		DeleteDC(bufdc);
		for (int i = 0; i < 1; i++)
			for (int j = 0; j < 4; j++)
				for (int k = 0; k < 3; k++)
					DeleteObject(hbm_per[i][j][k]);
		ReleaseDC(hwnd, hdc);

		StopBackgroundMusic();

		PostQuitMessage(0);
		return 0;
	}

	default:							//其他消息
		return DefWindowProc(hwnd, message, wParam, lParam);
	}
	return 0;
}

// 等比例缩放函数
void IsometricScalingByRyanzi(HBITMAP& hbm, char* name, int& wei, int& hei, int hy)
{
	BITMAP bm;
	hbm = (HBITMAP)LoadImage(NULL, name, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	GetObject(hbm, sizeof(BITMAP), &bm);
	DeleteObject(hbm);
	double w = wei * GetSystemMetrics(SM_CXSCREEN) / hy;
	double rate = w / bm.bmWidth;
	wei = bm.bmWidth * rate;
	hei = bm.bmHeight * rate;
	hbm = (HBITMAP)LoadImage(NULL, name, IMAGE_BITMAP, wei, hei, LR_LOADFROMFILE);
}

// 透明贴图
BOOL MyTransparentBlt2(HDC hdcDest, int nXOriginDest, int nYOriginDest, int nWidthDest, int nHeightDest, HDC hdcSrc,
	int nXOriginSrc, int nYOriginSrc, int nWidthSrc, int nHeightSrc, UINT crTransparent)
{
	if (nWidthDest < 1)
		return false;
	if (nWidthSrc < 1)
		return false;
	if (nHeightDest < 1)
		return false;
	if (nHeightSrc < 1)
		return false;

	HDC dc = CreateCompatibleDC(NULL);
	HBITMAP bitmap = CreateBitmap(nWidthSrc, nHeightSrc, 1, GetDeviceCaps(dc, BITSPIXEL), NULL);
	if (bitmap == NULL)
	{
		DeleteDC(dc);
		return false;
	}
	HBITMAP oldBitmap = (HBITMAP)SelectObject(dc, bitmap);
	if (!BitBlt(dc, 0, 0, nWidthSrc, nHeightSrc, hdcSrc, nXOriginSrc, nYOriginSrc, SRCCOPY))
	{
		SelectObject(dc, oldBitmap);
		DeleteObject(bitmap);
		DeleteDC(dc);
		return false;
	}
	HDC maskDC = CreateCompatibleDC(NULL);
	HBITMAP maskBitmap = CreateBitmap(nWidthSrc, nHeightSrc, 1, 1, NULL);
	if (maskBitmap == NULL)
	{
		SelectObject(dc, oldBitmap);
		DeleteObject(bitmap);
		DeleteDC(dc);
		DeleteDC(maskDC);
		return false;
	}
	HBITMAP oldMask = (HBITMAP)SelectObject(maskDC, maskBitmap);
	SetBkColor(maskDC, RGB(0, 0, 0));
	SetTextColor(maskDC, RGB(255, 255, 255));
	if (!BitBlt(maskDC, 0, 0, nWidthSrc, nHeightSrc, NULL, 0, 0, BLACKNESS))
	{
		SelectObject(maskDC, oldMask);
		DeleteObject(maskBitmap);
		DeleteDC(maskDC);
		SelectObject(dc, oldBitmap);
		DeleteObject(bitmap);
		DeleteDC(dc);
		return false;
	}
	SetBkColor(dc, crTransparent);
	BitBlt(maskDC, 0, 0, nWidthSrc, nHeightSrc, dc, 0, 0, SRCINVERT);
	SetBkColor(dc, RGB(0, 0, 0));
	SetTextColor(dc, RGB(255, 255, 255));
	BitBlt(dc, 0, 0, nWidthSrc, nHeightSrc, maskDC, 0, 0, SRCAND);
	HDC newMaskDC = CreateCompatibleDC(NULL);
	HBITMAP newMask;
	newMask = CreateBitmap(nWidthDest, nHeightDest, 1, GetDeviceCaps(newMaskDC, BITSPIXEL), NULL);
	if (newMask == NULL)
	{
		SelectObject(dc, oldBitmap);
		DeleteDC(dc);
		SelectObject(maskDC, oldMask);
		DeleteDC(maskDC);
		DeleteDC(newMaskDC);
		DeleteObject(bitmap);
		DeleteObject(maskBitmap);
		return false;
	}
	SetStretchBltMode(newMaskDC, COLORONCOLOR);
	HBITMAP oldNewMask = (HBITMAP)SelectObject(newMaskDC, newMask);
	StretchBlt(newMaskDC, 0, 0, nWidthDest, nHeightDest, maskDC, 0, 0, nWidthSrc, nHeightSrc, SRCCOPY);
	SelectObject(maskDC, oldMask);
	DeleteDC(maskDC);
	DeleteObject(maskBitmap);
	HDC newImageDC = CreateCompatibleDC(NULL);
	HBITMAP newImage = CreateBitmap(nWidthDest, nHeightDest, 1, GetDeviceCaps(newMaskDC, BITSPIXEL), NULL);
	if (newImage == NULL)
	{
		SelectObject(dc, oldBitmap);
		DeleteDC(dc);
		DeleteDC(newMaskDC);
		DeleteObject(bitmap);
		return false;
	}
	HBITMAP oldNewImage = (HBITMAP)SelectObject(newImageDC, newImage);
	StretchBlt(newImageDC, 0, 0, nWidthDest, nHeightDest, dc, 0, 0, nWidthSrc, nHeightSrc, SRCCOPY);
	SelectObject(dc, oldBitmap); DeleteDC(dc); DeleteObject(bitmap);
	BitBlt(hdcDest, nXOriginDest, nYOriginDest, nWidthDest, nHeightDest, newMaskDC, 0, 0, SRCAND);
	BitBlt(hdcDest, nXOriginDest, nYOriginDest, nWidthDest, nHeightDest, newImageDC, 0, 0, SRCPAINT);
	SelectObject(newImageDC, oldNewImage);
	DeleteDC(newImageDC);
	SelectObject(newMaskDC, oldNewMask);
	DeleteDC(newMaskDC);
	DeleteObject(newImage);
	DeleteObject(newMask);
	return true;
}

// 加载位图
void LoadBitmapInit()
{
	char a[100];

	// bk
	for (int i = 0; i < 5; i++)
	{
		sprintf_s(a, "images/bk%d.bmp", i);
		IsometricScalingByRyanzi(hbm_bk[i], a, sz_bk[i].first, sz_bk[i].second, BIG);
	}

	// per
	char op[5] = "qhzy";
	for (int i = 0; i < 2; i++) // 人物数量
	{
		for (int j = 0; j < 4; j++) // 四个方向
		{
			for (int k = 0; k < 3; k++) // 三种步子
			{
				sprintf_s(a, "images/per%d_%c%d.bmp", i, op[j], k);
				IsometricScalingByRyanzi(hbm_per[i][j][k], a, sz_per[i][j][k].first, sz_per[i][j][k].second, BIG);
			}
		}
	}

	// zj
	for (int i = 0; i < 6; i++)
	{
		sprintf_s(a, "images/zj%d.bmp", i);
		IsometricScalingByRyanzi(hbm_zj[i], a, sz_zj[i].first, sz_zj[i].second, BIG);
	}

	// ml
	for (int i = 0; i < 2; i++)
	{
		sprintf_s(a, "images/meal%d.bmp", i);
		IsometricScalingByRyanzi(hbm_ml[i], a, sz_ml[i].first, sz_ml[i].second, BIG);
	}

	// num
	for (int i = 0; i < 10; i++)
	{
		sprintf_s(a, "images/num%d.bmp", i);
		IsometricScalingByRyanzi(hbm_num[i], a, sz_num[i].first, sz_num[i].second, BIG);
	}

	// menu
	for (int i = 0; i < 7; i++)
	{
		sprintf_s(a, "images/menu%d.bmp", i);
		IsometricScalingByRyanzi(hbm_menu[i], a, sz_menu[i].first, sz_menu[i].second, BIG);
	}

	// click
	for (int i = 0; i < 8; i++)
	{
		sprintf_s(a, "images/click%d.bmp", i);
		hbm_click[i] = (HBITMAP)LoadImage(NULL, a, IMAGE_BITMAP, sz_click[i].first * 0.67, sz_click[i].second * 0.67, LR_LOADFROMFILE);
		sz_click[i].first *= 0.67, sz_click[i].second *= 0.67;
	}

	// plate
	sprintf_s(a, "images/plate.bmp");
	hbm_plate = (HBITMAP)LoadImage(NULL, a, IMAGE_BITMAP, sz_plate.first, sz_plate.second, LR_LOADFROMFILE);

	// src
	for (int i = 1; i <= 2; i++)
	{
		sprintf_s(a, "images/src%d.bmp", i);
		hbm_src[i] = (HBITMAP)LoadImage(NULL, a, IMAGE_BITMAP, sz_src[i].first, sz_src[i].second, LR_LOADFROMFILE);
	}

	// food
	for (int i = 1; i <= 2; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			sprintf_s(a, "images/food%d%d.bmp", i, j);
			hbm_food[i][j] = (HBITMAP)LoadImage(NULL, a, IMAGE_BITMAP, sz_food[i][j].first, sz_food[i][j].second, LR_LOADFROMFILE);
		}
	}

	// knife
	sprintf_s(a, "images/knife.bmp");
	hbm_knife = (HBITMAP)LoadImage(NULL, a, IMAGE_BITMAP, sz_knife.first, sz_knife.second, LR_LOADFROMFILE);

	// error
	for (int i = 0; i < 3; i++)
	{
		sprintf_s(a, "images/error%d.bmp", i);
		hbm_error[i] = (HBITMAP)LoadImage(NULL, a, IMAGE_BITMAP, sz_error[i].first, sz_error[i].second, LR_LOADFROMFILE);
	}

	// success
	sprintf_s(a, "images/getscore.bmp");
	hbm_success = (HBITMAP)LoadImage(NULL, a, IMAGE_BITMAP, sz_success.first, sz_success.second, LR_LOADFROMFILE);

	// lose score
	sprintf_s(a, "images/losescore.bmp");
	hbm_lose = (HBITMAP)LoadImage(NULL, a, IMAGE_BITMAP, sz_lose.first, sz_lose.second, LR_LOADFROMFILE);

	// num_res
	for (int i = 0; i < 10; i++)
	{
		sprintf_s(a, "images/num_res%d.bmp", i);
		hbm_numres[i] = (HBITMAP)LoadImage(NULL, a, IMAGE_BITMAP, sz_numres[i].first, sz_numres[i].second, LR_LOADFROMFILE);
	}

	// gameover
	sprintf_s(a, "images/gameover.bmp");
	IsometricScalingByRyanzi(hbm_gameover, a, sz_gameover.first, sz_gameover.second, BIG);

	// star
	sprintf_s(a, "images/star.bmp");
	hbm_star = (HBITMAP)LoadImage(NULL, a, IMAGE_BITMAP, sz_star.first, sz_star.second, LR_LOADFROMFILE);

	// num_s
	for (int i = 0; i < 10; i++)
	{
		sprintf_s(a, "images/num_s%d.bmp", i);
		hbm_num_s[i] = (HBITMAP)LoadImage(NULL, a, IMAGE_BITMAP, sz_num_s[i].first, sz_num_s[i].second, LR_LOADFROMFILE);
	}

	// bt
	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			sprintf_s(a, "images/bt%d%d.bmp", i, j);
			hbm_bt[i][j] = (HBITMAP)LoadImage(NULL, a, IMAGE_BITMAP, sz_bt[i].first, sz_bt[i].second, LR_LOADFROMFILE);
		}
	}

	// mc
	for (int i = 0; i < 5; i++)
	{
		sprintf_s(a, "images/map_change%d.bmp", i);
		hbm_mc[i] = (HBITMAP)LoadImage(NULL, a, IMAGE_BITMAP, sz_mc[i].first, sz_mc[i].second, LR_LOADFROMFILE);
	}

	// cancel
	sprintf_s(a, "images/cancel.bmp");
	hbm_cancel = (HBITMAP)LoadImage(NULL, a, IMAGE_BITMAP, sz_cancel.first, sz_cancel.second, LR_LOADFROMFILE);

	// map_start
	for (int i = 0; i < 2; i++)
	{
		sprintf_s(a, "images/map_start%d.bmp", i);
		hbm_map_start[i] = (HBITMAP)LoadImage(NULL, a, IMAGE_BITMAP, sz_map_start[i].first, sz_map_start[i].second, LR_LOADFROMFILE);
	}

	// dbl
	sprintf_s(a, "images/dblper.bmp");
	hbm_dbl = (HBITMAP)LoadImage(NULL, a, IMAGE_BITMAP, sz_dbl.first, sz_dbl.second, LR_LOADFROMFILE);

	// cook
	for (int i = 0; i < 5; i++)
	{
		sprintf_s(a, "images/cook%d.bmp", i);
		hbm_cook[i] = (HBITMAP)LoadImage(NULL, a, IMAGE_BITMAP, sz_cook[i].first, sz_cook[i].second, LR_LOADFROMFILE);
	}

	// per_sle
	for (int i = 0; i < 2; i++)
	{
		sprintf_s(a, "images/per_sle%d.bmp", i);
		hbm_per_sle[i] = (HBITMAP)LoadImage(NULL, a, IMAGE_BITMAP, sz_per_sle[i].first, sz_per_sle[i].second, LR_LOADFROMFILE);
	}
}

// 游戏中绘制背景
void GamePaintBackground()
{
	SelectObject(bufdc, hbm_bk[1]);
	for (int i = 0; i < GetSystemMetrics(SM_CXSCREEN); i += sz_bk[1].first)
	{
		for (int j = 50; j <= GetSystemMetrics(SM_CYSCREEN) - 57 - sz_bk[1].second; j += sz_bk[1].second)
		{
			BitBlt(mdc, i, j, sz_bk[1].first, sz_bk[1].second, bufdc, 0, 0, SRCCOPY);
		}
	}
	SelectObject(bufdc, hbm_bk[2]);
	BitBlt(mdc, GetSystemMetrics(SM_CXSCREEN) - sz_bk[2].first + 350, 50, sz_bk[2].first, GetSystemMetrics(SM_CYSCREEN) - 120, bufdc, 0, 0, SRCCOPY);
	SelectObject(bufdc, hbm_bk[3]);
	BitBlt(mdc, sz_bk[3].first - 450, 50, sz_bk[3].first, GetSystemMetrics(SM_CYSCREEN) - 120, bufdc, 0, 0, SRCCOPY);
	SelectObject(bufdc, hbm_bk[4]);
	MyTransparentBlt2(mdc, (GetSystemMetrics(SM_CXSCREEN) - sz_bk[4].first) / 2 + 20, 50, sz_bk[4].first, sz_bk[4].second, bufdc, 0, 0, sz_bk[4].first, sz_bk[4].second, RGB(165, 140, 85));
}

// 游戏中绘制订单
void GamePaintMeal()
{
	// 每20s派发一个订单
	if ((cur_time - last_time) > 5000) 
	{
		ml.push_back({ rand() % 2, cur_time, 30000, true});
		last_time = GetTickCount();
	}
	int lu = 10; // 当前订单左上角位置
	// 是否有订单已经过期
	for (auto& t : ml)
	{
		if (!t.st) continue;
		if ((cur_time - t.time_start) > t.time_len)
		{
			t.st = false;
			// 未及时完成
			message.push_back({ 4, GetTickCount(), 130, 750 });
			score -= 20;
			score = max(score, 0);
			cnt_los++;
		}
		else
		{
			SelectObject(bufdc, hbm_ml[t.id]);
			MyTransparentBlt2(mdc, lu, 50, sz_ml[t.id].first, sz_ml[t.id].second, bufdc, 0, 0, sz_ml[t.id].first, sz_ml[t.id].second, RGB(156, 125, 99));
			lu += sz_ml[t.id].first + 20;
		}
	}
}

// 游戏中绘制分数
void GamePaintScore()
{
	SelectObject(bufdc, hbm_zj[3]);
	MyTransparentBlt2(mdc, 20, GetSystemMetrics(SM_CYSCREEN) - sz_zj[3].second - 80, sz_zj[3].first, sz_zj[3].second, bufdc, 0, 0, sz_zj[3].first, sz_zj[3].second, RGB(156, 125, 99));
	int tmp_loc = 220, tmp_score = score;
	while (1)
	{
		int n = tmp_score % 10;
		tmp_score /= 10;
		SelectObject(bufdc, hbm_num[n]);
		MyTransparentBlt2(mdc, tmp_loc, 800, sz_num[n].first, sz_num[n].second, bufdc, 0, 0, sz_num[n].first, sz_num[n].second, RGB(75, 104, 136));
		tmp_loc -= sz_num[n].first + 5;
		if (tmp_score == 0) break;
	}
}

// 游戏中绘制时间
void GamePaintTime()
{
	SelectObject(bufdc, hbm_zj[4]);
	MyTransparentBlt2(mdc, GetSystemMetrics(SM_CXSCREEN) - sz_zj[4].first - 10, GetSystemMetrics(SM_CYSCREEN) - sz_zj[4].second - 80, sz_zj[4].first, sz_zj[4].second, bufdc, 0, 0, sz_zj[4].first, sz_zj[4].second, RGB(156, 125, 99));
	int tmp_loc = 1300, tmp_time = (time_have - (cur_time - start_time)) / 1000;
	if (st_game == OVER) tmp_time = 0;
	while (1)
	{
		int n = tmp_time % 10;
		tmp_time /= 10;
		SelectObject(bufdc, hbm_num[n]);
		MyTransparentBlt2(mdc, tmp_loc, 800, sz_num[n].first, sz_num[n].second, bufdc, 0, 0, sz_num[n].first, sz_num[n].second, RGB(75, 104, 136));
		tmp_loc -= sz_num[n].first - 5;
		if (tmp_time == 0) break;
	}
	if (st_game == GOING && time_have - ((int)cur_time - (int)start_time) <= 0)
	{
		st_game = OVER;
		over_time = GetTickCount();
	}
}

// 调整音量
void SetVolume(int volume)
{
	// 限制音量范围在 0 到 1000 之间
	if (volume < 0) volume = 0;
	if (volume > 1000) volume = 1000;

	std::string command = "setaudio bgm volume to " + std::to_string(volume);
	if (mciSendString(command.c_str(), NULL, 0, NULL) != 0) {
		char errorText[256];
		mciGetErrorString(mciSendString(command.c_str(), NULL, 0, NULL), errorText, sizeof(errorText));
		MessageBox(NULL, errorText, "MCI Error", MB_OK | MB_ICONERROR);
	}
}

// 播放音乐
void PlayBackgroundMusic(const char* filepath)
{
	// 关闭已有音乐
	mciSendString("close bgm", NULL, 0, NULL);

	std::string command = "open \"" + std::string(filepath) + "\" type mpegvideo alias bgm";
	mciSendString(command.c_str(), NULL, 0, NULL);

	// 设置初始音量
	SetVolume(currentVolume);

	// 重复播放
	mciSendString("play bgm repeat", NULL, 0, NULL);
}

// 停止音乐
void StopBackgroundMusic()
{
	mciSendString("stop bgm", NULL, 0, NULL);
	mciSendString("close bgm", NULL, 0, NULL);
}

// 更换音乐
void ChangeMusic(const char* filepath)
{
	StopBackgroundMusic();
	PlayBackgroundMusic(filepath);
}

// 判断是否需要更换
void ChangeGameState(int newState)
{
	std::map<int, int> mp;
	mp[FIRST_COVER] = 0;
	mp[MENU] = mp[GAME_OVER] = 1;
	mp[GAME_GOING] = 2;
	mp[MAP_CREATE] = 3;
	if (mp[lastState] == mp[newState]) return;
	if (mp[newState] == 0)
	{
		ChangeMusic("music/Menu_Title_Screen.mp3");
	}
	else if (mp[newState] == 1)
	{
		ChangeMusic("music/MAP.mp3");
	}
	else if (mp[newState] == 2)
	{
		ChangeMusic("music/Sushi_City.mp3");
	}
	else if (mp[newState] == 3)
	{
		ChangeMusic("music/Throne_Room.ogg");
	}
}

// 绘制渐变矩形
void DrawGradientRectangle(HDC mdc, int left, int top, int len)
{

	// 定义左下角和右上角的颜色
	COLORREF colorStart = RGB(203, 130, 57); // 左下角颜色
	COLORREF colorEnd = RGB(252, 185, 89);  // 右上角颜色

	// 创建两个顶点，用于定义渐变的颜色和位置
	TRIVERTEX vert[2];
	vert[0].x = left;
	vert[0].y = top + len;
	vert[0].Red = GetRValue(colorStart) << 8;   // 转换为0-255范围的颜色值
	vert[0].Green = GetGValue(colorStart) << 8; // 转换为0-255范围的颜色值
	vert[0].Blue = GetBValue(colorStart) << 8;  // 转换为0-255范围的颜色值
	vert[0].Alpha = 0x0000;

	vert[1].x = left + len;
	vert[1].y = top;
	vert[1].Red = GetRValue(colorEnd) << 8;   // 转换为0-255范围的颜色值
	vert[1].Green = GetGValue(colorEnd) << 8; // 转换为0-255范围的颜色值
	vert[1].Blue = GetBValue(colorEnd) << 8;  // 转换为0-255范围的颜色值
	vert[1].Alpha = 0x0000;

	// 创建梯形结构，定义渐变的区域
	GRADIENT_RECT gRect;
	gRect.UpperLeft = 0;
	gRect.LowerRight = 1;

	// 使用GradientFill函数填充渐变颜色
	GradientFill(mdc, vert, 2, &gRect, 1, GRADIENT_FILL_RECT_H);
}

// 绘制桌子
void DrawTable(int left, int top)
{
	hp = CreatePen(PS_SOLID, 1, RGB(240, 214, 142));
	SelectObject(mdc, hp);
	Rectangle(mdc, left, top, left + table_len + 1, top + table_len + 1);
	Rectangle(mdc, left, top + table_len + 1, left + table_len + 1, top + table_len + 1 + 13);
	DeleteObject(hp);

	hp = CreatePen(PS_SOLID, 1, RGB(63, 38, 18));
	hbr = CreateSolidBrush(RGB(63, 38, 18));
	SelectObject(mdc, hp);
	SelectObject(mdc, hbr);
	Rectangle(mdc, left, top + table_len + 1, left + table_len, top + table_len + 1 + 13);
	DeleteObject(hp);
	DeleteObject(hbr);

	hp = CreatePen(PS_SOLID, 1, RGB(179, 169, 156));
	hbr = CreateSolidBrush(RGB(179, 169, 156));
	SelectObject(mdc, hp);
	SelectObject(mdc, hbr);
	RoundRect(mdc, left + table_len / 2 - 5, top + table_len + 8 - 2, left + table_len / 2 + 5, top + table_len + 8 + 2, 3, 3);
	DeleteObject(hp);
	DeleteObject(hbr);

	DrawGradientRectangle(mdc, left, top, table_len);
}

// 更新人物所在方格
void CalculatePerPos(int k)
{
	per[k].xx = (int)((per[k].posy + sz_per[per[k].id][per[k].way][per[k].idx].second - 40 - 190) / table_len);
	per[k].yy = (int)((per[k].posx + sz_per[per[k].id][per[k].way][per[k].idx].first / 2 - 280) / table_len);
	if (per[k].way == Q) per[k].xxn = per[k].xx + 1, per[k].yyn = per[k].yy;
	else if (per[k].way == H) per[k].xxn = per[k].xx - 1, per[k].yyn = per[k].yy;
	else if (per[k].way == Z) per[k].xxn = per[k].xx, per[k].yyn = per[k].yy - 1;
	else if (per[k].way == Y) per[k].xxn = per[k].xx, per[k].yyn = per[k].yy + 1;
}

// 绘制食物
void DrawFood(int id, int x, int y)
{
	int kind = 0, tem = 0;
	bool plate = false, eat = true;
	if (id >= 1 && id <= 2) kind = id, tem = 0;
	else if (id >= 3 && id <= 4) kind = id - 2, tem = 1;
	else if (id >= 5 && id <= 7)
	{
		plate = true;
		if (id == 7) eat = false;
		else kind = id - 4, tem = 1;
	}
	if (plate)
	{
		SelectObject(bufdc, hbm_plate);
		MyTransparentBlt2(mdc, x - sz_plate.first / 2, y - sz_plate.second / 2, sz_plate.first, sz_plate.second, bufdc, 0, 0, sz_plate.first, sz_plate.second, RGB(245, 183, 97));
	}
	if (eat)
	{
		SelectObject(bufdc, hbm_food[kind][tem]);
		MyTransparentBlt2(mdc, x - sz_food[kind][tem].first / 2, y - sz_food[kind][tem].second / 2, sz_food[kind][tem].first, sz_food[kind][tem].second, bufdc, 0, 0, sz_food[kind][tem].first, sz_food[kind][tem].second, RGB(255, 255, 255));
	}
}

// 绘制人物
void DrawPerson(int i)
{
	int newX, newY, newIdxX, newIdxY, tmp1, tmp2;
	if (per[i].st == GO)
	{
		if (per[i].idx == 0 || per[i].idx == 2) per[i].idx = 1;
		else per[i].idx = 2;
		bool per_go = true;
		if (per[i].way == Q)
		{ 
			for (int j = 0; j < 3; j++)
			{
				newY = per[i].posy + sz_per[per[i].id][per[i].way][j].second * 3 / 5 + per[i].velocity;
				newIdxY = (int)((newY - 190) / table_len);
				if (g[newIdxY][per[i].yy]) per_go = false;
			}
			if (per_go) per[i].posy += per[i].velocity;
		}
		else if (per[i].way == H)
		{
			for (int j = 0; j < 3; j++)
			{
				newY = per[i].posy + sz_per[per[i].id][per[i].way][j].second * 2 / 5 - per[i].velocity;
				newIdxY = (int)((newY - 190) / table_len);
				if (g[newIdxY][per[i].yy]) per_go = false;
			}
			if (per_go) per[i].posy -= per[i].velocity; 
		}
		else if (per[i].way == Z)
		{
			for (int j = 0; j < 3; j++)
			{
				newX = per[i].posx + sz_per[per[i].id][per[i].way][j].first / 4 - per[i].velocity;
				newIdxX = (int)((newX - 280) / table_len);
				if (g[per[i].xx][newIdxX]) per_go = false;
			}
			if (per_go) per[i].posx -= per[i].velocity;
		}
		else if (per[i].way == Y)
		{
			for (int j = 0; j < 3; j++)
			{
				newX = per[i].posx + sz_per[per[i].id][per[i].way][j].first * 3 / 4 + per[i].velocity;
				newIdxX = (int)((newX - 280) / table_len);
				if (g[per[i].xx][newIdxX]) per_go = false;
			}
			if (per_go) per[i].posx += per[i].velocity;
		}
		per[i].posx = min(1030, max(340, per[i].posx));
		per[i].posy = min(630, max(230, per[i].posy));
	}
	if (per[i].food && per[i].way == H)
	{
		int x = per[i].posx + sz_per[per[i].id][per[i].way][per[i].idx].first / 2;
		int y = per[i].posy;
		DrawFood(per[i].food, x, y);
	}
	SelectObject(bufdc, hbm_per[per[i].id][per[i].way][per[i].idx]);
	Sleep(100);
	MyTransparentBlt2(mdc, per[i].posx, per[i].posy, sz_per[per[i].id][per[i].way][per[i].idx].first, sz_per[per[i].id][per[i].way][per[i].idx].second, bufdc, 0, 0, sz_per[per[i].id][per[i].way][per[i].idx].first, sz_per[per[i].id][per[i].way][per[i].idx].second, RGB(244, 67, 54));
	if (per[i].food && per[i].way != H)
	{
		int x, y;
		if (per[i].way == Q) x = per[i].posx + sz_per[per[i].id][per[i].way][per[i].idx].first / 2, y = per[i].posy + sz_per[per[i].id][per[i].way][per[i].idx].second;
		else if (per[i].way == Z) x = per[i].posx - sz_per[per[i].id][per[i].way][per[i].idx].first / 5, y = per[i].posy + sz_per[per[i].id][per[i].way][per[i].idx].second / 2;
		else if (per[i].way == Y) x = per[i].posx + sz_per[per[i].id][per[i].way][per[i].idx].first * 7 / 6, y = per[i].posy + sz_per[per[i].id][per[i].way][per[i].idx].second / 2;
		DrawFood(per[i].food, x, y);
	}
}

// 绘制盘子
void DrawPlate(int x, int y)
{
	SelectObject(bufdc, hbm_plate);
	MyTransparentBlt2(mdc, x, y, sz_plate.first, sz_plate.second, bufdc, 0, 0, sz_plate.first, sz_plate.second, RGB(245, 183, 97));
}

// 绘制材料箱
void DrawSrcBox(int id, int x, int y)
{
	hp = CreatePen(PS_SOLID, 1, RGB(181, 120, 63));
	hbr = CreateSolidBrush(RGB(181, 120, 63));
	SelectObject(mdc, hp);
	SelectObject(mdc, hbr);
	Rectangle(mdc, x, y, x + table_len + 1, y + table_len + 1);
	DeleteObject(hp);
	DeleteObject(hbr);

	hp = CreatePen(PS_SOLID, 2, RGB(95, 57, 27));
	hbr = CreateSolidBrush(RGB(139, 86, 50));
	SelectObject(mdc, hp);
	SelectObject(mdc, hbr);
	Rectangle(mdc, x + 10, y + 10, x + table_len + 1 - 10, y + table_len + 1 - 10);
	DeleteObject(hp);
	DeleteObject(hbr);

	hp = CreatePen(PS_SOLID, 2, RGB(226, 204, 180));
	hbr = CreateSolidBrush(RGB(226, 204, 180));
	SelectObject(mdc, hp);
	SelectObject(mdc, hbr);
	Ellipse(mdc, x + 13, y + 13, x + table_len - 1 - 13, y + table_len - 1 - 13);
	DeleteObject(hp);
	DeleteObject(hbr);

	SelectObject(bufdc, hbm_src[id]);
	MyTransparentBlt2(mdc, x + 35 - 20, y + 35 - 20, sz_src[id].first, sz_src[id].second, bufdc, 0, 0, sz_src[id].first, sz_src[id].second, RGB(240, 208, 153));
}

// 绘制垃圾桶
void DrawTrashcan(int x, int y)
{
	hp = CreatePen(PS_SOLID, 1, RGB(208, 185, 143));
	hbr = CreateSolidBrush(RGB(181, 153, 104));
	SelectObject(mdc, hp);
	SelectObject(mdc, hbr);
	Rectangle(mdc, x, y, x + table_len + 1, y + table_len + 1);
	DeleteObject(hp);
	DeleteObject(hbr);

	hp = CreatePen(PS_SOLID, 10, RGB(55, 59, 45));
	hbr = CreateSolidBrush(RGB(27, 30, 23));
	SelectObject(mdc, hp);
	SelectObject(mdc, hbr);
	Rectangle(mdc, x + 17, y + 17, x + table_len + 1 - 17, y + table_len + 1 - 17);
	DeleteObject(hp);
	DeleteObject(hbr);
}

// 绘制传送带
void DrawConveyorbelt(int way, int x, int y)
{
	hp = CreatePen(PS_SOLID, 1, RGB(218, 216, 217));
	hbr = CreateSolidBrush(RGB(218, 216, 217));
	SelectObject(mdc, hp);
	SelectObject(mdc, hbr);
	if (way == 1 || way == 2)
	{
		Rectangle(mdc, x, y, x + (table_len + 1) * 2, y + table_len + 1);
	}
	if (way == 3 || way == 4)
	{
		Rectangle(mdc, x, y, x + table_len + 1, y + 2 * (table_len + 1));
	}
	DeleteObject(hp);
	DeleteObject(hbr);

	// 传送带上箭头部分
	hp = CreatePen(PS_SOLID, 1, RGB(67, 64, 68));
	hbr = CreateSolidBrush(RGB(70, 68, 73));
	SelectObject(mdc, hp);
	SelectObject(mdc, hbr);
	if (way == 1)
	{
		POINT pt[3] = { {x + table_len + 1 - 12, y + table_len + 1 - 15}, {x + table_len + 1 + 12, y + table_len + 1 - 15}, { x + table_len + 1, y + table_len + 1 - 31} };
		Polygon(mdc, pt, 3);
		for (auto& t : pt) t.y -= 28;
		Polygon(mdc, pt, 3);
	}
	else if (way == 2)
	{
		POINT pt[3] = { {x + table_len + 1 - 12, y + 15}, {x + table_len + 1 + 12, y + 15}, { x + table_len + 1, y + 31} };
		Polygon(mdc, pt, 3);
		for (auto& t : pt) t.y += 28;
		Polygon(mdc, pt, 3);
	}
	else if (way == 3)
	{
		POINT pt[3] = { {x + table_len + 1 - 15, y + table_len + 1 - 12}, {x + table_len + 1 - 15, y + table_len + 1 + 12}, { x + table_len + 1 - 31, y + table_len + 1} };
		Polygon(mdc, pt, 3);
		for (auto& t : pt) t.x -= 28;
		Polygon(mdc, pt, 3);
	}
	else if (way == 4)
	{
		POINT pt[3] = { {x + 15, y + table_len + 1 - 12}, {x + 15, y + table_len + 1 + 12}, { x + 31, y + table_len + 1} };
		Polygon(mdc, pt, 3);
		for (auto &t : pt) t.x += 28;
		Polygon(mdc, pt, 3);
	}
	DeleteObject(hp);
	DeleteObject(hbr);

	hp = CreatePen(PS_SOLID, 1, RGB(213, 193, 143));
	hbr = CreateSolidBrush(RGB(137, 113, 75));
	SelectObject(mdc, hp);
	SelectObject(mdc, hbr);
	if (way == 1 || way == 2)
	{
		RoundRect(mdc, x, y, x + (table_len + 1) * 2, y + 7, 5, 5);
		RoundRect(mdc, x, y + table_len + 1 - 7, x + (table_len + 1) * 2, y + table_len + 1, 5, 5);
		RoundRect(mdc, x, y - 2, x + 10, y + table_len + 1 + 2, 5, 5);
		RoundRect(mdc, x + (table_len + 1) * 2 - 10, y - 2, x + (table_len + 1) * 2, y + table_len + 1 + 2, 5, 5);
	}
	else if (way == 3 || way == 4)
	{
		RoundRect(mdc, x, y, x + 7, y + 2 * (table_len + 1), 5, 5);
		RoundRect(mdc, x + table_len + 1 - 7, y, x + table_len + 1, y + 2 * (table_len + 1), 5, 5);
		RoundRect(mdc, x - 2, y, x + table_len + 1 + 2, y + 10, 5, 5);
		RoundRect(mdc, x - 2, y + 2 * (table_len + 1) - 10, x + table_len + 1 + 2, y + 2 * (table_len + 1), 5, 5);
	}
	DeleteObject(hp);
	DeleteObject(hbr);
}

// 绘制碗柜
void DrawCupBoard(int x, int y)
{
	hp = CreatePen(PS_SOLID, 1, RGB(240, 214, 142));
	hbr = CreateSolidBrush(RGB(97, 75, 95));
	SelectObject(mdc, hp);
	SelectObject(mdc, hbr);
	Rectangle(mdc, x, y, x + table_len + 1, y + table_len + 1);
	DeleteObject(hp);
	DeleteObject(hbr);

	hp = CreatePen(PS_SOLID, 1, RGB(86, 77, 64));
	hbr = CreateSolidBrush(RGB(86, 77, 64));
	SelectObject(mdc, hp);
	SelectObject(mdc, hbr);
	Rectangle(mdc, x + 5, y + 5, x + table_len + 1 - 5, y + table_len + 1 - 5);
	DeleteObject(hp);
	DeleteObject(hbr);

	hp = CreatePen(PS_SOLID, 1, RGB(200, 171, 137));
	hbr = CreateSolidBrush(RGB(200, 171, 137));
	SelectObject(mdc, hp);
	SelectObject(mdc, hbr);
	Rectangle(mdc, x + 8, y + 8, x + table_len + 1 - 8, y + table_len + 1 - 8);
	DeleteObject(hp);
	DeleteObject(hbr);

	hp = CreatePen(PS_SOLID, 1, RGB(141, 124, 157));
	hbr = CreateSolidBrush(RGB(141, 124, 157));
	SelectObject(mdc, hp);
	SelectObject(mdc, hbr);
	Ellipse(mdc, x + 10, y + 10, x + table_len + 1 - 10, y + table_len + 1 - 10);
	DeleteObject(hp);
	DeleteObject(hbr);
}

// 绘制切菜台
void DrawCutBoard(int x, int y, int knife)
{
	hp = CreatePen(PS_SOLID, 1, RGB(105, 85, 72));
	hbr = CreateSolidBrush(RGB(178, 168, 150));
	SelectObject(mdc, hp);
	SelectObject(mdc, hbr);
	RoundRect(mdc, x + 5, y + 10, x + 65, y + 60, 8, 8);
	DeleteObject(hp);
	DeleteObject(hbr);

	hp = CreatePen(PS_SOLID, 1, RGB(105, 85, 72));
	hbr = CreateSolidBrush(RGB(252, 185, 89));
	SelectObject(mdc, hp);
	SelectObject(mdc, hbr);
	RoundRect(mdc, x + 12, y + 20, x + 17, y + 50, 3, 3);
	RoundRect(mdc, x + 53, y + 20, x + 58, y + 50, 3, 3);
	DeleteObject(hp);
	DeleteObject(hbr);

	if (knife != 2)
	{
		SelectObject(bufdc, hbm_knife);
		MyTransparentBlt2(mdc, x + 12, y + 10, sz_knife.first, sz_knife.second, bufdc, 0, 0, sz_knife.first, sz_knife.second, RGB(200, 200, 200));
	}
}

// 错误 - 0没切 1没装盘 2没订单
void ErrorDispose(int id, int xx, int yy)
{
	SelectObject(bufdc, hbm_error[id]);
	MyTransparentBlt2(mdc, xx, yy, sz_error[id].first, sz_error[id].second, bufdc, 0, 0, sz_error[id].first, sz_error[id].second, RGB(255, 255, 255));
}

// 成功完成
void SuccessMeal(int posx, int posy)
{
	SelectObject(bufdc, hbm_success);
	MyTransparentBlt2(mdc, posx, posy, sz_success.first, sz_success.second, bufdc, 0, 0, sz_success.first, sz_success.second, RGB(217, 200, 139));
}

// 丢分
void LoseScore(int posx, int posy)
{
	SelectObject(bufdc, hbm_lose);
	MyTransparentBlt2(mdc, posx, posy, sz_lose.first, sz_lose.second, bufdc, 0, 0, sz_lose.first, sz_lose.second, RGB(217, 200, 139));
}

// 送餐程序处理
void DealDelivery(int id, int fd)
{
	if (fd == 1 || fd == 2) // 菜没切
	{
		message.push_back({ 0, GetTickCount(), per[id].xxn, per[id].yyn });
		return;
	}
	else if (fd == 3 || fd == 4) // 菜没装盘
	{
		message.push_back({ 1, GetTickCount(), per[id].xxn, per[id].yyn });
		return;
	}
	int flag = -1, tmp = -1;
	fd = (fd == 5 ? 0 : 1);
	for (auto t : ml)
	{
		tmp++; // 统计订单数
		// 订单已结束 或者 不是一样的菜
		if (t.st == false || t.id != fd) continue;
		// 找到了一样的菜
		flag = tmp;
		break;
	}
	if (flag == -1) // 当前订单栏没有一样的菜
	{
		message.push_back({ 2, GetTickCount(), per[id].xxn, per[id].yyn });
	}
	else // 成功做完这个订单
	{
		ml[tmp].st = false;
		per[id].food = 0;
		score += 20;
		message.push_back({ 3, GetTickCount(), per[id].xxn, per[id].yyn });
		update_plate.push_back(GetTickCount());
		cnt_suc++;
	}
}

// 切菜过程进度条
void DrawCutProgress(int id, int start, int now)
{
	int x = per[id].posx;
	int y = per[id].posy - 25;

	hp = CreatePen(PS_SOLID, 1, RGB(171, 169, 144));
	hbr = CreateSolidBrush(RGB(171, 169, 144));
	SelectObject(mdc, hp);
	SelectObject(mdc, hbr);
	Rectangle(mdc, x + 2, y + 2, per[id].posx + sz_per[per[id].id][per[id].way][per[id].idx].first - 10 + 2, y + 25 + 2);
	DeleteObject(hp);
	DeleteObject(hbr);

	hp = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
	hbr = CreateSolidBrush(RGB(255, 255, 255));
	SelectObject(mdc, hp);
	SelectObject(mdc, hbr);
	Rectangle(mdc, x, y, per[id].posx + sz_per[per[id].id][per[id].way][per[id].idx].first - 10, y + 25);
	DeleteObject(hp);
	DeleteObject(hbr);

	hp = CreatePen(PS_SOLID, 1, RGB(57, 164, 42));
	hbr = CreateSolidBrush(RGB(57, 164, 42));
	SelectObject(mdc, hp);
	SelectObject(mdc, hbr);
	Rectangle(mdc, x + 5, y + 5, per[id].posx + 1.0 * (sz_per[per[id].id][per[id].way][per[id].idx].first - 20 - 10) * (now - start) / cut_time_len, y + 25 - 5);
	DeleteObject(hp);
	DeleteObject(hbr);
}

// 切菜
void Cutting(int id, int xx, int yy)
{
	if (per[id].st != CUT)
	{
		per_cut_time[id] = GetTickCount();
		per[id].st = CUT;
	}
	DrawCutProgress(id, per_cut_time[id], cur_time);
	if ((int)cur_time - per_cut_time[id] >= 3000)
	{
		per[id].st = STAND;
		food[xx][yy] += 2;
	}
}

// 绘制消息
void DrawMessage()
{
	for (int i = 0; i < message.size(); i++)
	{
		if ((int)cur_time - (int)message[i].time_start >= 1000)
		{
			message.erase(message.begin() + i);
			i--;
		}
		else if (message[i].id <= 2) ErrorDispose(message[i].id, 280 + message[i].posy * (table_len + 1), 190 + message[i].posx * (table_len + 1));
		else if (message[i].id == 3) SuccessMeal(280 + message[i].posy * (table_len + 1), 190 + message[i].posx * (table_len + 1));
		else if (message[i].id == 4) LoseScore(message[i].posx, message[i].posy);
	}
}

// 更新碗柜上干净的盘子
void UpdateNewPlate(int i, int j)
{
	for (int z = 0; z < update_plate.size(); z++)
	{
		if ((int)cur_time - (int)update_plate[z] >= 4000)
		{
			pt[i][j]++;
			update_plate.erase(update_plate.begin() + z);
			z--;
		}
	}
}

// 结算界面星星
void DrawStar(int cnt)
{
	int posx = 530, posy = 215;
	for (int i = 0; i < cnt; i++)
	{
		SelectObject(bufdc, hbm_star);
		MyTransparentBlt2(mdc, posx, posy, sz_star.first, sz_star.second, bufdc, 0, 0, sz_star.first, sz_star.second, RGB(234, 231, 220));
		posx += 170;
	}
}

// 结算界面分数
void DrawScore(int score)
{
	std::string s = std::to_string(score);
	std::reverse(s.begin(), s.end());
	int posx = 975;
	for (int i = 0; i < s.size(); i++)
	{
		int c = s[i] - '0';
		SelectObject(bufdc, hbm_num_s[c]);
		MyTransparentBlt2(mdc, posx, 471, sz_num_s[c].first, sz_num_s[c].second, bufdc, 0, 0, sz_num_s[c].first, sz_num_s[c].second, RGB(234, 231, 220));
		posx -= 50;
	}
}

// 结算界面订单数
void DrawMealCnt(int cnt1, int cnt2)
{
	std::string s1 = std::to_string(cnt1);
	std::reverse(s1.begin(), s1.end());
	int posx = 980;
	for (int i = 0; i < s1.size(); i++)
	{
		int c = s1[i] - '0';
		SelectObject(bufdc, hbm_numres[c]);
		MyTransparentBlt2(mdc, posx, 365, sz_numres[c].first, sz_numres[c].second, bufdc, 0, 0, sz_numres[c].first, sz_numres[c].second, RGB(234, 231, 220));
		posx -= 25;
	}
	std::string s2 = std::to_string(cnt2);
	std::reverse(s2.begin(), s2.end());
	posx = 980;
	for (int i = 0; i < s2.size(); i++)
	{
		int c = s2[i] - '0';
		SelectObject(bufdc, hbm_numres[c]);
		MyTransparentBlt2(mdc, posx, 415, sz_numres[c].first, sz_numres[c].second, bufdc, 0, 0, sz_numres[c].first, sz_numres[c].second, RGB(234, 231, 220));
		posx -= 25;
	}
}

// 结算界面下方按键
void DrawResButton()
{
	int posx = 300;
	for (int i = 0; i < 2; i++)
	{
		SelectObject(bufdc, hbm_bt[i][res_button_st[i]]);
		MyTransparentBlt2(mdc, posx, 670, sz_bt[i].first, sz_bt[i].second, bufdc, 0, 0, sz_bt[i].first, sz_bt[i].second, RGB(145, 145, 145));
		posx += 600;
	}
}

// 隐藏终端
void hideConsoleWindow()
{
	::ShowWindow(::GetConsoleWindow(), SW_HIDE); // 隐藏控制台窗口
}

// 视频进度更新
void UpdateProgressBar() {
	if (!pMediaSeeking) return;

	LONGLONG current = 0, duration = 0;
	if (SUCCEEDED(pMediaSeeking->GetCurrentPosition(&current)) &&
		SUCCEEDED(pMediaSeeking->GetDuration(&duration)) &&
		duration > 0) {
		int pos = static_cast<int>((current * 100) / duration);
		SendMessage(hProgressBar, PBM_SETPOS, pos, 0);
	}
}

// 视频播放
void PlayVideo(HWND hwnd, LPCWSTR filename) {
	// 初始化 COM
	HRESULT hr = CoInitialize(NULL);

	// 创建过滤器图形管理器
	hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**)&pGraphBuilder);

	// 添加视频解码器
	hr = pGraphBuilder->QueryInterface(IID_IMediaControl, (void**)&pMediaControl);

	// 渲染视频文件
	hr = pGraphBuilder->RenderFile(filename, NULL);

	// 获取视频窗口接口
	IVideoWindow* pVideoWindow = NULL;
	hr = pGraphBuilder->QueryInterface(IID_IVideoWindow, (void**)&pVideoWindow);

	// 获取应用程序窗口的大小
	RECT clientRect;
	GetClientRect(hwnd, &clientRect);
	int clientWidth = clientRect.right - clientRect.left;
	int clientHeight = clientRect.bottom - clientRect.top;

	// 计算视频窗口的尺寸
	int videoWidth = 2160, videoHeight = 1440; // 假设视频的宽度和高度已知
	int targetWidth = videoWidth / 2;
	int targetHeight = videoHeight / 2;

	// 设置视频窗口的位置和大小
	pVideoWindow->put_Left((clientWidth - targetWidth) / 2);
	pVideoWindow->put_Top((clientHeight - targetHeight) / 2 - 30);
	pVideoWindow->put_Width(targetWidth);
	pVideoWindow->put_Height(targetHeight);

	// 设置视频窗口的父窗口并显示视频
	hr = pVideoWindow->put_Owner((OAHWND)hwnd);
	if (FAILED(hr)) {
		std::cerr << "Failed to set Video Window owner." << std::endl;
		pVideoWindow->Release();
		pMediaControl->Release();
		pGraphBuilder->Release();
		CoUninitialize();
		return;
	}

	// 设置视频窗口样式
	pVideoWindow->put_WindowStyle(WS_CHILD | WS_CLIPSIBLINGS);
	pVideoWindow->put_Visible(OATRUE);

	// 开始播放视频
	hr = pMediaControl->Run();

	// 获取视频播放位置接口
	hr = pGraphBuilder->QueryInterface(IID_IMediaSeeking, (void**)&pMediaSeeking);

	// 获取视频事件接口
	hr = pGraphBuilder->QueryInterface(IID_IMediaEvent, (void**)&pMediaEvent);

	// 创建进度条控件
	InitCommonControls();
	hProgressBar = CreateWindowEx(0, PROGRESS_CLASS, NULL, WS_CHILD | WS_VISIBLE | PBS_SMOOTH,
		10, clientHeight - 30, clientWidth - 20, 20, hwnd, NULL, NULL, NULL);

	// 启动一个线程来更新进度条
	CreateThread(NULL, 0, [](LPVOID) -> DWORD {
		while (true) {
			UpdateProgressBar();
			Sleep(100);
		}
		return 0;
		}, NULL, 0, NULL);

	// 等待视频播放完成
	long eventCode;
	hr = pMediaEvent->WaitForCompletion(INFINITE, &eventCode);

	// 释放资源
	pMediaEvent->Release();
	pMediaSeeking->Release();
	pVideoWindow->Release();
	pMediaControl->Release();
	pGraphBuilder->Release();

	// 释放 COM 资源
	CoUninitialize();
}

// 桌子等物品
void DrawCube()
{
	// 材料箱
	DrawTable(30, 100);
	DrawSrcBox(1, 30, 100);

	DrawTable(150, 100);
	DrawSrcBox(2, 150, 100);

	// 垃圾桶
	DrawTable(30, 210);
	DrawTrashcan(30, 210);

	// 切菜板
	DrawTable(150, 210);
	DrawCutBoard(150, 210, 0);

	// 碗柜
	DrawTable(30, 320);
	DrawCupBoard(30, 320);

	// 盘子
	DrawPlate(150, 320);

	// 传送带
	DrawConveyorbelt(3, 30, 430);
	DrawConveyorbelt(4, 150, 430);
	DrawConveyorbelt(1, 50, 590);
	DrawConveyorbelt(2, 50, 680);

	// 桌子
	DrawTable(30, 790);
}

// 物品选种边框
void DrawBlame()
{
	if (mouse_createmap.first != 0)
	{
		int posx = 0, posy = 0;
		if (mouse_createmap.first >= 1 && mouse_createmap.first <= 8)
		{
			if (mouse_createmap.first % 2 == 1) posx = 30;
			else posx = 150;
			posy = 100 + (mouse_createmap.first - 1) / 2 * 110;
		}
		else if (mouse_createmap.first == 9) posx = 50, posy = 590;
		else if (mouse_createmap.first == 10) posx = 50, posy = 680;
		else if (mouse_createmap.first == 11) posx = 30, posy = 790;

		hp = CreatePen(PS_DASH, 4, RGB(255, 255, 255));
		SelectObject(mdc, hp);
		POINT pt[5] = { {posx - 5, posy - 5}, {posx + table_len + 1 + 5, posy - 5}, {posx + table_len + 1 + 5, posy + table_len + 1 + 13 + 5}, {posx - 5, posy + table_len + 1 + 13 + 5}, {posx - 5, posy - 5} };
		if (mouse_createmap.first == 7 || mouse_createmap.first == 8)
		{
			pt[1] = { posx + table_len + 1 + 5, posy - 5 };
			pt[2] = { posx + table_len + 1 + 5, posy + (table_len + 1) * 2 + 5 };
			pt[3] = { posx - 5, posy + (table_len + 1) * 2 + 5 };
		}
		else if (mouse_createmap.first == 9 || mouse_createmap.first == 10)
		{
			pt[1] = { posx + (table_len + 1) * 2 + 5, posy - 5 };
			pt[2] = { posx + (table_len + 1) * 2 + 5, posy + table_len + 1 + 5 };
			pt[3] = { posx - 5, posy + table_len + 1 + 5 };
		}
		else if (mouse_createmap.first == 6)
		{
			pt[2] = { posx + table_len + 1 + 5, posy + table_len + 1 + 5 };
			pt[3] = { posx - 5, posy + table_len + 1 + 5 };
		}
		Polyline(mdc, pt, 5);
		DeleteObject(hp);
	}
}

// 组件栏
void DrawSub()
{
	hp = CreatePen(PS_SOLID, 1, RGB(168, 214, 230));
	hbr = CreateSolidBrush(RGB(168, 214, 230));
	SelectObject(mdc, hp);
	SelectObject(mdc, hbr);
	Rectangle(mdc, 0, 50, 260, 890);
	DeleteObject(hp);
	DeleteObject(hbr);

	DrawCube();
	DrawBlame();
}

// 画出已有地图
void DrawCurMap()
{
	bool stPaintPer[2] = { false, false };
	for (int i = 0; i < 9; i++)
	{
		for (int j = 0; j < 13; j++)
		{
			for (int k = 0; k < cnt_per; k++)
			{
				if (stPaintPer[k]) continue;
				if ((int)((per[k].posy + sz_per[per[k].id][per[k].way][per[k].idx].second - 190) / table_len) == i)
				{
					stPaintPer[k] = true;
					DrawPerson(k);
				}
			}
			if (g_lst[mapId][i][j]) DrawTable(280 + j * (table_len + 1), 190 + i * (table_len + 1));
			if (box_lst[mapId][i][j])
			{
				DrawTable(280 + j * (table_len + 1), 190 + i * (table_len + 1));
				DrawSrcBox(box_lst[mapId][i][j], 280 + j * (table_len + 1), 190 + i * (table_len + 1));
			}
			if (trash_lst[mapId][i][j])
			{
				DrawTable(280 + j * (table_len + 1), 190 + i * (table_len + 1));
				DrawTrashcan(280 + j * (table_len + 1), 190 + i * (table_len + 1));
			}
			if (i > 0 && belt_lst[mapId][i][j] && belt_lst[mapId][i - 1][j] == belt_lst[mapId][i][j])
			{
				DrawTable(280 + j * (table_len + 1), 190 + (i - 1) * (table_len + 1));
				DrawTable(280 + j * (table_len + 1), 190 + i * (table_len + 1));
				DrawConveyorbelt(belt_lst[mapId][i - 1][j], 280 + j * (table_len + 1), 190 + (i - 1) * (table_len + 1));
			}
			else if (j > 0 && belt_lst[mapId][i][j - 1] && belt_lst[mapId][i][j] == belt_lst[mapId][i][j - 1])
			{
				DrawTable(280 + (j - 1) * (table_len + 1), 190 + i * (table_len + 1));
				DrawTable(280 + j * (table_len + 1), 190 + i * (table_len + 1));
				DrawConveyorbelt(belt_lst[mapId][i][j - 1], 280 + (j - 1) * (table_len + 1), 190 + i * (table_len + 1));
			}
			if (cupboard_lst[mapId][i][j])
			{
				DrawTable(280 + j * (table_len + 1), 190 + i * (table_len + 1));
				DrawCupBoard(280 + j * (table_len + 1), 190 + i * (table_len + 1));
			}
			if (pt_lst[mapId][i][j]) DrawPlate(280 + j * (table_len + 1) + 3, 190 + i * (table_len + 1) + 3);
			if (cut_lst[mapId][i][j])
			{
				DrawTable(280 + j * (table_len + 1), 190 + i * (table_len + 1));
				DrawCutBoard(280 + j * (table_len + 1), 190 + i * (table_len + 1), cut_lst[mapId][i][j]);
			}
		}
	}
}

// 预设物品定位
void DrawNotDrop()
{
	if (x >= 280 && x <= 1190 && y >= 190 && y <= 820 && mouse_createmap.first != 0 && mouse_createmap.second != 0)
	{
		int x_idx = (y - 190) / (table_len + 1), y_idx = (x - 280) / (table_len + 1);
		if (mouse_createmap.first != 6)
		{
			if (!box_lst[mapId][x_idx][y_idx] && !g_lst[mapId][x_idx][y_idx] && !trash_lst[mapId][x_idx][y_idx] && !cut_lst[mapId][x_idx][y_idx] && !cupboard_lst[mapId][x_idx][y_idx] && !belt_lst[mapId][x_idx][y_idx])
			{
				if (mouse_createmap.first == 1 || mouse_createmap.first == 2)
				{
					DrawTable(y_idx * (table_len + 1) + 280, x_idx * (table_len + 1) + 190);
					DrawSrcBox(mouse_createmap.first, y_idx * (table_len + 1) + 280, x_idx * (table_len + 1) + 190);
					if (mouse_createmap.second == 2)
					{
						g_lst[mapId][x_idx][y_idx] = 1;
						box_lst[mapId][x_idx][y_idx] = mouse_createmap.first;
						mouse_createmap.first = mouse_createmap.second = 0;
					}
				}
				else if (mouse_createmap.first == 3)
				{
					DrawTable(y_idx * (table_len + 1) + 280, x_idx * (table_len + 1) + 190);
					DrawTrashcan(y_idx * (table_len + 1) + 280, x_idx * (table_len + 1) + 190);
					if (mouse_createmap.second == 2)
					{
						g_lst[mapId][x_idx][y_idx] = 1;
						trash_lst[mapId][x_idx][y_idx] = 1;
						mouse_createmap.first = mouse_createmap.second = 0;
					}
				}
				else if (mouse_createmap.first == 4)
				{
					DrawTable(y_idx * (table_len + 1) + 280, x_idx * (table_len + 1) + 190);
					DrawCutBoard(y_idx * (table_len + 1) + 280, x_idx * (table_len + 1) + 190, 0);
					if (mouse_createmap.second == 2)
					{
						g_lst[mapId][x_idx][y_idx] = 1;
						cut_lst[mapId][x_idx][y_idx] = 1;
						mouse_createmap.first = mouse_createmap.second = 0;
					}
				}
				else if (mouse_createmap.first == 5)
				{
					DrawTable(y_idx * (table_len + 1) + 280, x_idx * (table_len + 1) + 190);
					DrawCupBoard(y_idx * (table_len + 1) + 280, x_idx * (table_len + 1) + 190);
					if (mouse_createmap.second == 2)
					{
						g_lst[mapId][x_idx][y_idx] = 1;
						cupboard_lst[mapId][x_idx][y_idx] = 1;
						mouse_createmap.first = mouse_createmap.second = 0;
					}
				}
				else if (mouse_createmap.first >= 7 && mouse_createmap.first <= 10)
				{
					DrawTable(y_idx * (table_len + 1) + 280, x_idx * (table_len + 1) + 190);
					int way = 0;
					if (mouse_createmap.first >= 7 && mouse_createmap.first <= 8)
					{
						DrawTable(y_idx * (table_len + 1) + 280, (x_idx + 1) * (table_len + 1) + 190);
						way = mouse_createmap.first - 4;
					}
					else if (mouse_createmap.first >= 9 && mouse_createmap.first <= 10)
					{
						DrawTable((y_idx + 1) * (table_len + 1) + 280, x_idx * (table_len + 1) + 190);
						way = mouse_createmap.first - 8;
					}
					DrawConveyorbelt(way, y_idx * (table_len + 1) + 280, x_idx * (table_len + 1) + 190);
					if (mouse_createmap.second == 2)
					{
						if (mouse_createmap.first >= 7 && mouse_createmap.first <= 8)
						{
							g_lst[mapId][x_idx][y_idx] = g_lst[mapId][x_idx + 1][y_idx] = 1;
							belt_lst[mapId][x_idx][y_idx] = belt_lst[mapId][x_idx + 1][y_idx] = way;
						}
						else
						{
							g_lst[mapId][x_idx][y_idx] = g_lst[mapId][x_idx][y_idx + 1] = 1;
							belt_lst[mapId][x_idx][y_idx] = belt_lst[mapId][x_idx][y_idx + 1] = way;
						}
						mouse_createmap.first = mouse_createmap.second = 0;
					}
				}
				else if (mouse_createmap.first == 11)
				{
					DrawTable(y_idx * (table_len + 1) + 280, x_idx * (table_len + 1) + 190);
					DrawTable(y_idx * (table_len + 1) + 280, x_idx * (table_len + 1) + 190);
					if (mouse_createmap.second == 2)
					{
						g_lst[mapId][x_idx][y_idx] = 1;
						mouse_createmap.first = mouse_createmap.second = 0;
					}
				}
			}
		}
		else
		{
			if (!pt_lst[mapId][x_idx][y_idx] && g_lst[mapId][x_idx][y_idx] && !box_lst[mapId][x_idx][y_idx] && !trash_lst[mapId][x_idx][y_idx] && !cut_lst[mapId][x_idx][y_idx] && !cupboard_lst[mapId][x_idx][y_idx] && !belt_lst[mapId][x_idx][y_idx])
			{
				DrawPlate(y_idx * (table_len + 1) + 280, x_idx * (table_len + 1) + 190);
				if (mouse_createmap.second == 2)
				{
					pt_lst[mapId][x_idx][y_idx] = 1;
					mouse_createmap.first = mouse_createmap.second = 0;
				}
			}
		}
	}
}

// 取消放置
void DrawCancel()
{
	if (x >= 280 && x <= 1190 && y >= 190 && y <= 820 && mouse_cancel)
	{
		int x_idx = (y - 190) / (table_len + 1), y_idx = (x - 280) / (table_len + 1);
		if (g_lst[mapId][x_idx][y_idx] || box_lst[mapId][x_idx][y_idx] || trash_lst[mapId][x_idx][y_idx] || belt_lst[mapId][x_idx][y_idx] || cupboard_lst[mapId][x_idx][y_idx] || cut_lst[mapId][x_idx][y_idx])
		{
			SelectObject(bufdc, hbm_cancel);
			MyTransparentBlt2(mdc, 280 + y_idx * (table_len + 1), 190 + x_idx * (table_len + 1), sz_cancel.first, sz_cancel.second, bufdc, 0, 0, sz_cancel.first, sz_cancel.second, RGB(215, 165, 134));
		}
		if (mouse_cancel == 2)
		{
			if ((belt_lst[mapId][x_idx][y_idx] == 1 || belt_lst[mapId][x_idx][y_idx] == 2) && belt_lst[mapId][x_idx][y_idx + 1] == belt_lst[mapId][x_idx][y_idx + 1]) belt_lst[mapId][x_idx][y_idx + 1] = 0;
			else if ((belt_lst[mapId][x_idx][y_idx] == 3 || belt_lst[mapId][x_idx][y_idx] == 4) && belt_lst[mapId][x_idx + 1][y_idx] == belt_lst[mapId][x_idx][y_idx]) belt_lst[mapId][x_idx + 1][y_idx] = 0;
			g_lst[mapId][x_idx][y_idx] = box_lst[mapId][x_idx][y_idx] = trash_lst[mapId][x_idx][y_idx] = belt_lst[mapId][x_idx][y_idx] = cupboard_lst[mapId][x_idx][y_idx] = cut_lst[mapId][x_idx][y_idx] = 0;
			mouse_cancel = 0;
		}
	}
}

// 初始化游戏
void initMap()
{
	time_have = 30000;
	score = 0;

	for (int i = 0; i < 9; i++)
	{
		for (int j = 0; j < 13; j++)
		{
			g[i][j] = g_lst[mapId][i][j];
			pt[i][j] = pt_lst[mapId][i][j];
			box[i][j] = box_lst[mapId][i][j];
			trash[i][j] = trash_lst[mapId][i][j];
			belt[i][j] = belt_lst[mapId][i][j];
			cupboard[i][j] = cupboard_lst[mapId][i][j];
			food[i][j] = food_lst[mapId][i][j];
			cut[i][j] = cut_lst[mapId][i][j];
		}
	}
	
	for (int i = 0; i < cnt_per; i++)
	{
		per[i].way = Q;
		per[i].food = 0;
		if (i == 0) per[i].posx = 400, per[i].posy = 300;
		else per[i].posx = 400, per[i].posy = 500;
	}

	ml.clear();
	update_plate.clear();
	message.clear();


}