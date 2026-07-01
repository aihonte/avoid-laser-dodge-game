#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <Windows.h>
#include <time.h>
#pragma warning(disable:4996)//


// 색상 정의
#define BLACK	0
#define BLUE1	1
#define GREEN1	2
#define CYAN1	3
#define RED1	4
#define MAGENTA1 5
#define YELLOW1	6
#define GRAY1	7
#define GRAY2	8
#define BLUE2	9
#define GREEN2	10
#define CYAN2	11
#define RED2	12
#define MAGENTA2 13
#define YELLOW2	14
#define WHITE	15

#define PLAYER "♣"
#define BLANK "  "

#define ESC 0x1b
#define ENTER 13

#define SPECIAL1 0xe0
#define SPECIAL2 0x00

#define UP  0x48
#define DOWN 0x50
#define LEFT 0x4b
#define RIGHT 0x4d

#define WIDTH 80
#define HEIGHT 24

#define BOX_LEFT   33
#define BOX_TOP    10
#define BOX_RIGHT  43
#define BOX_BOTTOM 15
#define LASER 4
#define TB_LASER_LEN 12
#define LR_LASER_LEN 8
#define LASER_MAX_SYNC 20
#define LASER_MINI_SYNC 5
#define HEART "♥"  
#define SLOW "∞"


int Delay = 50;
int keep_moving = 1; // 1:계속이동, 0:한칸씩이동.

int frame_count = 0; // game 진행 frame count 로 속도 조절용으로 사용된다.
int player_sync = 1;// 플레이어의 속도제어를 위한 변수
int laser_sync_T = 15; // 위 / 아래 레이저의 속도제어를 위한 변수
int laser_sync_B = 15; // 좌 / 우 레이저의 속도제어를 위한 변수
int laser_sync_L = 10; // 좌 / 우 레이저의 속도제어를 위한 변수
int laser_sync_R = 10; // 좌 / 우 레이저의 속도제어를 위한 변수
int player_x, player_y; // player_move()함수에서 새로 갱신한 좌표를 저장하기 위한 변수

int run_time; // 현재 진행시간을 저장하는 변수
int start_time = 0;// play를 시작한 시간을 가짐
int end_time;// play가 끝난 시점의 시간을 가짐
int k = 0;//
char play_time[30]; //게임 플레이 시간을 출력하기 위한 문자열 (정수를 문자열로 변환해서 저장할 때 사용)

int life = 0;// heart 아이템을 먹으면 레이저 한 번 닿아도 죽지 않도록 함
int slow_time; // slow 아이템을 먹은 시간을 저장할 변수
int slow_flag; // slow 아이템을  먹었음을 표기할 변수
int slow_interval = 3; // slow 아이템을 지속시간 3초를 저장하는 변수

int laser_x[LASER] = { 33,33,1,79 };// 상,하,좌,우 4개의 레이저 x좌표 
int laser_y[LASER] = { 0,24,9,9 };// 상,하,좌,우 4개의 레이저 y좌표

int item_create_time = 0;// 해당 아이템이 생성된 시간 
int item_interval = 3; // 아이템 유지시간 3초
int item_exist = 0;// 해당 아이템 존재유무를 위한 변수 선언
int items[WIDTH][HEIGHT] = { 0 }; // 1이면 Gold 있다는 뜻

int choose_item;
int item_x[4] = { 38, 38, 31, 45 }; // 아이템이 생성될 4개의 x좌표
int item_y[4] = { 9, 16, 13, 13 }; // 아이템이 생성될 4개의 y좌표

int heart_x, heart_y; // 하트 아이템에 대한 x,y 좌표
int slow_x, slow_y; // 슬로우 아이템에 대한 x,y 좌표

int speed_interval = 10; // 게임 시작 기준 10초마다 레이저 속도를 증가시키기 위함
int speed_count = 0; // 10초 단위로 속도를 높이 위한  카운트 변수


int hidden_index; // Hidden 화면 번호 0 or 1
HANDLE scr_handle[2]; // 화면 버퍼 변수

void scr_init()//  보이는 화면이랑 숨은 화면 2개를 만듦. // 메인에 1번만 설정하면 됨(게임 시작시)
{ // 화면 번갈아 쓴다고 신호주는 역할임
	int i;
	CONSOLE_CURSOR_INFO cci;

	cci.dwSize = 1;
	cci.bVisible = FALSE;
	// 화면 버퍼 2개를 만든다.
	for (i = 0; i < 2; i++) {
		scr_handle[i] = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
		// 커서를 숨긴다.
		SetConsoleCursorInfo(scr_handle[i], &cci);
	}
	// 0번 화면이 default
	SetConsoleActiveScreenBuffer(scr_handle[0]);
	hidden_index = 1; // 1번 화면이 Hidden Screen
}

void scr_switch()// 숨은 화면과 보이는 화면을 바꿔주는 역할. 매 프레임마다 끝에 작성. 
{// 깜빡임을 방지하기 위한 핵심. 완성된 화면을 한 번에 보여줌

	// hidden 을 active 로 변경한다.
	SetConsoleActiveScreenBuffer(scr_handle[hidden_index]);
	// active를 hidden으로 변경한다.
	hidden_index = !hidden_index; // 0 <-> 1 toggle
}

void scr_clear()// 숨어있는 화면을 비우는 역할. 남아있는 그림을 지워준다고 생각하면 됨. 반복문에 첫줄에 들어간다고 생각하면됨
{
	COORD Coor = { 0, 0 };
	DWORD dw;
	// hidden screen를 clear한다.
	// WIDTH*2 * HEIGHT 값은 [속성]에서 설정한 값과 정확히 같아야 한다.
	// 즉, 화면 속성에서 너비(W)=80, 높이(H)=25라면 특수 문자는 2칸씩 이므로 WIDTH=40, HEIGHT=25이다.
	FillConsoleOutputCharacter(scr_handle[hidden_index], ' ', WIDTH * 2 * HEIGHT, Coor, &dw);
}

void scr_release() // 두 개의 화면을 완전히 삭제 즉, 메모리 해제 / 프로그램 끝낼 때, 한 번 사용하면 됨
{
	CloseHandle(scr_handle[0]);
	CloseHandle(scr_handle[1]);
}

void printscr(char* str) // 커서 위치에 문자열 출력(직접 위치 지정 필요없을 떄)
{
	DWORD dw;
	// hidden screen에 gotoxy 되었다고 가정하고 print
	WriteFile(scr_handle[hidden_index], str, strlen(str), &dw, NULL);
}
void printxy(int x, int y, const char* str)//숨은 화면의(x, y) 위치에 원하는 문자열을 찍음
{// gotoxy() + printf() = printxy() 라고 생각하면 됨

	DWORD dw;
	COORD CursorPosition = { x, y };
	// hidden screen에 gotoxy + print
	SetConsoleCursorPosition(scr_handle[hidden_index], CursorPosition);
	WriteFile(scr_handle[hidden_index], str, strlen(str), &dw, NULL);
}

void gotoxy(int x, int y) //내가 원하는 위치로 커서 이동
{
	COORD pos; // Windows.h 에 정의
	pos.X = x;
	pos.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void textcolor(int fg_color, int bg_color)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), fg_color | bg_color << 4);
}

void textcolor_double(int fg_color, int bg_color) // 더블버퍼에서 색상을 바꿀 수 있도록함
{
	SetConsoleTextAttribute(scr_handle[hidden_index], fg_color | bg_color << 4);
}


void removeCursor(void) { // 커서를 안보이게 한다
	CONSOLE_CURSOR_INFO curInfo;
	GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curInfo);
	curInfo.bVisible = 0;
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curInfo);
}
void showCursor(void) { // 커서를 보이게 한다
	CONSOLE_CURSOR_INFO curInfo;
	GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curInfo);
	curInfo.bVisible = 1;
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curInfo);
}
void cls(int text_color, int bg_color) // 화면 지우기
{
	char cmd[100];
	system("cls");
	sprintf(cmd, "COLOR %x%x", bg_color, text_color);
	system(cmd);
}

void choose_menu(int x, int y) {

	if (x == 29) {
		textcolor(GREEN1, WHITE);
		gotoxy(29, y);
		printf("▶START");
		textcolor(BLACK, WHITE);
		gotoxy(43, y);
		printf("▶EXIT");
	}
	else {
		textcolor(BLACK, WHITE);
		gotoxy(29, y);
		printf("▶START");
		textcolor(GREEN1, WHITE);
		gotoxy(43, y);
		printf("▶EXIT");
	}
}

void choose_menu2(int x, int y) {

	if (x == 23) {
		textcolor(GREEN1, WHITE);
		gotoxy(23, y);
		printf("▶SCORE");
		textcolor(BLACK, WHITE);
		gotoxy(34, y);
		printf("▶RESTART");
		textcolor(BLACK, WHITE);
		gotoxy(48, y);
		printf("▶EXIT");
	}
	else if (x == 34) {
		textcolor(BLACK, WHITE);
		gotoxy(23, y);
		printf("▶SCORE");
		textcolor(GREEN1, WHITE);
		gotoxy(34, y);
		printf("▶RESTART");
		textcolor(BLACK, WHITE);
		gotoxy(48, y);
		printf("▶EXIT");
	}
	else {
		textcolor(BLACK, WHITE);
		gotoxy(23, y);
		printf("▶SCORE");
		textcolor(BLACK, WHITE);
		gotoxy(34, y);
		printf("▶RESTART");
		textcolor(GREEN1, WHITE);
		gotoxy(48, y);
		printf("▶EXIT");
	}
}


void draw_box(int x1, int y1, int x2, int y2, const char* ch, const char* ch2)
{
	int x, y;
	for (x = x1; x <= x2; x += 2) {//
		gotoxy(x, y1);
		printf("%s", ch);
		gotoxy(x, y2);
		printf("%s", ch);
	}
	for (y = y1; y <= y2; y += 2) {
		gotoxy(x1, y);
		printf("%s", ch2);
		gotoxy(x2, y);
		printf("%s", ch2);
	}
}


void put_player(int x, int y, const char* ch) {// 지정 x,y 값으로 이동해 문자열 출력
	gotoxy(x, y);
	printf("%s", ch);
}

void erase_player(int x, int y) { // 지정 x,y 값으로 이동해 공백 출력
	gotoxy(x, y);
	printf("%s", BLANK);

}

void put_player2(int x, int y, const char* ch) { // put_player() 기능을 더블 버퍼에서 사용하기 위함
	printxy(x, y, ch);
}

void erase_player2(int x, int y) { // erase_player()기능을 더블 버퍼에서 사용하기 위함
	printxy(x, y, BLANK);
}

void middle_box(int x1, int y1, int x2, int y2, const char* ch, const char* ch2)// 게임 시작시 중심부에 있는 사각형을 그림
{
	int x, y;
	int temp = x1;
	printxy(x1, y1, "┌"); // 왼쪽 상단의 모서리
	printxy(x2, y1, "┐"); // 오른쪽 상단의 모서리
	printxy(x1, y2, "└"); // 왼쪽 하단의 모서리
	printxy(x2, y2, "┘");  // 오른쪽 하단의 모서리

	x1 += 2;
	for (x = x1; x < x2; x += 2) { // 윗변과 아랫변을 출력함
		printxy(x, y1, ch);
		printxy(x, y2, ch);
	}

	y1++;

	for (y = y1; y < y2; y++) { // 좌변과 우변을 출력함
		printxy(temp, y, ch2);
		printxy(x2, y, ch2);
	}

}

void ascii_print() {
	int x = 16, y = 8;
	textcolor(MAGENTA1, WHITE);
	gotoxy(x, y + 0); printf("  _   _ _   _   _  __    _     _   __  ___  ___ \n");
	gotoxy(x, y + 1); printf(" / \\ | | | / \\ | ||  \\  | |   / \\ / _|| __|| o \\\n");
	gotoxy(x, y + 2); printf("| o || V |( o )| || o ) | |_ | o |\\_ \\| _| |   /\n");
	gotoxy(x, y + 3); printf("|_n_| \\_/  \\_/ |_||__/  |___||_n_||__/|___||_|\\\\\n");
}


void start_exit() {
	int i;
	int fg_color = 1;//텍스트 색에 대한 변수
	int newx = 29, newy = 13;// 새로운 x, y 좌표
	unsigned char ch;// 특수키 한 번에 1바이트씩 2번 들어오기 때문에  char로는 오류발생
	int pre = -1;// 이전의 x좌표 비교를 위해서

	choose_menu(newx, newy);

	while (1) {
		if (kbhit()) { // 키가 눌려있는지 확인
			ch = getch(); // 방향키 읽게 하기 위해서 getchar() 사용하지 않음, getch() 2바이트 입력값을 받는 특수키 처리에 필요
			//getch(): enter없이 바로 입력가능
			if (ch == ESC) break;// Enter Key 대신 아무키나 누르면 진행된다

			else if (ch == SPECIAL1) {//방향키만 사용하기에 SPECIAL2를 사용할 필요는 없음
				ch = getch();

				switch (ch) {
				case LEFT:
					newx = 29;
					break;
				case RIGHT:
					newx = 43;
					break;
				}//switch
			}

			if (newx != pre) {//기존의 위치와 같지 않을 때만 이동을 시킴
				choose_menu(newx, newy);
				pre = newx;
			}

			if (ch == ENTER) {
				if (pre == 29) {
					cls(BLACK, WHITE);
					gotoxy(28, 11);
					printf("게임을 시작하겠습니다");
					Sleep(1000);
					gotoxy(28, 11);
					printf("                    ");
					for (int i = 3; i > 0; i--) {
						gotoxy(40, 11);
						printf("%d", i);
						Sleep(1000);
					}
					break;
				}

				else {
					cls(BLACK, WHITE);
					gotoxy(30, 11);
					printf("게임을 종료합니다");
					gotoxy(0, 20);
					exit(1);

				}

			}//if(kbhit())

		}

		textcolor(WHITE, fg_color); // 화면색과 관계없이 글자색을 변경한다				
		if (fg_color < 0xf) {
			fg_color++;
		}

		else {
			fg_color = 1;
		}
		for (i = 0; i <= 6; i += 2) {
			draw_box(i * 2, i, 78 - i * 2, 22 - i, "★", "★");
			Sleep(16);
		}
	}
}

int create_item() {

	int r;
	r = rand() % 2 + 1;
	int index; // 짝을 이루는 고정 x,y를 저장하기 위한 인덱스 설정
	index = rand() % 4; // item의 위치를 정하기 위해서 item_ or _y배열의 인덱스 번호인 0~3을 랜덤으로 가짐

	if (r == 1) {
		if (items[item_x[index]][item_y[index]] != 1) { // 같은 위치라면 표기를 하지 않도록 조건을 걺
			items[item_x[index]][item_y[index]] = 1; // 아이템이 있는 위치는 위치했음을 표기
			heart_x = item_x[index]; // 정해진 위치의 x좌표값을 랜덤으로 갖는다
			heart_y = item_y[index]; // 정해진 위치의 y좌표값을 랜덤으로 갖는다.
			item_exist = 1; // 아이템이 존재하면 1을 값을 갖도록 함
			item_create_time = time(NULL); // slow 아이템 생성 시간 저장
			return 1;
		}
	}

	else {
		if (items[item_x[index]][item_y[index]] != 1) { // 같은 위치라면 표기를 하지 않도록 조건을 걺
			items[item_x[index]][item_y[index]] = 1; // 아이템이 있는 위치는 위치했음을 표기
			slow_x = item_x[index]; // 정해진 위치의 x좌표값을 랜덤으로 갖는다
			slow_y = item_y[index]; // 정해진 위치의 y좌표값을 랜덤으로 갖는다.
			item_exist = 1; // 아이템이 존재하면 1을 값을 갖도록 함
			item_create_time = time(NULL); // slow 아이템 생성 시간 저장
			return 2;
		}
	}

}

void top_laser() {//위->사각형의 윗변까지 이동하는 레이저 
	if (laser_y[0] == 9) laser_y[0] = 1; // 레이저의 위치가 사각형 윗변 앞까지 오면 다시 위에서 생성하도록 좌표를 갱신
	laser_y[0]++; // 레이저의 y좌표를 1씩 증가시켜 레이저의 위치가 내려가도록 함 (좌표설정) 
}

void bottom_laser() {//아래->사각형의 아랫변까지 이동하는 레이저 
	if (laser_y[1] == 16) laser_y[1] = 23; // 레이저의 위치가 사각형 아랫변 앞까지 오면 다시 아래에서 생성하도록 좌표를 갱신
	laser_y[1]--;  // 레이저의 y좌표를 1씩 감소시켜 레이저의 위치가 올라가도록 함 (좌표설정) 
}

void left_laser() {//왼-> 사각형의 왼쪽 변까지 이동하는 레이저 
	if (laser_x[2] == 31) laser_x[2] = 1; // 레이저의 위치가 사각형 왼쪽 변 앞까지 오면 다시 왼쪽 벽면에서 레이저를 생성하도록 좌표를 갱신
	laser_x[2] += 2; // 레이저의 X좌표를 2씩 증가시켜 레이저가 오른쪽 방향으로 이동하게 함 
}

void right_laser() { //왼-> 사각형의 왼쪽 변까지 이동하는 레이저 
	if (laser_x[3] == 45) laser_x[3] = 79; //레이저의 위치가 사각형 오른쪽 변 앞까지 오면 다시 오른쪽 벽면에서 레이저를 생성하도록 좌표를 갱신
	laser_x[3] -= 2; // 레이저의 좌표를 2씩 증가시켜 레이저가 왼쪽 방향으로 이동하게 함
}


void player_move() {// 플레이어가 중심부 사각형을 기준으로만 움직일 수 있게 만듦
	static int oldx = 38, oldy = 9;
	static int newx = 38, newy = 9;
	int keep_moving = 0;
	unsigned char ch;

	if (kbhit()) { // 키보드가 눌려있다면
		ch = getch();
		if (k == 0) {// 딱 한 번만 실행하도록 함
			start_time = time(NULL);// 키보드가 눌린 순간을 게임의 시작시간으로 저장
			k = 1;
		}
		run_time = time(NULL) - start_time;// 
		sprintf(play_time, "play_time : %03d", run_time);// 문자열을 출력하는 것이 아닌 저장만 하는 단계

		if (ch == ESC) exit(1); // ESC 종료

		if (ch == SPECIAL1) {// 특수키가 눌렸다면
			ch = _getch();
			switch (ch) { // 즉, 방향키가 눌렸다면
			case UP:
			case DOWN:
			case LEFT:
			case RIGHT:
				keep_moving = 1;
				break;
			default:
				keep_moving = 0;
			}//switch
		}// if방향키
	}//if kbhit

	if (keep_moving) {

		switch (ch) { // x,y의 범위를 정해서 플레이어의 이동을 사각형 겉테두리에서만 가능하도록 함
		case UP:
			if ((oldx == 31 || oldx == 45) && oldy > 9)
				newy = oldy - 1;
			break;
		case DOWN:
			if ((oldx == 31 || oldx == 45) && oldy < 16)
				newy = oldy + 1;
			break;
		case LEFT:
			if (oldx > 31 && (oldy == 9 || oldy == 16))
				newx = oldx - 1;
			break;
		case RIGHT:
			if (oldx < 45 && (oldy == 9 || oldy == 16))
				newx = oldx + 1;
			break;
		}//switch
		oldx = newx;// 새로운 x좌표를 갱신
		oldy = newy;// 새로운 y좌표를 갱신
	}//if

	player_x = newx;// 새로운 x좌표를 계속해서 갱신해서 메인에서 put_player2()에게 매개변수로 넘김 
	player_y = newy;// 새로운 y좌표를 계속해서 갱신해서 메인에서 put_player2()에게 매개변수로 넘김
}



int laser_hit() {// player의 좌표와 레이저의 좌표가 동일하면 return 1; 그렇지 않다면 return 0; 
	if (choose_item == 1) {

		for (int i = 0; i < TB_LASER_LEN; i += 2) { // top_laser와의 충돌 확인
			if (heart_x == laser_x[0] + i && heart_y == laser_y[0]) {
				return 1;
			}
		}

		for (int i = 0; i < TB_LASER_LEN; i += 2) { // bottom_laser와의 충돌 확인
			if (heart_x == laser_x[1] + i && heart_y == laser_y[1]) {
				return 1;
			}
		}

		for (int i = 0; i < LR_LASER_LEN; i++) { // left_laser와의 충돌 확인
			if (heart_x == laser_x[2] && heart_y == laser_y[2] + i) {
				return 1;
			}
		}

		for (int i = 0; i < LR_LASER_LEN; i++) { // right_laser와의 충돌 확인
			if (heart_x == laser_x[3] && heart_y == laser_y[3] + i) {
				return 1;
			}
		}
	}

	else if (choose_item == 2) {

		for (int i = 0; i < TB_LASER_LEN; i += 2) { // top_laser와의 충돌 확인
			if (slow_x == laser_x[0] + i && slow_y == laser_y[0]) {
				return 1;
			}
		}

		for (int i = 0; i < TB_LASER_LEN; i += 2) { // bottom_laser와의 충돌 확인
			if (slow_x == laser_x[1] + i && slow_y == laser_y[1]) {
				return 1;
			}
		}

		for (int i = 0; i < LR_LASER_LEN; i++) { // left_laser와의 충돌 확인
			if (slow_x == laser_x[2] && slow_y == laser_y[2] + i) {
				return 1;
			}
		}

		for (int i = 0; i < LR_LASER_LEN; i++) { // right_laser와의 충돌 확인
			if (slow_x == laser_x[3] && slow_y == laser_y[3] + i) {
				return 1;
			}
		}
	}

	return 0; // 이 4개의 조건을 확인했을 때, 레이저의 충돌이 없었다면 0을 반환
}

int gameover() {// player의 좌표와 레이저의 좌표가 동일하면 return 1; 그렇지 않다면 return 0; 

	for (int i = 0; i < TB_LASER_LEN; i += 2) { // top_laser와의 충돌 확인
		if (player_x == laser_x[0] + i && player_y == laser_y[0]) {
			return 1;
		}
	}

	for (int i = 0; i < TB_LASER_LEN; i += 2) { // bottom_laser와의 충돌 확인
		if (player_x == laser_x[1] + i && player_y == laser_y[1]) {
			return 1;
		}
	}

	for (int i = 0; i < LR_LASER_LEN; i++) { // left_laser와의 충돌 확인
		if (player_x == laser_x[2] && player_y == laser_y[2] + i) {
			return 1;
		}
	}

	for (int i = 0; i < LR_LASER_LEN; i++) { // right_laser와의 충돌 확인
		if (player_x == laser_x[3] && player_y == laser_y[3] + i) {
			return 1;
		}
	}
	return 0; // 이 4개의 조건을 확인했을 때, 레이저의 충돌이 없었다면 0을 반환
}

void gameover_print() { // gameover!출력 및 주변을 둘러싸는 상자들을 출력함

	int i;
	gotoxy(29, 6);
	for (i = 0; i < 10; i++) {// 둘러싸는 벽의 윗면의 벽을 그림
		textcolor(WHITE, BLUE1 + i);
		printf("%s", "■");
	}

	for (i = 0; i < 3; i++) {//둘러싸는 왼쪽 벽을 그림
		textcolor(WHITE, GRAY2 + i);
		gotoxy(29, 7 + i);
		printf("%s", "■");
	}
	gotoxy(29, 10);

	for (i = 0; i < 10; i++) {//둘러싸는 아랫쪽  벽을 그림
		textcolor(WHITE, BLUE1 + i);
		printf("%s", "■");
	}

	for (i = 0; i < 3; i++) {//둘러싸는 우측 벽을 그림
		textcolor(WHITE, GRAY2 + i);
		gotoxy(47, 7 + i);
		printf("%s", "■");
	}
	textcolor(RED1, YELLOW2);
	gotoxy(34, 8);
	printf("GAME OVER!"); //28,11의 위치로 찾아가 GAME OVER 출력
}


void over_design() {// 게임오버 출력과 주변 목록들을 피해 별을 랜덤으로 출력. 
	// score 확인 후, back을 눌러 ending 화면으로 돌아갈 때마다 별이 랜덤으로 출력

	int rand_x1 = 0, rand_y1 = 0;
	int rand_x2 = 0, rand_y2 = 0;
	int rand_x3 = 0, rand_y3 = 0;
	int control = 0;

	for (int i = 0; i < 15; i++) {// x,y의 범위를 제한함
		control = rand() % 4 + 1;// 1~4까지 수를 랜덤으로 출력
		rand_x1 = rand() % 27 + 1;// 중심부 기준 좌측에서의 x좌표
		rand_y1 = rand() % 4 + 1; // 중심부 y좌표 > y좌표
		rand_x2 = rand() % 31 + 49; // 중심부 기준 우측에서의 x좌표
		rand_y2 = rand() % 7 + 17; // 중심부 y좌표 < y좌표
		rand_x3 = rand() % 19 + 29; // 중심부의 x좌표
		rand_y3 = rand() % 23 + 1; // 모든 y좌표

		textcolor(i, WHITE);
		switch (control) {
		case 1:
			gotoxy(rand_x1, rand_y3);
			printf("%s", "☆");
			break;
		case 2:
			gotoxy(rand_x2, rand_y3);
			printf("%s", "☆");
			break;
		case 3:
			gotoxy(rand_x3, rand_y1);
			printf("%s", "☆");
			break;
		case 4:
			gotoxy(rand_x3, rand_y2);
			printf("%s", "☆");
			break;
		}
	}
}


void restart_exit() {
	int newx = 23, newy = 13;// 새로운 x, y 좌표
	unsigned char ch;// 특수키는 한 번에 1바이트씩 2번 들어오기 때문에  char로는 오류발생
	int pre = -1;// 이전의 x좌표 비교를 위해서
	int location[3] = { 23,34,48 }; // ending 화면에 출력할 목록 3개의 x좌표
	int index = 0;

	choose_menu2(newx, newy);// 3개의 목록을 출력시킴 : score, restart, exit

	while (1) {
		if (kbhit()) { // 키가 눌려있는지 확인
			ch = getch(); // 방향키 읽게 하기 위해서 getchar() 사용하지 않음, getch() 2바이트 입력값을 받는 특수키 처리에 필요
			//getch(): enter없이 바로 입력가능
			if (ch == ESC) break;// Enter Key 대신 아무키나 누르면 진행된다

			else if (ch == SPECIAL1) {//방향키만 사용하기에 SPECIAL2를 사용할 필요는 없음
				ch = getch();

				switch (ch) {
				case LEFT:
					switch (index) {
					case 1: index = 0; break;// 가운데 목록일 경우, 가장 좌측 목록의 인덱스를 갖도록
					case 2: index = 1; break;// 우측 목록일 경우, 가운데 목록의 인덱스를 갖도록
					}
					break;

				case RIGHT:
					switch (index) {
					case 0: index = 1; break; // 좌측 목록일 경우, 가운데 목록의 인덱스를 갖도록
					case 1: index = 2; break; // 가운데 목록일 경우, 우측 목록의 인덱스를 갖도록
					}
					break;
				}//switch
				newx = location[index]; // 인덱스의 값에 따른 목록의 x좌표를 저장
			}

			if (newx != pre) {//기존의 위치와 같지 않을 때만 이동을 시킴
				choose_menu2(newx, newy);
				pre = newx;
			}

			if (ch == ENTER) {
				if (pre == 23) {
					cls(BLACK, WHITE);
					gotoxy(29, 9);
					for (int i = 0; i < 10; i++) {
						textcolor(BLUE1 + i, WHITE);
						printf("%s", "▼");
					}

					gotoxy(30, 11);
					textcolor(BLACK, WHITE);
					printf("SCORE : %03d sec", run_time);
					gotoxy(29, 13);
					for (int i = 0; i < 10; i++) {
						textcolor(BLUE1 + i, WHITE);
						printf("%s", "▲");
					}

					textcolor(GREEN1, WHITE);
					gotoxy(31, 16);
					for (int i = 0; i < 16; i++) { printf("%s", "-"); }

					textcolor(GREEN1, WHITE);
					gotoxy(31, 18);
					for (int i = 0; i < 16; i++) { printf("%s", "-"); }

					textcolor(BLACK, WHITE);
					gotoxy(32, 17);
					printf("back : (ENTER)");// enter를 누르면 뒤로 다시 돌아가도록 설계

					unsigned char ch2;
					while (1) {
						if (kbhit()) {
							ch2 = getch();
							if (ch2 == ENTER) break;// enter가 눌리면 gameover 화면으로 돌아감
						}
					}

					cls(BLACK, WHITE);// 화면을 지우고
					gameover_print(); // 다시 gameover 화면을 출력
					over_design(); // 랜덤으로 ending 화면에 별을 출력
					restart_exit();// 3개의 선택 목록도 다시 출력

				}

				else if (pre == 34) {
					cls(BLACK, WHITE);
					gotoxy(28, 11);
					printf("게임을 재시작하겠습니다");
					Sleep(1000);

					gotoxy(28, 11);
					cls(BLACK, WHITE);
					for (int i = 3; i > 0; i--) {
						gotoxy(40, 11);
						printf("%d", i);
						Sleep(1000);
					}
					cls(BLACK, WHITE);
					break;
				}

				else {
					cls(BLACK, WHITE);
					gotoxy(28, 11);
					printf("게임을 종료합니다");
					gotoxy(0, 20);
					exit(1);
				}
			}
		}//if(kbhit())

	}
}


int main() {

	int r;
	srand(time(NULL));// 레이저를 
	removeCursor(); // 커서를 안보이게 한다
	ascii_print(); // 아스키 아트 활용;
	start_exit(); // 초기화면 츌력
	cls(BLACK, WHITE);
	scr_init();// 화면 2개 생성 : 본 화면과 숨은화면 생성

	while (1)
	{
		scr_clear();// 숨은화면을 청소

		if (frame_count % player_sync == 0) {//플레이어를  5 frame마다 이동하게 함
			textcolor_double(BLACK, GRAY2);
			printxy(0, 1, "play_time : 000");// 시간이 측정되기 전에는 아무 값도 출력이 되지 않기에 미리 문자를 출력
			player_move();// key가 눌린 순간분터 시간이 측정되기 때문에
			textcolor_double(BLACK, GRAY2);
			printxy(0, 1, play_time);// 측정되는 시간을 더블버퍼에 작성
		}

		if (slow_flag == 1) {// 슬로우 아이템을 먹고 3초가 지나면 다시 원상 복구

			if (time(NULL) - slow_time >= slow_interval) {// 먹은지 3초가 지났다면, laser_sync를 다시 2로 나눠 속도를 기존과 동일하게 한다
				laser_sync_T /= 2; // 다시 기존의 sync를 갖도록 2를 나눔
				laser_sync_B /= 2; // 다시 기존의 sync를 갖도록 2를 나눔
				laser_sync_L /= 2; // 다시 기존의 sync를 갖도록 2를 나눔
				laser_sync_R /= 2; // 다시 기존의 sync를 갖도록 2를 나눔
				slow_flag--; // 슬로우 아이템 먹은 것을 없앰. 즉, 초기화
			}
		}

		if ((time(NULL) - start_time) / speed_interval > speed_count) {// 게임시작 기준 10초가 지날 때마다 레이저의 속도를 증가시킴;
			laser_sync_T -= 5;  // 10초마다 레이저의 sync를 5 낮춰 속도를 증가하도록 한다
			laser_sync_B -= 5;  // 10초마다 레이저의 sync를 5 낮춰 속도를 증가하도록 한다
			laser_sync_L -= 5;  // 10초마다 레이저의 sync를 5 낮춰 속도를 증가하도록 한다
			laser_sync_R -= 5;  // 10초마다 레이저의 sync를 5 낮춰 속도를 증가하도록 한다
			if (laser_sync_T < LASER_MINI_SYNC) { laser_sync_T = LASER_MINI_SYNC; }
			if (laser_sync_B < LASER_MINI_SYNC) { laser_sync_B = LASER_MINI_SYNC; }
			if (laser_sync_L < LASER_MINI_SYNC) { laser_sync_L = LASER_MINI_SYNC; }
			if (laser_sync_R < LASER_MINI_SYNC) { laser_sync_R = LASER_MINI_SYNC; }
			speed_count++; // 10초, 20초, 30초처럼 딱 10초단위로 속도를 높이기 위해  
		}

		if (frame_count % laser_sync_T == 0) { // 상단 레이저를 frame마다 이동하게 함
			top_laser();
		}

		if (frame_count % laser_sync_B == 0) { // 하단 레이저를 frame마다 이동하게 함
			bottom_laser();
		}

		if (frame_count % laser_sync_L == 0) {// 좌축 레이저를 frame마다 이동하게 함
			left_laser();
		}

		if (frame_count % laser_sync_R == 0) {// 우측 레이저를 frame마다 이동하게 함
			right_laser();
		}

		if (item_exist == 0) {// 아이템이 하나도 없으면 생성하도록 함
			choose_item = create_item();// heart를 생성하면 return 1을 하고, slow를 생성하면 return 2를 함
		}

		else {// 아이템이 이미 있다면

			if (choose_item == 1 && (player_x == heart_x && player_y == heart_y)) { //heart 아이템과 충돌
				items[heart_x][heart_y] = 0; // 플레이어와 충돌하면 위치 지우기
				item_exist = 0; // 아이템 삭제
				life++;// 목숨을 증가시킴
			}

			else if (choose_item == 2 && (player_x == slow_x && player_y == slow_y)) {//slow 아이템과 충돌
				items[slow_x][slow_y] = 0; // 플레이어와 충돌하면 위치 지우기
				item_exist = 0; // 아이템 삭제
				laser_sync_T *= 2; // 위/ 아래 레이저의 sync를 2 곱해 속도를 줄어들게 함 
				laser_sync_B *= 2; // 위/ 아래 레이저의 sync를 2 곱해 속도를 줄어들게 함 
				laser_sync_L *= 2; // 좌/ 우 레이저의 sync를 2 곱해 속도를 줄어들게 함
				laser_sync_R *= 2; // 좌/ 우 레이저의 sync를 2 곱해 속도를 줄어들게 함
				slow_flag = 1; // 슬로우 아이템을 먹었음을 표시
				slow_time = time(NULL); // 슬로우 아이템을 먹은 시간을 표시
			}

			else if (laser_hit() == 1) { // 레이저와 아이템이 부딪혔다면 아이템을 소멸시킴

				if (choose_item == 1) {// heart 아이템과 레이저과 부딪혔다면 아이템을 소멸시킴
					items[heart_x][heart_y] = 0; // 아이템이 위치 표시값이 1을 -> 0으로 저장
					item_exist = 0; // 아이템 소멸을 표시함
				}

				else if (choose_item == 2) {// slow 아이템과 레이저과 부딪혔다면 아이템을 소멸시킴
					items[slow_x][slow_y] = 0;// 아이템이 위치 표시값이 1을 -> 0으로 저장
					item_exist = 0; // 아이템 소멸을 표시함
				}

			}
		}


		if (gameover()) {// 플레이어가 레이저에 닿았을 때,
			if (life > 0) {//life가 0보다 크다면 1을 감소시킴 
				life--; // 목숨 감소
				continue;//루프를 벗어나지 않고 다시 돌게함
			}
			else {
				break;// 만약 life가 0과 같거나 작다면 바로 while문을 벗어나면서 게임종료
			}
		}
		textcolor_double(BLACK, WHITE);
		middle_box(33, 10, 43, 15, "─", "│"); // 중심부의 사각형을 계속 그려냄

		textcolor_double(MAGENTA1, WHITE);
		put_player2(player_x, player_y, PLAYER);

		for (int j = 0; j < TB_LASER_LEN; j += 2) {// x축은 2바이트씩 차지하기에 2씩 증가
			textcolor_double(BLUE2, WHITE);
			printxy(laser_x[0] + j, laser_y[0], "▼"); // top_laser의 출력
		}

		for (int j = 0; j < TB_LASER_LEN; j += 2) { // x축은 2바이트씩 차지하기에 2씩 증가
			textcolor_double(GREEN2, WHITE);
			printxy(laser_x[1] + j, laser_y[1], "▲"); // bottom_laser의 출력
		}

		for (int j = 0; j < LR_LASER_LEN; j++) {// y축은 1바이트씩 차지하기에 1씩 증가
			textcolor_double(YELLOW2, WHITE);
			printxy(laser_x[2], laser_y[2] + j, "▶"); // left_laser의 출력
		}

		for (int j = 0; j < LR_LASER_LEN; j++) {// y축은 1바이트씩 차지하기에 1씩 증가
			textcolor_double(GRAY2, WHITE);
			printxy(laser_x[3], laser_y[3] + j, "◀"); // right_laser의 출력
		}

		if (choose_item == 1) {
			textcolor_double(RED2, WHITE);
			printxy(heart_x, heart_y, HEART);
		}

		else if (choose_item == 2) {
			textcolor_double(CYAN2, WHITE);
			printxy(slow_x, slow_y, SLOW);
		}

		frame_count++;
		scr_switch(); // 본 화면과 숨은화면을 변환시킨다
		Sleep(Delay);
	}
	end_time = time(NULL);// 게임이 종료된 시점에서의 시간을 저장
	run_time = end_time - start_time; // 게임이 종료된 시간 - 시작한 시간을 뺴서 총 플레이 시간을 저장
	scr_release();// 더블버퍼 해제(화면 2개에서 1개로 변함)
	cls(BLACK, WHITE);
	gameover_print();
	over_design();
	restart_exit();

	return 0;
}
