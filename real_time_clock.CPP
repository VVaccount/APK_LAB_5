#include <dos.h>
#include <stdio.h>
#include <conio.h>

struct VIDEO {
	unsigned char symb;
	unsigned char attr;
};
VIDEO text[] = {
	0x41,0x0F,0x6C,0x0F,0x61,0x0F,0x72,0x0F,0x6D,0x0F
};
void reset_alarm_clock();
void interrupt(*old_alarm_clock_handler) (...);
void interrupt new_alarm_clock_handler(...);
unsigned int sleep_duration = 0;
void interrupt(*old_RTC_handler) (...);
void interrupt new_RTC_handler(...);
int BCD_to_decimal(int number);
int decimal_to_BCD(int number);
void get_time();
void get_date();
void get_alarm_clock_time();
void set_time(int flag);
void delay();

int main() {
	char choice;
	clrscr();
	while (1) {
		printf("1. Set time\n");
		printf("2. Sleep (in ms)\n");
		printf("3. Set alarm clock\n");
		printf("4. Update time\n");
		printf("0. Exit\n\n");
		do {
			rewind(stdin);
			choice = getch();
		} while (choice < '0' || choice > '4');
		switch (choice) {
		case '0':
			reset_alarm_clock();
			return 0;
		case '1':
			clrscr();
			set_time(1);//для RTC
			get_date();
			get_time();
			break;
		case '2':
			clrscr();
			delay();
			break;
		case '3':
			clrscr();
			set_time(0);//для будильника
			get_alarm_clock_time();
			break;
		case '4':
			clrscr();
			get_date();
			get_time();
			break;
		}
	}
}

void interrupt new_alarm_clock_handler(...) {
	VIDEO far* screen = (VIDEO far *)MK_FP(0xB800, 0);
	screen += 80 * 12 + 35;
	VIDEO* it = text;
	screen->attr = 0x20;
	screen->symb = 0x20;
	for (int i = 0; i < 5; i++)
	{
		screen->attr = it->attr;
		screen->symb = it->symb;
		screen++;
		it++;
	}
}
void interrupt new_RTC_handler(...) {
	sleep_duration--;
	old_RTC_handler();
}
int BCD_to_decimal(int number) {
	return ((number / 16 * 10) + (number % 16));
}
int decimal_to_BCD(int number) {
	return ((number / 10 * 16) + (number % 10));
}
void get_time() {
	outp(0x70, 0x04); //часы
	printf("%02d:", BCD_to_decimal(inp(0x71)));
	outp(0x70, 0x02); //минуты
	printf("%02d:", BCD_to_decimal(inp(0x71)));
	outp(0x70, 0x00); //секунды
	printf("%02d\n\n", BCD_to_decimal(inp(0x71)));
}
void get_date() {
	char day_of_week[7][10] = { "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday" };
	outp(0x70, 0x07); //день
	printf("%02d.", BCD_to_decimal(inp(0x71)));
	outp(0x70, 0x08); //месяц
	printf("%02d.", BCD_to_decimal(inp(0x71)));
	outp(0x70, 0x09); //год
	printf("%02d\n", BCD_to_decimal(inp(0x71)));
	outp(0x70, 0x06); //день недели
	printf("%s\n", day_of_week[BCD_to_decimal(inp(0x71)) - 1]);
}
void get_alarm_clock_time() {
	outp(0x70, 0x05); //часы
	printf("Alarm:\n%02d:", BCD_to_decimal(inp(0x71)));
	outp(0x70, 0x03); //минуты
	printf("%02d:", BCD_to_decimal(inp(0x71)));
	outp(0x70, 0x01); //секунды
	printf("%02d\n\n", BCD_to_decimal(inp(0x71)));
}
void set_time(int flag) {
	int day;
	int month;
	int year;
	int day_of_week;
	int hours;
	int minutes;
	int seconds;
	printf("Hours: ");
	while (!scanf("%i", &hours) || hours < 0 || hours > 23) { //10/8/16 система счисления(удобная для пользователя)
		rewind(stdin);
	}
	hours = decimal_to_BCD(hours);
	printf("Minutes: ");
	while (!scanf("%i", &minutes) || minutes < 0 || minutes > 59) {
		rewind(stdin);
	}
	minutes = decimal_to_BCD(minutes);
	printf("Seconds: ");
	while (!scanf("%i", &seconds) || seconds < 0 || seconds > 59) {
		rewind(stdin);
	}
	seconds = decimal_to_BCD(seconds);
	if (flag) {
		printf("Day of month: ");
		while (!scanf("%i", &day) || day < 1 || day > 31) {
			rewind(stdin);
		}
		day = decimal_to_BCD(day);
		printf("Month: ");
		while (!scanf("%i", &month) || month < 1 || month > 12) {
			rewind(stdin);
		}
		month = decimal_to_BCD(month);
		printf("Year: ");
		while (!scanf("%i", &year) || year < 0 || year > 99) {
			rewind(stdin);
		}
		year = decimal_to_BCD(year);
		printf("Day of week: ");
		while (!scanf("%i", &day_of_week) || day_of_week < 1 || day_of_week > 7) {
			rewind(stdin);
		}
		day_of_week = decimal_to_BCD(day_of_week);
		int number_of_tries = 100;
		for (int i = 0; i < number_of_tries; i++) {
			if (!(inp(0x71) & 0x80)) { //7 бит A=0-готов к записи/чтению (& 10000000)
				outp(0x70, 0x0A);
				break;
			}
		}
		if (i == number_of_tries) {
			printf("Setting time error");
			return;
		}
		outp(0x70, 0x0B);
		outp(0x71, inp(0x71) | 0x80); //7 бит B=0-нормальный ход (| 10000000)-остановка часов,затрагивает только 1 бит
		outp(0x70, 0x07);
		outp(0x71, day);
		outp(0x70, 0x08);
		outp(0x71, month);
		outp(0x70, 0x09);
		outp(0x71, year);
		outp(0x70, 0x06);
		outp(0x71, day_of_week);
		outp(0x70, 0x04);
		outp(0x71, hours);
		outp(0x70, 0x02);
		outp(0x71, minutes);
		outp(0x70, 0x00);
		outp(0x71, seconds);
		outp(0x70, 0x0B);
		outp(0x71, inp(0x71) & 0x7F); //7 бит B=1-остановка часов (& 01111111)-нормальный ход,затрагивает только 1 бит
	}
	else {
		outp(0x70, 0x05);
		outp(0x71, hours);
		outp(0x70, 0x03);
		outp(0x71, minutes);
		outp(0x70, 0x01);
		outp(0x71, seconds);
		outp(0x70, 0xB);
		outp(0x71, (inp(0x71) | 0x20));
		old_alarm_clock_handler = getvect(0x4A);
		setvect(0x4A, new_alarm_clock_handler); // Set new handler of interrupt
		outp(0xA1, (inp(0xA0) & 0xFE));
	}
}
void reset_alarm_clock() {
	if (old_alarm_clock_handler == NULL) { //не установлен
		return;
	}
	setvect(0x4A, old_alarm_clock_handler);
	outp(0xA1, (inp(0xA0) | 0x01)); 
	outp(0x70, 0x05);	
	outp(0x71, 0x00);	
	outp(0x70, 0x03);	
	outp(0x71, 0x00);
	outp(0x70, 0x01);	
	outp(0x71, 0x00);
	outp(0x70, 0xB);
	outp(0x71, (inp(0x71) & 0xDF)); // 0xDF = 11011111, bit 5 to 0 to disable alarm (Alarm Interrupt disable)
}
void delay() {
	rewind(stdin);
	printf("Delay (in ms): ");
	scanf("%d", &sleep_duration);
	disable();
	old_RTC_handler = getvect(0x70);
	setvect(0x70, new_RTC_handler);
	enable();
	outp(0xA1, inp(0xA1) & 0xFE);//размаскировать прерывания от RTC(11111110)
	outp(0x70, 0xB);
	outp(0x71, inp(0x71) | 0x40);//разрешение периодических прерываний(01000000)
	while (sleep_duration > 0);
	disable();
	setvect(0x70, old_RTC_handler);
	enable();
	return;
}