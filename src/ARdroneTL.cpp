<<<<<<< HEAD
// 项目名：ARDroneTL 
// 作  者：偷懒飞行员团队
// 时  间：2015.03.13
=======
// 椤圭洰鍚嶏細ARDroneTL 
// 浣�  鑰咃細鍋锋噿椋炶鍛樺洟闃�
// 鏃�  闂达細2015.02.09 23锛�45
>>>>>>> origin/master

#include <stdio.h>  
#include <winsock2.h>  
#include <assert.h>
#include <iostream>
<<<<<<< HEAD
#include <fstream>
// C++11头文件
=======
// C++11澶存枃浠�
>>>>>>> origin/master
#include <mutex>
#include <thread>
// 闄勫姞搴�
#include <gtk/gtk.h>
<<<<<<< HEAD
#include <gdk/gdkkeysyms.h>
//本地头文件
=======

//鏈湴澶存枃浠�
>>>>>>> origin/master
#include "MemoryLibrary.h"
#pragma comment(lib, "ws2_32.lib") 

using namespace std;

/*************************************************
* 甯搁噺澹版槑瀹氫箟妯″潡
**************************************************/
// three Port
const int  NAVDATA_PORT = 5554;
const int  VIDEO_PORT   = 5555;
const int  AT_PORT      = 5556;

const int INTERVAL = 100;
const char* ARDrone_IP = "192.168.1.1";
const int C_OK = 1;
const int C_ERRO = 0;


fstream outfile;

/*************************************************
* 绫诲瀷澹版槑妯″潡
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

// UI 鎺т欢鑱斿悎浣�
struct ARDrone_UI
{
	GtkTextBuffer*	buffer;		// 杈撳嚭鏂囨湰缂撳啿鍖�
	GtkWidget*		window;		// 绐椾綋
	GtkWidget*		view;		// 
	GtkWidget*		box;		// 瀹瑰櫒
	GtkWidget*		button;		// 鎸夐挳
};
ARDrone_UI* arui = new ARDrone_UI();	// 鍏ㄥ眬鐨剈i 鎺т欢

// 鑱斿悎浣擄細Buffer
union INT_FLOAT_BUFFER
{
	float	fBuff;
	int		iBuff;
};

/*
* ARDrone 鑷畾涔夌被鐨勮缁嗕粙缁�
*
<<<<<<< HEAD
*【控制操作】
* 已完成：起飞、降落、前进、后退、向左飞、向右飞、调整速度
*【成员变量操作】
* 已完成：获取当前序号、获取前序号、获取后序号、设置前序号
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
*
* 原则：有所为，有所不为
=======
*銆愭帶鍒舵搷浣溿��
* 宸插畬鎴愶細璧烽銆侀檷钀姐�佸墠杩涖�佸悗閫�銆佸悜宸﹂銆佸悜鍙抽銆佽皟鏁撮�熷害
* 寰呭畬鎴愶細鍘熷湴宸︽棆杞紝鍘熷湴鍙虫棆杞�
*銆愭垚鍛樺彉閲忔搷浣溿��
* 宸插畬鎴愶細鑾峰彇褰撳墠搴忓彿銆佽幏鍙栧墠搴忓彿銆佽幏鍙栧悗搴忓彿銆佽缃墠搴忓彿
* 寰呭畬鎴愶細鏃�
*銆愬彂閫佹寚浠ゃ��
* 鍙戦�佸熀纭�鎸囦护銆佸彂閫侀琛屾帶鍒舵寚浠�
*銆愬垵濮嬪寲鎿嶄綔銆�
* 鍙戦�侀琛屽櫒鍒濆鍖栭厤缃寚浠ゃ�乻ocket鍒濆鍖栨搷浣�
*銆愯緟鍔╁垎鏋愩��
* 宸插畬鎴愶細鏁版嵁鍖呭垎鏋愩�乫loat绫诲瀷杞崲int绫诲瀷
* 寰呭畬鎴愶細鏃�
*銆愬叕鏈夋垚鍛樺彉閲忋��
* 瀵艰埅鏁版嵁銆佸墠涓�涓寚浠�
*銆愮鏈夋垚鍛樺彉閲忋��
* 鍙戦�佹寚浠ょ殑濂楁帴瀛楋紙涓�濂楋級銆侀琛岄�熷害銆侀琛屽櫒鍚嶅瓧銆佸寘搴忓彿銆佸墠鍖呭簭鍙枫�佷簰鏂ラ攣

* 鍘熷垯锛氭湁鎵�涓猴紝鏈夋墍涓嶄负
>>>>>>> origin/master
*/
class ARDrone
{
public:
	ARDrone(void){}
	ARDrone(char*);
	~ARDrone(void);

public:
	// 椋炶鍣ㄦ搷浣�
	void takeoff();
	void land();
	void hover();

	void goingUp();
	void goingDown();
	void goingForward();
	void goingBack();
	void goingLeft();
	void goingRight();

	void turnLeft();
	void turnRight();

	void setSpeed(int);		// 璁剧疆椋炶閫熷害

public:
	int		getCurrentSeq(){ return this->seq_; }			// get the current sequence
	int		getLastSeq(){ return this->lastSeq_; }		// get the last sequence
	int		getNextSeq();								// get the next sequence
	void	setLastSeq(int);							// set the last sequence

	int		send_at_cmd(char*);					// send all command
	int		send_pcmd(int, float, float, float, float);	// send control command
	void	parse(MemoryLibrary::Buffer&);				// 鏁版嵁鍖呭垎鏋�

protected:
	void	initializeCmd();				// initialize command
	void	initializeSocketaddr();			// initialize sockaddr_in
<<<<<<< HEAD
	int		floatToInt(float);				// 使用联合体实现float 转化int	

=======
	int		floatToInt(float);				// 浣跨敤鑱斿悎浣撳疄鐜癴loat 杞寲int	
	
>>>>>>> origin/master
public:
	NAV_DATA		navData;			// ardrone's navdata 
	char*			at_cmd_last;		// save the last command

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
* ARDrone 绫绘垚鍛樺嚱鏁扮殑瀹炵幇妯″潡锛坢odel锛�
**************************************************/
ARDrone::ARDrone(char* name)
{
	this->name_ = name;
<<<<<<< HEAD
	// 用WSAStartup 启动Ws2_32.lib
=======

	// 鐢╓SAStartup 鍚姩Ws2_32.lib
>>>>>>> origin/master
	WORD socketVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	assert(WSAStartup(socketVersion, &wsaData) == 0);

	// 鎴愬憳鍙橀噺鍒濆鍖�
	this->socketat_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	initializeSocketaddr();

	lenSin_		= sizeof(Atsin_);
	speed_		= 0.1f;
	seq_		= 1;
	lastSeq_	= 1;
	at_cmd_last = new char[1024];

	cout << "ARDrone initialized" << endl;
	outfile << "ARDrone initialized" << endl;
}

// Destructors
ARDrone::~ARDrone()
{
	delete[] at_cmd_last;
	delete[] name_;
<<<<<<< HEAD

	WSACleanup();					// 释放Winsock库
	closesocket(this->socketat_);	// 关闭SOCKET
=======
	
	WSACleanup();					// 閲婃斁Winsock搴�
	closesocket(this->socketat_);	// 鍏抽棴SOCKET
>>>>>>> origin/master
}

// initialize sockaddr_in
void ARDrone::initializeSocketaddr()
{
	Atsin_.sin_family = AF_INET;
	Atsin_.sin_port = htons(AT_PORT);
	Atsin_.sin_addr.s_addr = inet_addr(ARDrone_IP);
	cout << "IP:" << ARDrone_IP << "Port:" << AT_PORT << std::endl;
	outfile << "IP:" << ARDrone_IP << "Port:" << AT_PORT << std::endl;
}

// initialize command锛堟病鏈変娇鐢級
void ARDrone::initializeCmd()
{
	char cmd[1024];
	// 璁剧疆鏈�澶ч珮搴�
	sprintf_s(cmd, "AT*CONFIG=%d,\"control:altitude_max\",\"2000\"\r", getNextSeq());
	assert(send_at_cmd(cmd));
	Sleep(INTERVAL);

	// ????
	sprintf_s(cmd, "AT*CONFIG=%d,\"control:control_level\",\"0\"\\r", getNextSeq());
	assert(send_at_cmd(cmd));
	Sleep(INTERVAL);

	// 璁剧疆瓒呭０娉㈤鐜�
	sprintf_s(cmd, "AT*CONFIG=%d,\"pic:ultrasound_freq\",\"8\"\r", getNextSeq());
	assert(send_at_cmd(cmd));
	Sleep(INTERVAL);
	//flat trim
	sprintf_s(cmd, "AT*FTRIM=%d\r", getNextSeq());
	assert(send_at_cmd(cmd));
	Sleep(INTERVAL);
}

// 椋炶鎺у埗鍑芥暟瀹炵幇閮ㄥ垎
void ARDrone::takeoff()
{
	char cmd[1024];
	sprintf_s(cmd, "AT*REF=%d,290718208\r", getNextSeq());
	assert(send_at_cmd(cmd));
	printf("take off\n");
	outfile << "Take off" << endl;
}

void ARDrone::land()
{
	char cmd[100];
	sprintf_s(cmd, "AT*REF=%d,290717696\r", getNextSeq());
	assert(send_at_cmd(cmd));
	printf("land\n");
	outfile << "Land" << endl;
}

//有问题？？？？？？？？？？？？？？？？？？？？？？？？？
void ARDrone::hover()
{
	assert(send_pcmd(0, 0, 0, 0, 0));
	outfile << "Hover" << endl;
}

void ARDrone::goingUp()
{
	assert(send_pcmd(1, 0, 0, speed_, 0));
	printf("goingUp\n");
	outfile << "Going up" << endl;
}

void ARDrone::goingDown()
{
	assert(send_pcmd(1, 0, 0, -speed_, 0));
	printf("goingDown\n");
	outfile << "Going down" << endl;
}

void ARDrone::goingForward()
{
	assert(send_pcmd(1, 0, -speed_, 0, 0));
	outfile << "Going Forward" << endl;
}

void ARDrone::goingBack()
{
	assert(send_pcmd(1, 0, +speed_, 0, 0));
	outfile << "Going Back" << endl;
}
void ARDrone::goingLeft()
{
	assert(send_pcmd(1, -speed_, 0, 0, 0));
	outfile << "Going Left" << endl;
}

void ARDrone::goingRight()
{
	assert(send_pcmd(1, speed_, 0, 0, 0));
	outfile << "Going Right" << endl;
}

void ARDrone::turnLeft()
{
	assert(send_pcmd(1, 0, 0, 0, -speed_));
	printf("turn Left\n");
	outfile << "Turn left" << endl;
}

void ARDrone::turnRight()
{
	assert(send_pcmd(1, 0, 0, 0, speed_));
	printf("turnRight\n");
	outfile << "Turn right" << endl;
}

void ARDrone::setSpeed(int mul)
{
	this->speed_ = mul * 0.1f;
}

// get the lastest sequence
int ARDrone::getNextSeq()
{
	// 浜掓枼閿�:鐢ㄤ簬澶氱嚎绋�
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
	outfile<< "speed:" << this->speed_<<endl;
	char cmd[1024];
	sprintf_s(cmd, "AT*PCMD=%d,%d,%d,%d,%d,%d\r", getNextSeq(), enable,
		floatToInt(roll), floatToInt(pitch), floatToInt(gaz), floatToInt(yaw));
	int result = send_at_cmd(cmd);
	return result;
}

// send all command
int ARDrone::send_at_cmd(char* cmd)
{
	// 浜掓枼閿�:鐢ㄤ簬澶氱嚎绋婥++11
	this->mtx.lock();
	at_cmd_last = cmd;
	int result = sendto(this->socketat_, cmd, strlen(cmd), 0, (sockaddr *)&Atsin_, this->lenSin_);
	if (result == SOCKET_ERROR)
		return C_ERRO;

	printf_s("AT command: %s\n", cmd);

	
	outfile << "AT command: " << cmd << endl;

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
		outfile << "NavigationDataReceiver FAIL, because the header != 0x55667788" << endl;
		return;
	}
	// /////////////////////////////////////////////////////////////////(Test)show in console
	offset += 4;
	int state = buffer.MakeValueFromOffset<int32_t>(offset);
	cout << "state: " << state << endl;
	outfile << "state: " << state << endl;

	offset += 4;
	int sequence = buffer.MakeValueFromOffset<int32_t>(offset);
	cout << "sequence: " << sequence << endl;
	outfile << "sequence: " << sequence << endl;

	offset += 4;
	int visionDefined = buffer.MakeValueFromOffset<int32_t>(offset);
	cout << "visionDefined:" << visionDefined << endl;
	outfile << "visionDefined:" << visionDefined << endl;

	offset += 4;
	int16_t tag = buffer.MakeValueFromOffset<int16_t>(offset);
	cout << "tag: " << tag << endl;
	outfile << "tag: " << tag << endl;

	offset += 2;
	int16_t size = buffer.MakeValueFromOffset<int16_t>(offset);
	cout << "size: " << size << endl;
	outfile << "size: " << size << endl;

	offset += 2;
	int ctrlState = buffer.MakeValueFromOffset<int32_t>(offset);
	cout << "ctrlState: " << ctrlState << endl;
	outfile << "ctrlState: " << ctrlState << endl;

	offset += 4;
	cout << "batteryLevel: " << buffer.MakeValueFromOffset<int>(offset) << "%" << endl;
	outfile << "batteryLevel: " << buffer.MakeValueFromOffset<int>(offset) << "%" << endl;

	offset += 4;
	cout << "pitch: " << buffer.MakeValueFromOffset<float>(offset) / 1000.0f << endl;
	outfile << "pitch: " << buffer.MakeValueFromOffset<float>(offset) / 1000.0f << endl;

	offset += 4;
	cout << "roll: " << buffer.MakeValueFromOffset<float>(offset) / 1000.0f << endl;
	outfile << "roll: " << buffer.MakeValueFromOffset<float>(offset) / 1000.0f << endl;

	offset += 4;
	cout << "yaw: " << buffer.MakeValueFromOffset<float>(offset) / 1000.0f << endl;
	outfile << "yaw: " << buffer.MakeValueFromOffset<float>(offset) / 1000.0f << endl;

	offset += 4;
	cout << "altitude: " << (float)buffer.MakeValueFromOffset<int>(offset) / 1000.0f << endl;
	outfile << "altitude: " << (float)buffer.MakeValueFromOffset<int>(offset) / 1000.0f << endl;

	offset += 4;
	cout << "vx: " << buffer.MakeValueFromOffset<float>(offset) << endl;
	outfile << "vx: " << buffer.MakeValueFromOffset<float>(offset) << endl;

	offset += 4;
	cout << "vy: " << buffer.MakeValueFromOffset<float>(offset) << endl;
	outfile << "vy: " << buffer.MakeValueFromOffset<float>(offset) << endl;

	offset += 4;
	cout << "vz: " << buffer.MakeValueFromOffset<float>(offset) << endl;
	outfile << "vz: " << buffer.MakeValueFromOffset<float>(offset) << endl;

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
* 娑堟伅鍝嶅簲鎺у埗妯″潡锛坈ontrol锛�
*********************************/
<<<<<<< HEAD

// 窗口销毁
void destroy(GtkWidget *widget, gpointer data)
{
	gtk_main_quit();
}

// 键盘控制消息响应函数
=======
// 閿洏鎺у埗娑堟伅鍝嶅簲鍑芥暟
>>>>>>> origin/master
void control(GtkWidget* widget, GdkEventKey* event, gpointer data)
{
	ARDrone* ardrone = (ARDrone*)data;
	guint keyValue = event->keyval;
	switch (keyValue)
	{
<<<<<<< HEAD
	case 1://1
		ardrone->takeoff(); break;
	case 50://2
		ardrone->land(); break;
	case 51://3
		ardrone->goingForward(); break;
	case 52://4
		ardrone->hover(); break;
	case 53://5
		ardrone->turnLeft(); break;
	case 54://6
		ardrone->turnRight(); break;
	default:
		printf("指令不支持\n"); 
		outfile << "指令不支持" << endl; break;
	}
}

void keyReleaseEvent(GtkWidget* widget, GdkEventKey* event, gpointer data)
=======
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
			printf("鎸囦护涓嶆敮鎸乗n");break;
	}
}

/******************************************
* UI 妯″潡 (VIEW)
******************************************/

// 绐楀彛閿�姣�
void destroy (GtkWidget *widget, gpointer data)
>>>>>>> origin/master
{
	ARDrone* ardrone = (ARDrone*)data;
	guint keyValue = event->keyval;
	outfile << keyValue << endl;
	if (keyValue != 49 || keyValue != 50) 
		return;
	ardrone->hover();
}

<<<<<<< HEAD
/******************************************
* UI 模块 (VIEW)
******************************************/
// UI 初始化
void initGtk(int argc, char* argv[], ARDrone* ardrone)
{
	gtk_init(&argc, &argv);

	// 窗体属性设置
	arui->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(arui->window), "myARDrone");
	gtk_window_set_default_size(GTK_WINDOW(arui->window), 400, 300);
	gtk_window_set_position(GTK_WINDOW(arui->window), GTK_WIN_POS_CENTER);
	gtk_container_set_border_width(GTK_CONTAINER(arui->window), 40);
	/*
	// 控件设置
	arui->box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);		// 容器控件在水平方位
=======
// UI 鍒濆鍖�
void initGtk(int argc, char* argv[])
{
	gtk_init(&argc, &argv);

	// 绐椾綋灞炴�ц缃�
	arui->window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(arui->window), "myARDrone");
    gtk_window_set_default_size(GTK_WINDOW(arui->window), 400, 300);
    gtk_window_set_position(GTK_WINDOW(arui->window), GTK_WIN_POS_CENTER);
    gtk_container_set_border_width(GTK_CONTAINER(arui->window), 40);
	
	// 鎺т欢璁剧疆
	arui->box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);		// 瀹瑰櫒鎺т欢鍦ㄦ按骞虫柟浣�
>>>>>>> origin/master
	gtk_container_add(GTK_CONTAINER(arui->window), arui->box);
	arui->buffer = gtk_text_buffer_new(NULL);
	arui->view = gtk_text_view_new_with_buffer(arui->buffer);
	gtk_box_pack_start(GTK_BOX(arui->box), arui->view, TRUE, TRUE, 10);
	gtk_widget_set_size_request(arui->view, 10, 15);
<<<<<<< HEAD
	*/
	// 消息响应
	g_signal_connect(G_OBJECT(arui->window), "destroy", G_CALLBACK(destroy), NULL);
	g_signal_connect(G_OBJECT(arui->window), "key_press_event", G_CALLBACK(control), ardrone);
	//g_signal_connect(G_OBJECT(arui->window), "key_release_event", G_CALLBACK(keyReleaseEvent), ardrone);
=======

	// 娑堟伅鍝嶅簲
	g_signal_connect(G_OBJECT (arui->window), "destroy", G_CALLBACK(destroy), NULL);

	// button = gtk_button_new_with_label("getNavData");
	// g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(button_event), NULL);
	// gtk_box_pack_start(GTK_BOX(box), button, TRUE, TRUE, 10);
>>>>>>> origin/master

	gtk_widget_show_all(arui->window);
}

/******************************************
* 宸ヤ綔绾跨▼妯″潡
******************************************/

// 淇濇寔涓嶢rdrone杩炴帴鐨勭嚎绋�
void weakUpThread(ARDrone* ardrone)
{
	char cmd[1024] = { 0 };
	int delay = 0;
	while (true)
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

// 鑾峰彇瀵艰埅鏁版嵁鐨勭嚎绋�
void NavDataThread(ARDrone* ardrone)
{
	SOCKET socketNav_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	sockaddr_in	navSin_;
	navSin_.sin_family = AF_INET;
	navSin_.sin_port = htons(NAVDATA_PORT);
	navSin_.sin_addr.s_addr = inet_addr(ARDrone_IP);
	int lenNavSin_ = sizeof(navSin_);

	// 婵�娲绘寚浠ゅ彂寰�NAVDATA_PORT 绔彛
	const char trigger[4] = { 0x01, 0x00, 0x00, 0x00 };
	int result = sendto(socketNav_, trigger, strlen(trigger), 0, (sockaddr *)&navSin_, lenNavSin_);
	if (result != SOCKET_ERROR)
	{
		printf_s("Sent trigger flag to UDP port : %d \n", NAVDATA_PORT);
		outfile << "Sent trigger flag to UDP port : " << NAVDATA_PORT << endl;
	}

	// 閰嶇疆鎸囦护鍙戝線AT_PORT 绔彛
	char initCmd[1024] = { 0 };
	sprintf_s(initCmd, "AT*CONFIG=%d,\"general:navdata_demo\",\"TRUE\"\r", ardrone->getNextSeq());
	assert(ardrone->send_at_cmd(initCmd));
<<<<<<< HEAD

	// 接收数据包
	MemoryLibrary::Buffer navDataBuffer;	// 二进制数据缓冲区
	char recv[1024] = { 0 };				// 数据包接收数组
	int lenRecv = 0;
	int delay = 0;
	// UI 部分
	char text[100] = { 0 };					// 导航数据显示字符串
	
=======
	
	// 鎺ユ敹鏁版嵁鍖�
	MemoryLibrary::Buffer navDataBuffer;	// 浜岃繘鍒舵暟鎹紦鍐插尯
	char recv[1024] = {0};					// 鏁版嵁鍖呮帴鏀舵暟缁�
	int lenRecv		= 0;
	int delay		= 0;
	// UI 閮ㄥ垎
	char text[100]	= {0};					// 瀵艰埅鏁版嵁鏄剧ず瀛楃涓�
	GtkTextIter start,end;					// 缂撳啿鍖哄紑濮嬪拰缁撴潫浣嶇疆

>>>>>>> origin/master
	while (true)
	{
		lenRecv = recvfrom(socketNav_, recv, 1024, 0, (struct sockaddr*)&navSin_, &lenNavSin_);
		delay++;
		if (delay >= 5)
		{
			delay = 0;
			printf("received %d bytes\n", lenRecv);
			outfile << "received " << lenRecv << " bytes" << endl;
			navDataBuffer.Set(recv, lenRecv);
			ardrone->parse(navDataBuffer);

			sprintf_s(text, "batreeyLevel: %d%%  altitude: %d\n"
<<<<<<< HEAD
				, ardrone->navData.batteryLevel, ardrone->navData.altitude);
			//outfile <<text;
			// gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(arui->buffer), &start, &end);    //获得缓冲区开始和结束位置的Iter	
			// gtk_text_buffer_set_text(GTK_TEXT_BUFFER(arui->buffer), text, -1);		//插入文本到缓冲区

=======
					, ardrone->navData.batteryLevel, ardrone->navData.altitude); 
			gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(arui->buffer), &start, &end);//鑾峰緱缂撳啿鍖哄紑濮嬪拰缁撴潫浣嶇疆鐨処ter	
			gtk_text_buffer_set_text(GTK_TEXT_BUFFER(arui->buffer), text, 30);		//鎻掑叆鏂囨湰鍒扮紦鍐插尯
>>>>>>> origin/master
		}
	}
}

/******************************************
* 涓诲嚱鏁版ā鍧�
******************************************/
int main(int argc, char* argv[])
{
	outfile.open("data.txt");
	if(!outfile.is_open())
		return 0;
	ARDrone* ardrone = new ARDrone("myardrone");
	initGtk(argc, argv, ardrone);

	// C++11 澹版槑瀵艰埅鏁版嵁鐨勭嚎绋�
	std::thread navThread(NavDataThread, ardrone);
	std::thread weakThread(weakUpThread, ardrone);

	// UI寰幆
	gtk_main();

<<<<<<< HEAD
	// 空循环：UI关闭后保持其他线程运行
	while (true)
=======
	// 绌哄惊鐜細UI鍏抽棴鍚庝繚鎸佸叾浠栫嚎绋嬭繍琛�
	while(true)
>>>>>>> origin/master
	{
		;
	}
	outfile.close();
	return 0;
}
