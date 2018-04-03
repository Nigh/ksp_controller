
#include <DueTimer.h>
#include <Joystick.h>
#include <Keyboard.h>

typedef struct
{
	int pin;
	unsigned char delay;
	bool statu;
	void (*push)(void);
	void (*release)(void);
}ISR_G;

int ledPin[5]={
	26,28,30,32,34,
};

int switchPin[5]={
	27,29,31,33,35,
};

int numPin[10]={
	44,46,48,50,52,
	45,47,49,51,53,
};

int metalPin[4]={
	37, 39, 41, 43,
};

void m_down0(void);
void m_down1(void);
void m_down2(void);
void m_down3(void);

void k_down0(void);
void k_down1(void);
void k_down2(void);
void k_down3(void);
void k_down4(void);
void k_down5(void);
void k_down6(void);
void k_down7(void);
void k_down8(void);
void k_down9(void);

#define ISR_MAX (sizeof(isr)/sizeof(ISR_G))
ISR_G isr[]={
	{metalPin[0],0,true,m_down0,NULL},
	{metalPin[1],0,true,m_down1,NULL},
	{metalPin[2],0,true,m_down2,NULL},
	{metalPin[3],0,true,m_down3,NULL},
	{numPin[0],0,true,k_down0,NULL},
	{numPin[1],0,true,k_down1,NULL},
	{numPin[2],0,true,k_down2,NULL},
	{numPin[3],0,true,k_down3,NULL},
	{numPin[4],0,true,k_down4,NULL},
	{numPin[5],0,true,k_down5,NULL},
	{numPin[6],0,true,k_down6,NULL},
	{numPin[7],0,true,k_down7,NULL},
	{numPin[8],0,true,k_down8,NULL},
	{numPin[9],0,true,k_down9,NULL},
};
// Create Joystick
// Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID,JOYSTICK_TYPE_JOYSTICK,
// 	16,2,true,true,true,
// 	true,true,true,
// 	false,true,false,true,true);
Joystick_ Joystick;

void joystick_setting(void){
	// Joystick.buttonCount=16;
}


int x_center=0,y_center=0,z_center=0,x2_center=0,y2_center=0;

void calibrate(void){
	for (int i = 0; i < 16; ++i)
	{
		delay(30);
		x_center+=analogRead(A0);
		y_center+=analogRead(A1);
		z_center+=analogRead(A2);
		y2_center+=analogRead(A3);
		x2_center+=analogRead(A4);
	}
	x_center/=16;
	y_center/=16;
	z_center/=16;
	x2_center/=16;
	y2_center/=16;
}

int getAxisX2(int ad){
	return (ad-x2_center)/4;
}
int getAxisY2(int ad){
	return (ad-y2_center)/4;
}

int getAxisX(int ad){
	return (ad-x_center)/4;
}

int getAxisY(int ad){
	return (ad-y_center)/4;
}

int getAxisZR(int ad){
	return (ad-z_center)/4;
}

void setup() {
	Serial.begin(115200);
	// Serial.println(ISR_MAX);
	// Set Range Values
	Joystick.setXAxisRange(-127, 127);
	Joystick.setYAxisRange(-127, 127);
	Joystick.setZAxisRange(-127, 127);
	Joystick.setRxAxisRange(-127, 127);
	Joystick.setRyAxisRange(-127, 127);
	Joystick.setRzAxisRange(-127, 127);
	Joystick.setThrottleRange(0, 255);
	// Joystick.setRudderRange(255, 0);

	for (int i = 0; i < 5; ++i){
		pinMode(ledPin[i],OUTPUT);
	}
	for (int i = 0; i < 5; ++i){
		pinMode(switchPin[i],INPUT_PULLUP);
	}

	for (int i = 0; i < 4; ++i){
		pinMode(metalPin[i],INPUT_PULLUP);
	}
	for (int i = 0; i < 10; ++i) {
		pinMode(numPin[i],INPUT_PULLUP);
	}

	Serial.println("begin");
	while(1){
		if(!digitalRead(37)) break;
		delay(50);
	}
	Serial.println("start");
	noInterrupts();

	for (int i = 0; i < ISR_MAX; ++i) {
		isr_ctrl(i,true);
	}

	Joystick.begin(false);
	Keyboard.begin();
	Serial.println("Joystick start");
	DueTimer::getAvailable().attachInterrupt(timer100hz).start(10000);
	// Timer3.attachInterrupt(timer100hz).start(10000);

	interrupts();
	// automatic calibrate
	calibrate();
}

void loop() {
	// unsigned int a[4];
	// a[3]=analogRead(A5);
	// Serial.println("========");
	// Serial.println(a[0]);
	// Serial.println(a[1]);
	// Serial.println(a[2]);
	// Serial.println(a[3]);
	// Serial.println("");
	delay(1000);
}

void timer100hz(void){
	static unsigned char t=0;
	if(t++>=10){
		t=0;
		// Serial.print("x=");Serial.println(analogRead(A4));
		// Serial.print("y=");Serial.println(analogRead(A3));
		// for (int i = 0; i < 5; ++i) {
		// 	Serial.println(digitalRead(switchPin[i]));
		// 	digitalWrite(ledPin[i],digitalRead(switchPin[i]));
		// }
	}
	Joystick.setXAxis(getAxisX(analogRead(A0)));
	Joystick.setYAxis(getAxisY(analogRead(A1)));
	Joystick.setRzAxis(getAxisZR(analogRead(A2)));
	Joystick.setRxAxis(getAxisX2(analogRead(A4)));
	Joystick.setRyAxis(getAxisY2(analogRead(A3)));
	Joystick.setThrottle(analogRead(A5)/4);
	for (int i = 0; i < ISR_MAX; ++i)
	{
		if(isr[i].delay<10){
			isr[i].delay+=1;
		}
		if(isr[i].statu==false && digitalRead(isr[i].pin)==1){
			if(isr[i].delay>3){
				if(isr[i].release!=NULL) isr[i].release();
				isr_ctrl(i,true);
			}
		}
	}
	Joystick.sendState();
}

int test=0;
#define mdown(x) void m_down##x(void) {\
	if(isr[x].delay<4)return; isr_ctrl(x,false);\
	Serial.println(test++);\
	Serial.println("m key down" #x "detected");\
}
#define kdown(x) void k_down##x(void) {\
	if(isr[x].delay<4)return; isr_ctrl(x,false);\
	Serial.println(test++);\
	Serial.println("n key down" #x "detected");\
}

mdown(0);
mdown(1);
mdown(2);
mdown(3);

kdown(0);
kdown(1);
kdown(2);
kdown(3);
kdown(4);
kdown(5);
kdown(6);
kdown(7);
kdown(8);
kdown(9);

// void m_down0(void) {
// 	if(isr[0].delay<2)return; isr_ctrl(0,false);
// 	Serial.println(test++);
// 	Serial.println("key down 0 detected");
// }

// void key_up0(void) {
// 	if(isr[0].delay<2)return;
// 	Serial.println("key up 0 detected\n");
// }

void isr_ctrl(unsigned char isr_n,bool en){
	if(en){
		attachInterrupt(isr[isr_n].pin,isr[isr_n].push,FALLING);
	}else{
		detachInterrupt(isr[isr_n].pin);
	}
	isr[isr_n].statu=en;
	isr[isr_n].delay=0;
}
