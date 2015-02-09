// 项目名：ARDroneTL 
// 作  者：偷懒飞行员团队
// 时  间：2015.02.09 23：45

#include <stdio.h>  
#include <winsock2.h>  
#include <assert.h>
#include <iostream>
// C++11头文件
#include <mutex>
#include <thread>
// 附加库
#include <gtk/gtk.h>

//本地头文件
#include "MemoryLibrary.h"
#pragma comment(lib, "ws2_32.lib") 

using namespace std;

/*************************************************
* 常量声明定义模块
**************************************************/
// three Port
const int  NAVDATA_PORT = 5554;
const int  VIDEO_PORT   = 5555;
const int  AT_PORT      = 5556;

const int INTERVAL      = 100;
const char* ARDrone_IP	= "192.168.1.1";
const int C_OK			= 1;							
const int C_ERRO		= 0;


/*************************************************
* 类型声明模块
**************************************************/

// Navdata Struct
struct NAV_DATA
{
	int32_t header;
	int32_t state;
	int32_t sequence;
	int32_t visionDefined;
	int16_t tag;
	int16_t size;
	int32_t ctrlState;
	int32_t batteryLevel;
	int32_t pitch;
	int32_t roll;
	int32_t yaw;
	int32_t altitude;
	int32_t vx;
	int32_t vy;
	int32_t vz;
};

// UI 控件联合体
struct ARDrone_UI
{
	GtkTextBuffer*	buffer;		// 输出文本缓冲区
	GtkWidget*		window;		// 窗体
	GtkWidget*		view;		// 
	GtkWidget*		box;		// 容器
	GtkWidget*		button;		// 按钮
};
ARDrone_UI* arui = new ARDrone_UI();	// 全局的ui 控件

// 联合体：Buffer
union INT_FLOAT_BUFFER
{
	float	fBuff;
	int		iBuff;
};

/*
* ARDrone 自定义类的详细介绍
*
*【控制操作】
* 已完成：起飞、降落、前进、后退、向左飞、向右飞、调整速度
* 待完成：原地左旋转，原地右旋转
*【成员变量操作】
* 已完成：获取当前序号、获取前序号、获取后序号、设置前序号
* 待完成：无
*【发送指令】
* 发送基础指令、发送飞行控制指令
*【初始化操作】
* 发送飞行器初始化配置指令、socket初始化操作
*【辅助分析】
* 已完成：数据包分析、float类型转换int类型
* 待完成：无
*【公有成员变量】
* 导航数据、前一个指令
*【私有成员变量】
* 发送指令的套接字（一套）、飞行速度、飞行器名字、包序号、前包序号、互斥锁

* 原则：有所为，有所不为
*/
class ARDrone
{
public:
	ARDrone(void){}
	ARDrone(char*);
	~ARDrone(void);

public:
	// 飞行器操作
	void takeoff();
	void land();

	void goingUp();
	void goingDown();

	void turnLeft();
	void turnRight();

	void setSpeed(int);		// 设置飞行速度

public:
	int		getCurrentSeq(){return this->seq_;}			// get the current sequence
	int		getLastSeq(){return this->lastSeq_;}		// get the last sequence
	int		getNextSeq();								// get the next sequence
	void	setLastSeq(int);							// set the last sequence

	int		send_at_cmd(const char*);					// send all command
	int		send_pcmd(int, float, float, float, float);	// send control command
	void	parse(MemoryLibrary::Buffer&);				// 数据包分析

protected:
	void	initializeCmd();				// initialize command
	void	initializeSocketaddr();			// initialize sockaddr_in
	int		floatToInt(float);				// 使用联合体实现float 转化int	
	
public:
	NAV_DATA		navData;			// ardrone's navdata 
	const char*		at_cmd_last;		// save the last command

private:
	SOCKET		socketat_;		// socket
	sockaddr_in Atsin_;			// struct
	int			lenSin_;		// the length of sin
	float		speed_;			// fly speed
	char*		name_;			// ardrone's name
	int			seq_;			// sequence of data packet 
	int			lastSeq_;		// save the last seq
	std::mutex	mtx;			// mutex for critical section
};

/*************************************************
* ARDrone 类成员函数的实现模块（model）
**************************************************/
ARDrone::ARDrone(char* name)
{
	this->name_ = name;

	// 用WSAStartup 启动Ws2_32.lib
	WORD socketVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	assert(WSAStartup(socketVersion, &wsaData) == 0);

	// 成员变量初始化
	this->socketat_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	initializeSocketaddr();

	lenSin_		= sizeof(Atsin_);
	speed_		= 0.1f;
	seq_		= 1;
	lastSeq_	= 1;
	at_cmd_last = "";
	cout << "ARDrone initialized" <<endl;
}

// Destructors
ARDrone::~ARDrone()
{
	delete[] at_cmd_last;
	delete[] name_;
	
	WSACleanup();					// 释放Winsock库
	closesocket(this->socketat_);	// 关闭SOCKET
}

// initialize sockaddr_in
void ARDrone::initializeSocketaddr()
{
	Atsin_.sin_family		= AF_INET;
	Atsin_.sin_port			= htons(AT_PORT);
	Atsin_.sin_addr.s_addr	= inet_addr(ARDrone_IP);
	std::cout << "IP:" << ARDrone_IP << "Port:" << AT_PORT << std::endl;
}

// initialize command（没有使用）
void ARDrone::initializeCmd()
{
	char cmd[1024];
	// 设置最大高度
	sprintf_s(cmd, "AT*CONFIG=%d,\"control:altitude_max\",\"2000\"\r", getNextSeq());
	assert(send_at_cmd(cmd));
	Sleep(INTERVAL);

	// ????
	sprintf_s(cmd, "AT*CONFIG=%d,\"control:control_level\",\"0\"\\r", getNextSeq());
	assert(send_at_cmd(cmd));
	Sleep(INTERVAL);

	// 设置超声波频率
	sprintf_s(cmd, "AT*CONFIG=%d,\"pic:ultrasound_freq\",\"8\"\r", getNextSeq());
	assert(send_at_cmd(cmd));
	Sleep(INTERVAL);
	//flat trim
	sprintf_s(cmd, "AT*FTRIM=%d\r", getNextSeq());
	assert(send_at_cmd(cmd));
	Sleep(INTERVAL);
}

// 飞行控制函数实现部分
void ARDrone::takeoff()
{
	assert(send_at_cmd("AT*REF=1,290718208"));
	printf("takeoff\n");
}

void ARDrone::land()
{
	assert(send_at_cmd("AT*REF=1,290717696"));
	printf("land\n");
}

void ARDrone::goingUp()
{
	assert(send_pcmd(1, 0, 0, speed_, 0));
	printf("goingUp\n");
}

void ARDrone::goingDown()
{
	assert(send_pcmd(1, 0, 0, -speed_, 0));
	printf("goingDown\n");
}

void ARDrone::turnLeft()
{
	assert(send_pcmd(1, 0, 0, 0, -speed_));
	printf("turnLeft\n");
}

void ARDrone::turnRight()
{
	assert(send_pcmd(1, 0, 0, 0, speed_));
	printf("turnRight\n");
}

void ARDrone::setSpeed(int mul)
{
	this->speed_ = mul * 0.1f;
}

// get the lastest sequence
int ARDrone::getNextSeq()
{
	// 互斥锁:用于多线程
	this->mtx.lock();
	seq_ += 1;
	this->mtx.unlock();
	return seq_;
}

//set the last sequence
void ARDrone::setLastSeq(int currentSeq)
{
	this->lastSeq_ = currentSeq;
}

// send control command
int ARDrone::send_pcmd(int enable, float roll, float pitch, float gaz, float yaw)
{
	char cmd[1024];
	sprintf_s(cmd, "AT*PCMD=%d,%d,%d,%d,%d,%d", getNextSeq(), enable,
			floatToInt(roll), floatToInt(pitch), floatToInt(gaz), floatToInt(yaw));
	int result = send_at_cmd(cmd);
	return result;
}

// send all command
int ARDrone::send_at_cmd(const char* cmd)
{
	// 互斥锁:用于多线程C++11
	this->mtx.lock();
	at_cmd_last = cmd;
	int result = sendto(this->socketat_, cmd, strlen(cmd), 0, (sockaddr *)&Atsin_, this->lenSin_);
	if (result == SOCKET_ERROR)
		return C_ERRO;

	printf_s("AT command: %s\n", cmd);
	this->mtx.unlock();
	return C_OK;
}

// get the same memory of float
int ARDrone::floatToInt(float f)
{
	INT_FLOAT_BUFFER buff;
	buff.fBuff = f;
	return buff.iBuff;
}

// parse data packet
void ARDrone::parse(MemoryLibrary::Buffer& buffer)
{
	int offset = 0;
	int header = buffer.MakeValueFromOffset<int32_t>(offset);
	if (header != 0x55667788)
	{
		cout << "NavigationDataReceiver FAIL, because the header != 0x55667788\n";
		return;
	}
	// /////////////////////////////////////////////////////////////////(Test)show in console
	offset += 4;
	int state = buffer.MakeValueFromOffset<int32_t>(offset);
	cout << "state: "<< state<< endl;
	offset += 4;
	int sequence = buffer.MakeValueFromOffset<int32_t>(offset);
	cout << "sequence: "<< sequence<< endl;

	offset += 4;
	int visionDefined = buffer.MakeValueFromOffset<int32_t>(offset);
	cout << "visionDefined:"<< visionDefined<< endl;
	offset += 4;
	int16_t tag = buffer.MakeValueFromOffset<int16_t>(offset);
	cout << "tag: " << tag << endl;
	offset += 2;
	int16_t size = buffer.MakeValueFromOffset<int16_t>(offset);
	cout << "size: " << size << endl;
	offset += 2;
	int ctrlState = buffer.MakeValueFromOffset<int32_t>(offset);
	cout << "ctrlState: " << ctrlState << endl;

	offset += 4;
	cout << "batteryLevel: " << buffer.MakeValueFromOffset<int>(offset) << "%" << endl;
	offset += 4;
	cout << "pitch: " << buffer.MakeValueFromOffset<float>(offset) / 1000.0f << endl;
	offset += 4;
	cout << "roll: " << buffer.MakeValueFromOffset<float>(offset) / 1000.0f << endl;
	offset += 4;
	cout << "yaw" << buffer.MakeValueFromOffset<float>(offset) / 1000.0f << endl;
	offset += 4;
	cout << "altitude: " << (float)buffer.MakeValueFromOffset<int>(offset) / 1000.0f << endl;

	offset += 4;
	cout << "vx: " << buffer.MakeValueFromOffset<float>(offset) << endl;
	offset += 4;
	cout << "vy: " << buffer.MakeValueFromOffset<float>(offset) << endl;
	offset += 4;
	cout << "vz: " << buffer.MakeValueFromOffset<float>(offset) << endl;
	offset += 4;
	
	////////////////////////////////////////////////////////////////// set  
	offset = 0;
	this->navData.header = buffer.MakeValueFromOffset<int32_t>(offset);
	offset += 4;
	this->navData.state = buffer.MakeValueFromOffset<int32_t>(offset);
	offset += 4;
	this->navData.sequence = buffer.MakeValueFromOffset<int32_t>(offset);
	offset += 4;
	this->navData.visionDefined = buffer.MakeValueFromOffset<int32_t>(offset);
	offset += 4;
	
	this->navData.tag = buffer.MakeValueFromOffset<int16_t>(offset);
	offset += 2;
	this->navData.size = buffer.MakeValueFromOffset<int16_t>(offset);
	offset += 2;
	this->navData.ctrlState = buffer.MakeValueFromOffset<int32_t>(offset);
	offset += 4;
	
	this->navData.batteryLevel = buffer.MakeValueFromOffset<int32_t>(offset);
	offset += 4;
	this->navData.altitude = buffer.MakeValueFromOffset<int32_t>(offset);
	offset += 4;

	this->navData.pitch = buffer.MakeValueFromOffset<int32_t>(offset);
	offset += 4;
	this->navData.roll = buffer.MakeValueFromOffset<int32_t>(offset);
	offset += 4;
	this->navData.yaw = buffer.MakeValueFromOffset<int32_t>(offset);
	offset += 4;

	this->navData.vx = buffer.MakeValueFromOffset<int32_t>(offset);
	offset += 4;
	this->navData.vy = buffer.MakeValueFromOffset<int32_t>(offset);
	offset += 4;
	this->navData.vz = buffer.MakeValueFromOffset<int32_t>(offset);
	offset += 4;
}

/********************************
* 消息响应控制模块（control）
*********************************/
// 键盘控制消息响应函数
void control(GtkWidget* widget, GdkEventKey* event, gpointer data)
{
	ARDrone* ardrone = (ARDrone*)data;
	switch (event->keyval)
	{
		case GDK_KEY_KP_Enter:
			ardrone->takeoff(); break;
		case GDK_KEY_BackSpace:
			ardrone->land(); break;
		case GDK_KEY_W: 
			ardrone->goingUp(); break;
		case GDK_KEY_S: 
			ardrone->goingDown(); break;
		case GDK_KEY_A: 
			ardrone->turnLeft(); break;
		case GDK_KEY_D: 
			ardrone->turnRight(); break;
		default: 
			printf("指令不支持\n");break;
	}
}

/******************************************
* UI 模块 (VIEW)
******************************************/

// 窗口销毁
void destroy (GtkWidget *widget, gpointer data)
{
    gtk_main_quit ();
}

// UI 初始化
void initGtk(int argc, char* argv[])
{
	gtk_init(&argc, &argv);

	// 窗体属性设置
	arui->window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(arui->window), "myARDrone");
    gtk_window_set_default_size(GTK_WINDOW(arui->window), 400, 300);
    gtk_window_set_position(GTK_WINDOW(arui->window), GTK_WIN_POS_CENTER);
    gtk_container_set_border_width(GTK_CONTAINER(arui->window), 40);
	
	// 控件设置
	arui->box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);		// 容器控件在水平方位
	gtk_container_add(GTK_CONTAINER(arui->window), arui->box);
	arui->buffer = gtk_text_buffer_new(NULL);
	arui->view = gtk_text_view_new_with_buffer(arui->buffer);
	gtk_box_pack_start(GTK_BOX(arui->box), arui->view, TRUE, TRUE, 10);
	gtk_widget_set_size_request(arui->view, 10, 15);

	// 消息响应
	g_signal_connect(G_OBJECT (arui->window), "destroy", G_CALLBACK(destroy), NULL);

	// button = gtk_button_new_with_label("getNavData");
	// g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(button_event), NULL);
	// gtk_box_pack_start(GTK_BOX(box), button, TRUE, TRUE, 10);

	gtk_widget_show_all(arui->window);
}

/******************************************
* 工作线程模块
******************************************/

// 保持与Ardrone连接的线程
void weakUpThread(ARDrone* ardrone)
{
	char cmd[1024]	= { 0 };
	int delay		= 0;
	while( true)
	{
		// keep weak up
		Sleep(40);
		if (ardrone->getCurrentSeq() == ardrone->getLastSeq())
			ardrone->send_at_cmd(ardrone->at_cmd_last);

		ardrone->setLastSeq(ardrone->getCurrentSeq());
		delay++;
		if (delay >= 4)
		{
			delay = 0;
			sprintf_s(cmd, "AT*COMWDG=%d\r", ardrone->getNextSeq());
			assert(ardrone->send_at_cmd(cmd));
		}
	}
}

// 获取导航数据的线程
void NavDataThread(ARDrone* ardrone)
{
	SOCKET socketNav_		= socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	sockaddr_in	navSin_;
	navSin_.sin_family		= AF_INET;
	navSin_.sin_port		= htons(NAVDATA_PORT);
	navSin_.sin_addr.s_addr = inet_addr(ARDrone_IP);
	int lenNavSin_			= sizeof(navSin_);

	// 激活指令发往NAVDATA_PORT 端口
	const char trigger[4] = { 0x01, 0x00, 0x00, 0x00 };
	int result = sendto(socketNav_, trigger, strlen(trigger), 0, (sockaddr *)&navSin_, lenNavSin_);
	if (result != SOCKET_ERROR)
		printf_s("Sent trigger flag to UDP port : %d \n", NAVDATA_PORT);

	// 配置指令发往AT_PORT 端口
	char initCmd[1024] = { 0 };
	sprintf_s(initCmd, "AT*CONFIG=%d,\"general:navdata_demo\",\"TRUE\"\r", ardrone->getNextSeq());
	assert(ardrone->send_at_cmd(initCmd));
	
	// 接收数据包
	MemoryLibrary::Buffer navDataBuffer;	// 二进制数据缓冲区
	char recv[1024] = {0};					// 数据包接收数组
	int lenRecv		= 0;
	int delay		= 0;
	// UI 部分
	char text[100]	= {0};					// 导航数据显示字符串
	GtkTextIter start,end;					// 缓冲区开始和结束位置

	while (true)
	{
		lenRecv = recvfrom(socketNav_, recv, 1024, 0, (struct sockaddr*)&navSin_, &lenNavSin_);
		delay++;
		if (delay >= 5)
		{
			delay = 0;
			printf("received %d bytes\n", lenRecv);
			navDataBuffer.Set(recv, lenRecv);
			ardrone->parse(navDataBuffer);	

			sprintf_s(text, "batreeyLevel: %d%%  altitude: %d\n"
					, ardrone->navData.batteryLevel, ardrone->navData.altitude); 
			gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(arui->buffer), &start, &end);//获得缓冲区开始和结束位置的Iter	
			gtk_text_buffer_set_text(GTK_TEXT_BUFFER(arui->buffer), text, 30);		//插入文本到缓冲区
		}
	}
}

/******************************************
* 主函数模块
******************************************/
int main(int argc, char* argv[])
{
	ARDrone* ardrone = new ARDrone("myardrone");
	initGtk(argc, argv);

	// C++11 声明导航数据的线程
	std::thread navThread(NavDataThread, ardrone);
	std::thread weakThread(weakUpThread, ardrone);

	// UI循环
	gtk_main();

	// 空循环：UI关闭后保持其他线程运行
	while(true)
	{
		;
	}
	return 0;
}

