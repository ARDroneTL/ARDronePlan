// ��Ŀ����ARDroneTL 
// ��  �ߣ�͵������Ա�Ŷ�
// ʱ  �䣺2015.02.08 

#include <stdio.h>  
#include <winsock2.h>  
#include <assert.h>
#include <iostream>
// C++11ͷ�ļ�
#include <mutex>
#include <thread>
// ���ӿ�
#include <gtk/gtk.h>

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

const int INTERVAL      = 100;					// time interval
const char* ARDrone_IP	= "192.168.1.1";
const int C_OK			= 1;							
const int C_ERRO		= 0;


/*************************************************
* ��������ģ��
**************************************************/

// �����壺Buffer
union INT_FLOAT_BUFFER
{
	float	fBuff;
	int		iBuff;
};

// ����:ARDrone
class ARDrone
{
public:
	ARDrone(){}
	ARDrone(char* name, int argc, char* argv[]);
	~ARDrone(void);

public:
	// ����������
	void takeoff();
	void land();

	void goingUp();
	void goingDown();

	void turnLeft();
	void turnRight();

public:
	void	initGtk(int argc, char **argv);
	int		getSeq();																// get the lastest sequence
	int		send_at_cmd(const char* cmd);											// send all command
	int		send_pcmd(int enable, float roll, float pitch, float gaz, float yaw);	// send control command
	void	parse(MemoryLibrary::Buffer&);											// ���ݰ�����

protected:
	void	initializeCmd();				// initialize command
	void	initializeSocketaddr();			// initialize sockaddr_in
	int		floatToInt(float);				// ʹ��������ʵ��float ת��int	
	
public:
	int				seq_;				// sequence of data packet 
	int				lastSeq_;			// save the last seq
	const char*		at_cmd_last;		// save the last command

private:
	SOCKET		socketat_;		// socket
	sockaddr_in Atsin_;			// struct
	int			lenSin_;		// the length of sin
	float		speed_;			// fly speed
	char*		name_;
	std::mutex	mtx;			// mutex for critical section
};

/*************************************************
* ARDrone ���Ա������ʵ��ģ��
**************************************************/
ARDrone::ARDrone(char* name, int argc, char* argv[])
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
	at_cmd_last = "";

	//initGtk(argc, argv);
	cout << "ARDrone initialized" <<endl;
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
	Atsin_.sin_addr.S_un.S_addr = inet_addr(ARDrone_IP);
	std::cout << "IP:" << ARDrone_IP << "Port:" << AT_PORT << std::endl;
}

// initialize command��û��ʹ�ã�
void ARDrone::initializeCmd()
{
	char cmd[1024];
	// �������߶�
	sprintf_s(cmd, "AT*CONFIG=%d,\"control:altitude_max\",\"2000\"\r", getSeq());
	assert(send_at_cmd(cmd));
	Sleep(INTERVAL);

	// ????
	sprintf_s(cmd, "AT*CONFIG=%d,\"control:control_level\",\"0\"\\r", getSeq());
	assert(send_at_cmd(cmd));
	Sleep(INTERVAL);

	// ���ó�����Ƶ��
	sprintf_s(cmd, "AT*CONFIG=%d,\"pic:ultrasound_freq\",\"8\"\r", getSeq());
	assert(send_at_cmd(cmd));
	Sleep(INTERVAL);
	//flat trim
	sprintf_s(cmd, "AT*FTRIM=%d\r", getSeq());
	assert(send_at_cmd(cmd));
	Sleep(INTERVAL);
}

// ���п��ƺ���ʵ�ֲ���
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

// initialize gtk UI module
// ��չ����ʾʵʱ��Ƶ
void ARDrone::initGtk(int argc, char **argv)
{
	GtkWidget* window;
	gtk_init(&argc, &argv);
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);

	// ���Խ׶Σ�ʹ�ü��̿���
	//g_signal_connect(G_OBJECT(window), "key-press-event", G_CALLBACK(control), this);

	gtk_widget_show_all(window);
	gtk_main();
}

// get the lastest sequence
int ARDrone::getSeq()
{
	// ������:���ڶ��߳�
	this->mtx.lock();
	seq_ += 1;
	this->mtx.unlock();
	return seq_;
}

// send control command
int ARDrone::send_pcmd(int enable, float roll, float pitch, float gaz, float yaw)
{
	char cmd[1024];
	sprintf_s(cmd, "AT*PCMD=%d,%d,%d,%d,%d,%d", getSeq(), enable,
			floatToInt(roll), floatToInt(pitch), floatToInt(gaz), floatToInt(yaw));
	int result = send_at_cmd(cmd);
	return result;
}

// send all command
int ARDrone::send_at_cmd(const char* cmd)
{
	// ������:���ڶ��߳�
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
}

/********************************
* ��Ϣ��Ӧģ��
*********************************/
// ���̿�����Ϣ��Ӧ����
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
			printf("ָ�֧��\n");break;
	}
}



/******************************************
* �����߳�ģ��
******************************************/

// ��ȡ�������ݵ��߳�
void NavDataThread(ARDrone* ardrone)
{
	SOCKET socketNav_		= socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	sockaddr_in	navSin_;
	navSin_.sin_family		= AF_INET;
	navSin_.sin_port		= htons(NAVDATA_PORT);
	navSin_.sin_addr.s_addr = inet_addr(ARDrone_IP);
	int lenNavSin_			= sizeof(navSin_);

	// ����ָ���NAVDATA_PORT �˿�
	const char trigger[4] = { 0x01, 0x00, 0x00, 0x00 };
	int result = sendto(socketNav_, trigger, strlen(trigger), 0, (sockaddr *)&navSin_, lenNavSin_);
	if (result != SOCKET_ERROR)
		printf_s("Sent trigger flag to UDP port : %d \n", NAVDATA_PORT);

	// ����ָ���AT_PORT �˿�
	char initCmd[1024] = { 0 };
	sprintf_s(initCmd, "AT*CONFIG=%d,\"general:navdata_demo\",\"TRUE\"\r", ardrone->getSeq());
	assert(ardrone->send_at_cmd(initCmd));
	
	// �������ݰ�
	char recv[1024] = {0};
	int lenRecv		= 0;
	int delay		= 0;
	MemoryLibrary::Buffer navDataBuffer;		// ���������ݻ�����

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
		}
	}
}

// ��Ƶ��ʾ�߳�

// ������ָ������߳�


/******************************************
* ������ģ��
******************************************/
int main(int argc, char* argv[])
{
	ARDrone* ardrone = new ARDrone("myardrone", argc, argv);

	// C++11 �����������ݵ��߳�
	std::thread navThread(NavDataThread, ardrone);
	char cmd[1024] = { 0 };

	int delay = 0;
	while (true)
	{
		// keep weak up
		Sleep(40);
		if (ardrone->seq_ == ardrone->lastSeq_)
			ardrone->send_at_cmd(ardrone->at_cmd_last);

		ardrone->lastSeq_ = ardrone->seq_;
		delay++;
		if (delay >= 4)
		{
			delay = 0;
			sprintf_s(cmd, "AT*COMWDG=%d\r", ardrone->getSeq());
			ardrone->send_at_cmd(cmd);
		}
	}

	return 0;
}

