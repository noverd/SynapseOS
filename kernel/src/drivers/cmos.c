#include <kernel.h>

#define CURRENT_YEAR        2022    // Change this each year!
 
int32_t century_register = 0x00;     // Set by ACPI table parsing code if possible
 
unsigned char second;
unsigned char minute;
unsigned char hour;
unsigned char day;
unsigned char month;
uint32_t year;
unsigned char century;

unsigned char last_second;
unsigned char last_minute;
unsigned char last_hour;
unsigned char last_day;
unsigned char last_month;
unsigned char last_year;
unsigned char last_century;
unsigned char registerB;

static const unsigned char synapse_months[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
static const unsigned char synapse_months_leap[] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

enum {
    cmos_address = 0x70,
    cmos_data    = 0x71
};
 
int32_t get_update_in_progress_flag() {
    outb(cmos_address, 0x0A);
    return (inb(cmos_data) & 0x80);
}
 
unsigned char get_RTC_register(int32_t reg) {
    outb(cmos_address, reg);
    return inb(cmos_data);
}
 
void read_rtc() {

 
    // Note: This uses the "read registers until you get the same values twice in a row" technique
    //     to avoid getting dodgy/inconsistent values due to RTC updates
 
    while (get_update_in_progress_flag());          // Make sure an update isn't in progress
    second = get_RTC_register(0x00);
    minute = get_RTC_register(0x02);
    hour = get_RTC_register(0x04);
    day = get_RTC_register(0x07);
    month = get_RTC_register(0x08);
    year = get_RTC_register(0x09);
    if(century_register != 0) {
        century = get_RTC_register(century_register);
    }
 
    do {
        last_second = second;
        last_minute = minute;
        last_hour = hour;
        last_day = day;
        last_month = month;
        last_year = year;
        last_century = century;
 
        while (get_update_in_progress_flag());       // Make sure an update isn't in progress
        second = get_RTC_register(0x00);
        minute = get_RTC_register(0x02);
        hour = get_RTC_register(0x04);
        day = get_RTC_register(0x07);
        month = get_RTC_register(0x08);
        year = get_RTC_register(0x09);
        if(century_register != 0) {
            century = get_RTC_register(century_register);
        }
    } while( (last_second != second) || (last_minute != minute) || (last_hour != hour) ||
           (last_day != day) || (last_month != month) || (last_year != year) ||
           (last_century != century) );
 
    registerB = get_RTC_register(0x0B);
 
    // Convert BCD to binary values if necessary
 
    if (!(registerB & 0x04)) {
        second = (second & 0x0F) + ((second / 16) * 10);
        minute = (minute & 0x0F) + ((minute / 16) * 10);
        hour = ( (hour & 0x0F) + (((hour & 0x70) / 16) * 10) ) | (hour & 0x80);
        day = (day & 0x0F) + ((day / 16) * 10);
        month = (month & 0x0F) + ((month / 16) * 10);
        year = (year & 0x0F) + ((year / 16) * 10);
        if(century_register != 0) {
            century = (century & 0x0F) + ((century / 16) * 10);
        }
    }
 
    // Convert 12 hour clock to 24 hour clock if necessary
 
    if (!(registerB & 0x02) && (hour & 0x80)) {
        hour = ((hour & 0x7F) + 12) % 24;
    }
 
    // Calculate the full (4-digit) year
 
    if(century_register != 0) {
        year += century * 100;
    } else {
        year += (CURRENT_YEAR / 100) * 100;
        if(year < CURRENT_YEAR) year += 100;
    }
}

int isleap(int year) {
    return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
}

struct synapse_time get_time() {
    read_rtc();
	struct synapse_time time = {
		second, minute, hour, day, month, year, century
	};
	return time;
}

/*
def unx():
    t = time.localtime()
    mn = [31, 29 if is_leap(t.tm_year) else 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31]
    total = 0
    total += t.tm_sec
    total += t.tm_min*60
    total += t.tm_hour*60*60
    total += t.tm_mday*60*60*24
    total += t.tm_mon*60*60*24*mn[t.tm_mon-1]
    total -= 33555600
    total += (t.tm_year-1970)*60*60*24*mn[t.tm_mon-1]*12
    return total
*/

unsigned int synapse_time_to_unix(struct synapse_time ktime) {
	unsigned int t = 0;
	unsigned char cmdt = (isleap(ktime.year)?synapse_months_leap[ktime.month]:synapse_months[ktime.month]);

	t += ktime.seconds;
	t += ktime.minutes*60;
	t += ktime.hours*60*60;
	t += ktime.day*60*60*24;
	t += ktime.month*60*60*24*cmdt;
	t -= 33555600;
	t += (ktime.year-1970)*60*60*24*cmdt*12;
    return t;
}
