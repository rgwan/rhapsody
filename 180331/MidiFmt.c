#include <stdio.h>
#include <stdint.h>
#include <windows.h>
#include <unistd.h>
#include <time.h>

/*
You 're a luck dog to find this stuff.
For hooking YAMAHA's Karaoke player, it can save MIDI file to your disk.

Author Zhiyuan Wan
WTFPL
*/

extern int __cdecl __declspec (dllexport)  MRACreateInstance(int a1, int a2);

typedef int __cdecl (*RealFunction) (int a1, int a2);

typedef int  __thiscall (*ORIGFUNC)(void *p, int a2, int a3, int a4, char a5);

volatile ORIGFUNC FuncSelect;

void *base_address;

FILE *fp;

static int lock = 0;

uint32_t calcMIDIlength(void *buffer)
{
	uint8_t *internalBuf = buffer;
	register uint32_t i = 0;
	register uint32_t chunksize;
	register uint32_t high;
	while(internalBuf[i] == 'M' || internalBuf[i] == 'X')
	{
		chunksize =  internalBuf[i + 7] + (internalBuf[i + 6] << 8);
		high = (internalBuf[i + 5] << 16) + (internalBuf[i + 4] << 24);
		fprintf(fp, "i = %08x, sizel = %08x, sizeh = %08x, %c\n", i, chunksize, high, internalBuf[i]);
		i += (4 + 4) + chunksize + high;
	}
	return i;
}

DWORD WINAPI ThreadToWrite(LPVOID pM)
{
	FILE *midifp;
	void *ptrmidi;
	char filename[64];
	char name[32];
	time_t timer;
	struct tm *tblock;

	//char filename[64];
	if(lock)
		return 0;
	lock = 1;

	fprintf(fp, "Child process ID = %d\n", (uint32_t)GetCurrentThreadId());
	fflush(fp);

	Sleep(1200);

	timer = time(NULL);
	tblock = localtime(&timer);

	memset(name, 0x00, sizeof(name));
	strncpy(name, base_address + 0x134, 31);

	ptrmidi = (void *)(*(uint32_t *)(base_address + 0x84));

	fprintf(fp, "Song name = %s\n", name);
	fprintf(fp, "String addr = %08x, MIDI buffer addr = %08x\n", (uint32_t)(base_address + 0x134), (uint32_t)ptrmidi);
	fprintf(fp, "Location = %08x\n", (uint32_t)(base_address + 0x84));
	fflush(fp);
	if(ptrmidi != 0)
	{
		uint32_t filesize = calcMIDIlength(ptrmidi);
		fprintf(fp, "MIDI file size = %08x %d\n", filesize, filesize);
		sprintf(filename, "%s.mid", name);
		int i;
		for(i = 0; i < strlen(filename); i++)
		{
			if(filename[i] == ':' || filename[i] == '?')
				filename[i] = ' ';
		}
		midifp = fopen(filename, "rb");
		if(midifp != 0)
		{
			fclose(midifp);
			sprintf(filename, "%s-%d%d%d%d%d.mid", name, tblock->tm_mon, tblock->tm_mday, tblock->tm_hour, tblock->tm_min, tblock->tm_sec);
		}
		midifp = fopen(filename, "wb+");
		if(midifp == 0)
		{
			sprintf(filename, "%d%d%d%d%d.mid", tblock->tm_mon, tblock->tm_mday, tblock->tm_hour, tblock->tm_min, tblock->tm_sec);
			midifp = fopen(filename, "wb+");
		}
		if(midifp != 0)
		{
			int ret = fwrite(ptrmidi, 1, filesize, midifp);
			int fret = fflush(midifp);
			int bytesWritten = ftell(midifp);
			fclose(midifp);
			fprintf(fp, "flush = %d, ret = %d, Write it to file %s, %d bytes\n", fret, ret, filename, bytesWritten);
		}
	}
	fprintf(fp, "Closing Log File\n");
	fflush(fp);
	fclose(fp);
	lock = 0;

	return 0;
}

int __thiscall FunctionSelect (void *p, int a2, int a3, int a4, char a5)
{
	/*
		ptr = 0382e5c4
		ptrmidi = 0382e644
		name = 0382e6f4
	*/
	int ret;
	HANDLE ht = CreateThread(NULL, 0, ThreadToWrite, NULL, 0, NULL);

	fprintf(fp, "Call Function select, %08x %08x\n", (uint32_t)p, (uint32_t)a2);
	base_address = p;
	ret = FuncSelect(p, a2, a3 ,a4, a5);
	fprintf(fp, "ret = %d, handle = %08x\n", ret, (uint32_t)ht);

	fflush(fp);

	return ret;
}

int __cdecl MRACreateInstance(int a1, int a2)
{

	int ret;
	volatile uint32_t classbase;
	volatile uint32_t base;
	volatile uint32_t *ptr;

	MEMORY_BASIC_INFORMATION MemInfo;

	fp = fopen("midifmt.txt", "w+");
	HMODULE mod;
	RealFunction MCI;
    mod = LoadLibrary("MidiFmt-Orig.dll");
	if(mod)
	{
		fprintf(fp, "Load Library OK!\n");
	}
	MCI = (RealFunction)GetProcAddress(mod, "MRACreateInstance");
	if(MCI != NULL)
	{
		fprintf(fp, "Get Function OK\n");
		ret = MCI(a1, a2);
		fprintf(fp, "Run Function OK, ret = %08x, a1 = %08x, a2 = %08x\n", ret, a1, a2);

		fprintf(fp, "Finding Read MIDI function...\n");
		classbase = *(int *)a1;
		fprintf(fp, "Class base addr = %08X\n", classbase);// base指向函数表
		base = *(int *)classbase;
        fprintf(fp, "base addr = %08X\n", (uint32_t) base);

		ptr = (uint32_t *) base + 20;//8;
		//FuncSelect = (ORIGFUNC)(*ptr); // Save original function pointer
		/*直接换吧
	offset addr = 0339d4e4
	func addr = 033860D0
		*/
		FuncSelect = (ORIGFUNC)((uint32_t)ptr & 0xffff0000) - 0x10000 + 0x7640;//+ 0x60d0;
		fprintf(fp, "offset addr = %08x\n", (uint32_t)ptr);
		fprintf(fp, "func addr = %08X\n", (uint32_t)FuncSelect);

		fprintf(fp, "my func addr = %08X\n", (uint32_t)&FunctionSelect);
#if 1
		VirtualQuery((LPCVOID)ptr, &MemInfo, sizeof(MEMORY_BASIC_INFORMATION));
		fprintf(fp, "Get page address = %08x, protect = %08x\n", (uint32_t)MemInfo.BaseAddress, (uint32_t)MemInfo.Protect);

		VirtualProtect(MemInfo.BaseAddress, MemInfo.RegionSize, PAGE_READWRITE, &MemInfo.Protect);
		VirtualQuery((LPCVOID)ptr, &MemInfo, sizeof(MEMORY_BASIC_INFORMATION));
		fprintf(fp, "Make page writable, state = %08x\n", (uint32_t)MemInfo.Protect);
#endif
		fflush(fp);

		//sleep(0.001);

		fprintf(fp, "Replace it with crack function\n");
		(*ptr) = (uint32_t)&FunctionSelect;

		fprintf(fp, "func addr = %08X\n", (*ptr));

		fflush(fp);
	}
	//fclose(fp);
	return ret;
}
