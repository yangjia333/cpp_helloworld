#include <iostream>
#include <Windows.h>
#include <chrono>
#include <vector>
#include <thread>
#include <mutex>
#include <algorithm>
#include <random>
#include<memory>

using namespace std;
using namespace std::chrono;

bool isover = false;
mutex m;

// 场景范围
const int W = 61;
const int H = 21;

// FUNCTIONS

// 清空屏幕
void clrscr(void)
{
	CONSOLE_SCREEN_BUFFER_INFO    csbiInfo;
	HANDLE    hConsoleOut;
	COORD    Home = { 0, 0 };
	DWORD    dummy;
	hConsoleOut = GetStdHandle(STD_OUTPUT_HANDLE);
	GetConsoleScreenBufferInfo(hConsoleOut, &csbiInfo);
	FillConsoleOutputCharacter(hConsoleOut, ' ',
		csbiInfo.dwSize.X * csbiInfo.dwSize.Y, Home,
		&dummy);
	csbiInfo.dwCursorPosition.X = 0;
	csbiInfo.dwCursorPosition.Y = 0;
	SetConsoleCursorPosition(hConsoleOut, csbiInfo.dwCursorPosition);
}
// 定位输出位置
void gotoxy(int x, int y)
{
	CONSOLE_SCREEN_BUFFER_INFO    csbiInfo;
	HANDLE    hConsoleOut;
	hConsoleOut = GetStdHandle(STD_OUTPUT_HANDLE);
	GetConsoleScreenBufferInfo(hConsoleOut, &csbiInfo);
	csbiInfo.dwCursorPosition.X = x;
	csbiInfo.dwCursorPosition.Y = y;
	SetConsoleCursorPosition(hConsoleOut, csbiInfo.dwCursorPosition);
}

// 清理某个特定位置，这里用于清理贪吃蛇的尾巴
void clrxy(int x, int y)
{
	gotoxy(x, y);
	cout << ' ';
}

void Settitle(int x, int y){
	gotoxy(x, y);
	cout << "贪吃虫";
	//gotoxy(x, y + 1);
	//cout << "by YJ";
}

// 蛇身上的节点
class node
{
public:
	node(int _x, int _y) :x(_x), y(_y) {}

	virtual void draw()
	{
		gotoxy(x, y);
		cout << "#";
	}
	int getx() const
	{
		return x;
	}
	int gety() const
	{
		return y;
	}
	void setx(int _x)
	{
		x = _x;
	}
	void sety(int _y)
	{
		y = _y;
	}
	// 偏移一个位置，而不是移动到
	void move(int _x, int _y)
	{
		x += _x;
		y += _y;
	}
	bool operator == (const node& a)
	{
		if (a.getx() == x && a.gety() == y)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
protected:
	int x;
	int y;
};

class node1 :public node{
public:
	node1(int _x, int _y) :node(_x, _y){
	}
	virtual void draw(){
		gotoxy(x, y);
		cout << "o";
	}
	virtual int getenergy(){
		return 1;
	}
};

class node2 :public node{
public:
	node2(int _x, int _y) :node(_x, _y){
	}
	virtual void draw(){
		gotoxy(x, y);
		cout << "@";
	}
};

// 围墙，由节点构成
class border
{
public:
	border(int _ux, int _uy,
		int _dx, int _dy)
		:ux(_ux), uy(_uy), dx(_dx), dy(_dy)
	{

		// 构建围墙
		for (int i = ux; i <= dx; ++i)
		{
			v.push_back(node(i, uy));
			v.push_back(node(i, dy));
		}
		for (int i = uy; i <= dy; ++i)
		{
			v.push_back(node(ux, i));
			v.push_back(node(dx, i));
		}
	}
public:
	void draw()
	{
		for (node& n : v)
		{
			n.draw();
		}
	}
private:
	int ux, uy;
	int dx, dy;
	vector<node> v;
};

enum DIRECTION { UP = 1, DOWN, LEFT, RIGHT };

class snake
{
public:
	snake(int _w, int _h)
		: w(_w), h(_h), head(_w / 2, _h / 2)
	{
		// 初始的移动方向，x轴正方向
		dirx = 1;
		diry = 0;

		addbody(head, 10);
	}
	void draw()
	{
		// 蛇头
		head.draw();
		// 蛇身
		for (node& n : body)
		{
			n.draw();
		}
		// 清理贪吃蛇移动过程中产生的尾巴
		clrxy(tailx, taily);
	}
	bool move() // 蛇的移动
	{
		int lastx = head.getx();
		int lasty = head.gety();
		// 移动控制点，蛇头
		head.move(dirx, diry);

		int newx = head.getx();
		int newy = head.gety();

		// 判断移动是否超出场景范围
		if (newx <= 1 || newx >= w
			|| newy <= 1 || newy >= h)

		{
			over();

			return false;
		}

		// 移动身体
		for (node& n : body)
		{
			tailx = n.getx();
			taily = n.gety();

			// 移动到前一个节点的位置
			n.setx(lastx);
			n.sety(lasty);

			lastx = tailx;
			lasty = taily;
		}

		// 判断是否碰到自身
		auto it = find(body.begin(), body.end(), head);

		if (body.end() != it)
		{
			over();
			return false;
		}
		else
		{
			return true;
		}
	}

	void turn(DIRECTION d)
	{
		if (0 == dirx)
		{
			if (LEFT == d)
			{
				dirx = -1;
				diry = 0;
			}
			else if (RIGHT == d)
			{
				dirx = 1;
				diry = 0;
			}
		}
		else if (0 == diry)
		{
			if (UP == d)
			{
				dirx = 0;
				diry = -1;
			}
			else if (DOWN == d)
			{
				dirx = 0;
				diry = 1;
			}
		}
	}
	node& gethead()
	{
		return head;
	}
	// 在尾巴增加一个节点
	void addtail()
	{
		auto rit = body.rbegin();
		//vector<node>::reverse_iterator rit = body.rbegin();
		if (rit != body.rend())
			addbody(*rit);
	}
private:
	void over()
	{
		m.lock();
		isover = true;
		m.unlock();
	}
	// 身体增加
	void addbody(node& h, int n = 1)
	{
		// 根据head的位置，构建body节点
		int x = h.getx();
		int y = h.gety();
		for (int i = 1; i < n + 1; ++i)
		{
			body.push_back(node(x - i*dirx, y - i*diry));
		}
	}
private:
	int w, h;  // 场景范围ie
	node2 head;
	vector<node> body;
	int dirx, diry; // 方向
	int tailx, taily;// 尾巴位置
};



// 表示食物的类
class food
{
public:
	food()
	{
		// 初始状态，随机的15个小食物
		addfood(1);
	}
	void draw()
	{
		for (shared_ptr<node> p : v)
		{
			p->draw();
		}
	}
	// 判断是否吃到食物
	bool eat(node& n)
	{
		shared_ptr<node> sp(new node(n));
		auto it = find_if(v.begin(), v.end(), [&](shared_ptr<node> a)->bool{
			return (n == *a);
		});
		if (it != v.end()){
			v.erase(it);
			return true;
		}
		else{
			return false;
		}
	}
	// 添加食物
	void addfood(int n = 1)
	{
		default_random_engine eng;
		random_device  rnd_device;
		eng.seed(rnd_device());
		uniform_int_distribution<int> wpos(2, W - 1);
		uniform_int_distribution<int> hpos(2, H - 1);


		for (int i = 0; i < n; ++i)
		{
			node *p = nullptr;
			// 场景内的随机位置
			int x = wpos(eng);
			int y = hpos(eng);
			p = new node1(x, y);
			v.push_back(shared_ptr<node>(p));
		}
	}
private:
	vector<shared_ptr<node>> v;
};
// MAIN


snake s(W, H);
mutex ms;
food f;


void render()
{
	while (true)
	{
		ms.lock();
		s.move();
		node& head = s.gethead();
		ms.unlock();

		// 判断是否吃到了食物
		if (f.eat(head))
		{
			ms.lock();
			// 如果吃到，增加长度
			s.addtail();
			f.addfood();
			ms.unlock();
		}

		m.lock();
		if (isover)
		{
			m.unlock();
			break;
		}
		m.unlock();

		ms.lock();
		s.draw();
		f.draw();
		ms.unlock();

		this_thread::sleep_for(seconds(1));
	}
}

void control()
{
	// 截获键盘消息并进行处理
	HANDLE hIn;
	HANDLE hOut;
	int KeyEvents = 0;
	bool Continue = TRUE;
	INPUT_RECORD InRec;
	DWORD NumRead;

	// 截获键盘消息，并对特定事件进行处理
	hIn = GetStdHandle(STD_INPUT_HANDLE);
	hOut = GetStdHandle(STD_OUTPUT_HANDLE);

	DWORD  fdwMode, fdwSaveOldMode;
	GetConsoleMode(hIn, &fdwSaveOldMode);

	fdwMode = fdwSaveOldMode&(~ENABLE_ECHO_INPUT)
		&(~ENABLE_LINE_INPUT);
	SetConsoleMode(hIn, fdwMode);

	while (Continue)
	{
		ReadConsoleInput(hIn,
			&InRec,
			1,
			&NumRead);

		switch (InRec.EventType)
		{
		case KEY_EVENT:
			if (InRec.Event.KeyEvent.bKeyDown)
			{

				DIRECTION d = LEFT;
				if (InRec.Event.KeyEvent.wVirtualKeyCode == VK_UP)
				{
					d = UP;
				}
				else if (InRec.Event.KeyEvent.wVirtualKeyCode == VK_DOWN)
				{
					d = DOWN;
				}
				else if (InRec.Event.KeyEvent.wVirtualKeyCode == VK_LEFT)
				{
					d = LEFT;
				}
				else if (InRec.Event.KeyEvent.wVirtualKeyCode == VK_RIGHT)
				{
					d = RIGHT;
				}

				ms.lock();
				s.turn(d);
				ms.unlock();
			}
			break;
		}
	}
}

int main()
{
	srand(time(NULL));
	// 清理屏幕
	clrscr();
	// 绘制围墙 
	border b(1, 1, W, H);
	b.draw();

	Settitle(64, 11);
	thread ren(render);
	thread con(control);
	ren.join();
	con.join();


	return 0;
}