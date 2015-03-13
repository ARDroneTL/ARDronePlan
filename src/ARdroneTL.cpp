// ��Ŀ����ARDroneTL 
// ��  �ߣ�͵������Ա�Ŷ�
// ʱ  �䣺2015.03.13

#include <stdio.h>  
#include <winsock2.h>  
#include <assert.h>
#include <iostream>
#include <fstream>
// C++11ͷ�ļ�
#include <mutex>
#include <thread>
// ���ӿ�
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
//����ͷ�ļ�
#include "MemoryLibrary.h"
#pragma comment(lib, "ws2_32.lib") 

using namespace std;

/*************************************************
* ������������ģ��
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
* ��������ģ��
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

// UI �ؼ�������
struct ARDrone_UI
{
	GtkTextBuffer*	buffer;		// ����ı�������
	GtkWidget*		window;		// ����
	GtkWidget*		view;		// 
	GtkWidget*		box;		// ����
	GtkWidget*		button;		// ��ť
};
ARDrone_UI* arui = new ARDrone_UI();	// ȫ�ֵ�ui �ؼ�

// �����壺Buffer
union INT_FLOAT_BUFFER
{
	float	fBuff;
	int		iBuff;
};

/*
* ARDrone �Զ��������ϸ����
*
*�����Ʋ�����
* ����ɣ���ɡ����䡢ǰ�������ˡ�����ɡ����ҷɡ������ٶ�
*����Ա����������
* ����ɣ���ȡ��ǰ��š���ȡǰ��š���ȡ����š�����ǰ���
*������ָ�
* ���ͻ���ָ����ͷ��п���ָ��
*����ʼ��������
* ���ͷ�������ʼ������ָ�socket��ʼ������
*������������
* ����ɣ����ݰ�������float����ת��int����
* ����ɣ���
*�����г�Ա������
* �������ݡ�ǰһ��ָ��
*��˽�г�Ա������
* ����ָ����׽��֣�һ�ף��������ٶȡ����������֡�����š�ǰ����š�������
*
* ԭ������Ϊ��������Ϊ
*/
class ARDrone
{
public:
	ARDrone(void){}
	ARDrone(char*);
	~ARDrone(void);

public:
	// ����������
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

	void setSpeed(int);		// ���÷����ٶ�

public:
	int		getCurrentSeq(){ return this->seq_; }			// get the current sequence
	int		getLastSeq(){ return this->lastSeq_; }		// get the last sequence
	int		getNextSeq();								// get the next sequence
	void	setLastSeq(int);							// set the last sequence

	int		send_at_cmd(char*);					// send all command
	int		send_pcmd(int, float, float, float, float);	// send control command
	void	parse(MemoryLibrary::Buffer&);				// ���ݰ�����

protected:
	void	initializeCmd();				// initialize command
	void	initializeSocketaddr();			// initialize sockaddr_in
	int		floatToInt(float);				// ʹ��������ʵ��float ת��int	

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
* ARDrone ���Ա������ʵ��ģ�飨model��
**************************************************/
ARDrone::ARDrone(char* name)
{
	this->name_ = name;
	// ��WSAStartup ����Ws2_32.lib
	WORD socketVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	assert(WSAStartup(socketVersion, &wsaData) == 0);

	// ��Ա������ʼ��
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

	WSACleanup();					// �ͷ�Winsock��
	closesocket(this->socketat_);	// �ر�SOCKET
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

// initialize command��û��ʹ�ã�
void ARDrone::initializeCmd()
{
	char cmd[1024];
	// �������߶�
	sprintf_s(cmd, "AT*CONFIG=%d,\"control:altitude_max\",\"2000\"\r", getNextSeq());
	assert(send_at_cmd(cmd));
	Sleep(INTERVAL);

	// ????
	sprintf_s(cmd, "AT*CONFIG=%d,\"control:control_level\",\"0\"\\r", getNextSeq());
	assert(send_at_cmd(cmd));
	Sleep(INTERVAL);

	// ���ó�����Ƶ��
	sprintf_s(cmd, "AT*CONFIG=%d,\"pic:ultrasound_freq\",\"8\"\r", getNextSeq());
	assert(send_at_cmd(cmd));
	Sleep(INTERVAL);
	//flat trim
	sprintf_s(cmd, "AT*FTRIM=%d\r", getNextSeq());
	assert(send_at_cmd(cmd));
	Sleep(INTERVAL);
}

// ���п��ƺ���ʵ�ֲ���
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

//�����⣿������������������������������������������������
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
	// ������:���ڶ��߳�
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
	// ������:���ڶ��߳�C++11
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
* ��Ϣ��Ӧ����ģ�飨control��
*********************************/

// ��������
void destroy(GtkWidget *widget, gpointer data)
{
	gtk_main_quit();
}

// ���̿�����Ϣ��Ӧ����
void control(GtkWidget* widget, GdkEventKey* event, gpointer data)
{
	ARDrone* ardrone = (ARDrone*)data;
	guint keyValue = event->keyval;
	switch (keyValue)
	{
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
		printf("ָ�֧��\n"); 
		outfile << "ָ�֧��" << endl; break;
	}
}

void keyReleaseEvent(GtkWidget* widget, GdkEventKey* event, gpointer data)
{
	ARDrone* ardrone = (ARDrone*)data;
	guint keyValue = event->keyval;
	outfile << keyValue << endl;
	if (keyValue != 49 || keyValue != 50) 
		return;
	ardrone->hover();
}

/******************************************
* UI ģ�� (VIEW)
******************************************/
// UI ��ʼ��
void initGtk(int argc, char* argv[], ARDrone* ardrone)
{
	gtk_init(&argc, &argv);

	// ������������
	arui->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(arui->window), "myARDrone");
	gtk_window_set_default_size(GTK_WINDOW(arui->window), 400, 300);
	gtk_window_set_position(GTK_WINDOW(arui->window), GTK_WIN_POS_CENTER);
	gtk_container_set_border_width(GTK_CONTAINER(arui->window), 40);
	/*
	// �ؼ�����
	arui->box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);		// �����ؼ���ˮƽ��λ
	gtk_container_add(GTK_CONTAINER(arui->window), arui->box);
	arui->buffer = gtk_text_buffer_new(NULL);
	arui->view = gtk_text_view_new_with_buffer(arui->buffer);
	gtk_box_pack_start(GTK_BOX(arui->box), arui->view, TRUE, TRUE, 10);
	gtk_widget_set_size_request(arui->view, 10, 15);
	*/
	// ��Ϣ��Ӧ
	g_signal_connect(G_OBJECT(arui->window), "destroy", G_CALLBACK(destroy), NULL);
	g_signal_connect(G_OBJECT(arui->window), "key_press_event", G_CALLBACK(control), ardrone);
	//g_signal_connect(G_OBJECT(arui->window), "key_release_event", G_CALLBACK(keyReleaseEvent), ardrone);

	gtk_widget_show_all(arui->window);
}

/******************************************
* �����߳�ģ��
******************************************/

// ������Ardrone���ӵ��߳�
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

// ��ȡ�������ݵ��߳�
void NavDataThread(ARDrone* ardrone)
{
	SOCKET socketNav_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	sockaddr_in	navSin_;
	navSin_.sin_family = AF_INET;
	navSin_.sin_port = htons(NAVDATA_PORT);
	navSin_.sin_addr.s_addr = inet_addr(ARDrone_IP);
	int lenNavSin_ = sizeof(navSin_);

	// ����ָ���NAVDATA_PORT �˿�
	const char trigger[4] = { 0x01, 0x00, 0x00, 0x00 };
	int result = sendto(socketNav_, trigger, strlen(trigger), 0, (sockaddr *)&navSin_, lenNavSin_);
	if (result != SOCKET_ERROR)
	{
		printf_s("Sent trigger flag to UDP port : %d \n", NAVDATA_PORT);
		outfile << "Sent trigger flag to UDP port : " << NAVDATA_PORT << endl;
	}

	// ����ָ���AT_PORT �˿�
	char initCmd[1024] = { 0 };
	sprintf_s(initCmd, "AT*CONFIG=%d,\"general:navdata_demo\",\"TRUE\"\r", ardrone->getNextSeq());
	assert(ardrone->send_at_cmd(initCmd));

	// �������ݰ�
	MemoryLibrary::Buffer navDataBuffer;	// ���������ݻ�����
	char recv[1024] = { 0 };				// ���ݰ���������
	int lenRecv = 0;
	int delay = 0;
	// UI ����
	char text[100] = { 0 };					// ����������ʾ�ַ���
	
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
				, ardrone->navData.batteryLevel, ardrone->navData.altitude);
			//outfile <<text;
			// gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(arui->buffer), &start, &end);    //��û�������ʼ�ͽ���λ�õ�Iter	
			// gtk_text_buffer_set_text(GTK_TEXT_BUFFER(arui->buffer), text, -1);		//�����ı���������

		}
	}
}

/******************************************
* ������ģ��
******************************************/
int main(int argc, char* argv[])
{
	outfile.open("data.txt");
	if(!outfile.is_open())
		return 0;
	ARDrone* ardrone = new ARDrone("myardrone");
	initGtk(argc, argv, ardrone);

	// C++11 �����������ݵ��߳�
	std::thread navThread(NavDataThread, ardrone);
	std::thread weakThread(weakUpThread, ardrone);

	// UIѭ��
	gtk_main();

	// ��ѭ����UI�رպ󱣳������߳�����
	while (true)
	{
		;
	}
	outfile.close();
	return 0;
}