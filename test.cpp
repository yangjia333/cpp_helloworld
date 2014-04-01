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

// ������Χ
const int W = 61;
const int H = 21;

// FUNCTIONS

// �����Ļ
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
// ��λ���λ��
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

// ����ĳ���ض�λ�ã�������������̰���ߵ�β��
void clrxy(int x, int y)
{
	gotoxy(x, y);
	cout << ' ';
}

void Settitle(int x, int y){
	gotoxy(x, y);
	cout << "̰�Գ�";
	//gotoxy(x, y + 1);
	//cout << "by YJ";
}

// �����ϵĽڵ�
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
	// ƫ��һ��λ�ã��������ƶ���
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

// Χǽ���ɽڵ㹹��
class border
{
public:
	border(int _ux, int _uy,
		int _dx, int _dy)
		:ux(_ux), uy(_uy), dx(_dx), dy(_dy)
	{

		// ����Χǽ
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
		// ��ʼ���ƶ�����x��������
		dirx = 1;
		diry = 0;

		addbody(head, 10);
	}
	void draw()
	{
		// ��ͷ
		head.draw();
		// ����
		for (node& n : body)
		{
			n.draw();
		}
		// ����̰�����ƶ������в�����β��
		clrxy(tailx, taily);
	}
	bool move() // �ߵ��ƶ�
	{
		int lastx = head.getx();
		int lasty = head.gety();
		// �ƶ����Ƶ㣬��ͷ
		head.move(dirx, diry);

		int newx = head.getx();
		int newy = head.gety();

		// �ж��ƶ��Ƿ񳬳�������Χ
		if (newx <= 1 || newx >= w
			|| newy <= 1 || newy >= h)

		{
			over();

			return false;
		}

		// �ƶ�����
		for (node& n : body)
		{
			tailx = n.getx();
			taily = n.gety();

			// �ƶ���ǰһ���ڵ��λ��
			n.setx(lastx);
			n.sety(lasty);

			lastx = tailx;
			lasty = taily;
		}

		// �ж��Ƿ���������
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
	// ��β������һ���ڵ�
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
	// ��������
	void addbody(node& h, int n = 1)
	{
		// ����head��λ�ã�����body�ڵ�
		int x = h.getx();
		int y = h.gety();
		for (int i = 1; i < n + 1; ++i)
		{
			body.push_back(node(x - i*dirx, y - i*diry));
		}
	}
private:
	int w, h;  // ������Χie
	node2 head;
	vector<node> body;
	int dirx, diry; // ����
	int tailx, taily;// β��λ��
};



// ��ʾʳ�����
class food
{
public:
	food()
	{
		// ��ʼ״̬�������15��Сʳ��
		addfood(1);
	}
	void draw()
	{
		for (shared_ptr<node> p : v)
		{
			p->draw();
		}
	}
	// �ж��Ƿ�Ե�ʳ��
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
	// ���ʳ��
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
			// �����ڵ����λ��
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

		// �ж��Ƿ�Ե���ʳ��
		if (f.eat(head))
		{
			ms.lock();
			// ����Ե������ӳ���
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
	// �ػ������Ϣ�����д���
	HANDLE hIn;
	HANDLE hOut;
	int KeyEvents = 0;
	bool Continue = TRUE;
	INPUT_RECORD InRec;
	DWORD NumRead;

	// �ػ������Ϣ�������ض��¼����д���
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
	// ������Ļ
	clrscr();
	// ����Χǽ 
	border b(1, 1, W, H);
	b.draw();

	Settitle(64, 11);
	thread ren(render);
	thread con(control);
	ren.join();
	con.join();


	return 0;
}