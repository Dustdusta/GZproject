#include <fstream>
#include <vector>
static uint8_t U8[4] = { 0,0,0,0 };//unsigned int转uint8_t的输出结果
std::vector<uint8_t> FinalFile;//最后的文件流

void ConvertType(unsigned int UI)
{
	U8[0] = (uint8_t)UI;
	U8[1] = (uint8_t)(UI >>= 8);
	U8[2] = (uint8_t)(UI >>= 8);
	U8[3] = (uint8_t)(UI >>= 8);
}
void GetCrc32(uint8_t* InStr, unsigned int len)
{
	//生成Crc32的查询表
	unsigned int Crc32Table[256];
	int i, j;
	unsigned int Crc;
	for (i = 0; i < 256; i++)
	{
		Crc = i;
		for (j = 0; j < 8; j++)
		{
			if (Crc & 1)
				Crc = (Crc >> 1) ^ 0xEDB88320;
			else
				Crc >>= 1;
		}
		Crc32Table[i] = Crc;
	}

	//开始计算CRC32校验值
	Crc = 0xffffffff;
	for (int i = 0; i < len; i++)
	{
		Crc = (Crc >> 8) ^ Crc32Table[(Crc & 0xFF) ^ InStr[i]];
	}

	Crc ^= 0xFFFFFFFF;
	ConvertType(Crc);
}

//输出最后的压缩文件
void CreateFile()
{
	FILE* fp_out = nullptr;
	fopen_s(&fp_out, "test.gz", "wb");
	if (fp_out != nullptr)
	{
		int size_out = FinalFile.size();
		fwrite(FinalFile.data(), 1, size_out, fp_out);
		fclose(fp_out);
	}
}

//存储压缩
void StaticCompress()
{
	FILE* fp_in = nullptr;
	const char Filename[] = "abc.txt";
	fopen_s(&fp_in, Filename, "rb");
	std::vector<uint8_t> ReadFile;
	if (fp_in != nullptr)
	{
		std::fseek(fp_in, 0L, SEEK_END);
		int size_in = ftell(fp_in);
		std::fseek(fp_in, 0, SEEK_SET);
		ReadFile.resize(size_in);
		fread(ReadFile.data(), sizeof(uint8_t), size_in, fp_in);
		fclose(fp_in);
	}
	for (auto i : Filename)
	{
		FinalFile.push_back(i);
	}

	uint8_t Header[5] = { 0x01,0x01,0x00,0xFE,0XFF };
	for (int i : {0, 1, 2, 3, 4})
	{
		FinalFile.push_back(Header[i]);
	}

	FinalFile.insert(FinalFile.end(), ReadFile.begin(), ReadFile.end());

	GetCrc32(ReadFile.data(), ReadFile.size());
	for (int i : {0, 1, 2, 3})
	{
		FinalFile.push_back(U8[i]);
	}
	ConvertType(ReadFile.size());
	for (int i : {0, 1, 2, 3})
	{
		FinalFile.push_back(U8[i]);
	}
}

int main()
{
	/*FLG的标示符作用
	Bit 0        FTEXT
	Bit 1        FHCRC
	Bit 2        FEXTRA
	Bit 3        FNAME
	Bit 4        FCOMMENT
	Bit 5~7   预留，必须全0*/
	uint8_t IDCMFLG[4] = { 0x1F,0x8B,0x08,0x08 };
	uint8_t MTINE[4] = { 0x00,0x00,0x00,0x00 };
	/*当CM = 8时，XFL = 2表示最大压缩但最慢的算法；XFL = 4表示最快但最小压缩的算法*/
	uint8_t XFL[5] = { 0x00,0x01,0x02,0x03,0x04 };
	/*0 - FAT filesystem(MS - DOS, OS / 2, NT / Win32)
	1 - Amiga
	2 - VMS(or OpenVMS)
	3 - Unix
	4 - VM / CMS
	5 - Atari TOS
	6 - HPFS filesystem(OS / 2, NT)
	7 - Macintosh
	8 - Z - System
	9 - CP / M
	10 - TOPS - 20
	11 - NTFS filesystem(NT)
	12 - QDOS
	13 - Acorn RISCOS
	255 - unknown*/
	uint8_t OS[15] = { 0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0xFF };

	for (int i : {0, 1, 2, 3})
	{
		FinalFile.push_back(IDCMFLG[i]);
	}
	for (int i : {0, 1, 2, 3})
	{
		FinalFile.push_back(MTINE[i]);
	}
	FinalFile.push_back(XFL[2]);
	FinalFile.push_back(OS[0]);

	FILE* fp_in = nullptr;
	const char Filename[] = "abc.txt";
	fopen_s(&fp_in, Filename, "rb");
	std::vector<uint8_t> ReadFile;
	if (fp_in != nullptr)
	{
		std::fseek(fp_in, 0L, SEEK_END);
		int size_in = ftell(fp_in);
		std::fseek(fp_in, 0, SEEK_SET);
		ReadFile.resize(size_in);
		fread(ReadFile.data(), sizeof(uint8_t), size_in, fp_in);
		fclose(fp_in);
	}

	CreateFile();
	return 0;
}