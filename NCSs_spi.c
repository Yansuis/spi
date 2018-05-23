#include <stdio.h>//fprintf()
#include <stdlib.h>//exit()
#include <string.h>
#include <errno.h>
#include <unistd.h>//close

#include <wiringPi.h>
#include <wiringPiSPI.h>

#define err_sys(info)\
{\
	fprintf(stderr,"%s:%s\n",info,strerror(errno));\
	exit(EXIT_FAILURE);\
}

#define SPI_CHAN 0
#define SPI_SPEED 1000000

#define MAX_SIZE 1024*1024

int WiringPiSPISetup(int channel, int speed)
{
	int n;
	if((n=wiringPiSPISetup(channel,speed))<0)
	{
		err_sys("wiringPiSPISetup");
	}
	return n;
}

int WiringPiSetup(void)
{
	int n;
	if((n=wiringPiSetup())<0)
	{
		err_sys("wiringPiSetup");
	}
	return n;
}

int WiringPiSPIDataRW(int channel, char* data, int len)
{
	int n;
	if((n=wiringPiSPIDataRW(channel, data, len)) == -1)
	{
		printf("SPI Failed:%s\n",strerror(errno));
	}
	return n;
}

int main(void)
{
	int fd = -1;
	int start = 0;
	int end = 0;
	char *sendBuf;
	int spiFlag = 0;
	if((sendBuf = malloc(MAX_SIZE)) == NULL)
	{
		err_sys("malloc");
	}

	WiringPiSetup();

	for(int speed=1;speed<32;speed*=2)
	{
		printf ("+-------+--------+----------+----------+-----------+------------+\n") ;
		printf ("|   MHz |   Size | mS/Trans |      TpS |    Mb/Sec | Latency mS |\n") ;
		printf ("+-------+--------+----------+----------+-----------+------------+\n") ;
		fd = WiringPiSPISetup(SPI_CHAN,speed*SPI_SPEED);
		for(int size=1;size<MAX_SIZE;size*=2)
		{
			printf("| %5d | %6d ",speed, size);
			start = millis();
			for(int cnt=0;cnt<100;cnt++)
			{
				spiFlag = WiringPiSPIDataRW(SPI_CHAN,sendBuf,size);
				if (spiFlag == -1)
				{
					break ;
				}
			}
			end = millis();
			if(spiFlag == -1)
			{
				break;
			}
			double transferTime = (double)(end - start)/100/1000;//s
			double TpS = (size*8)/transferTime;
			double transferSpeed = (double)(size*8)/MAX_SIZE/1000000/transferTime;
			double latencyTime = (transferTime - (double)(size*8)/speed/1000000)*1000;
	printf("| %8.5f | %9.1f | %8.5f | %8.5f |\n",transferTime*1000,TpS,transferSpeed,latencyTime);
		}
		printf ("+-------+--------+----------+----------+-----------+------------+\n") ;
	}
	printf("%s\n",sendBuf);
	getchar();
	close(fd);
	return 0;
}

