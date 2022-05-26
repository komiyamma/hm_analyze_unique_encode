// ConsoleApplication50.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

#include <iostream>
#include <string>
#include <vector>
#include <vector>
#include <windows.h>

using namespace std;

std::wstring utf8_to_utf16(const std::string& text)
{
	int size = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, NULL, 0);
	std::vector<wchar_t> buf(size);
	size = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, &buf[0], (int)buf.size());
	return std::wstring(buf.begin(), buf.begin() + size).data(); // このdata()必要
}

std::string utf16_to_utf8(const std::wstring& text)
{
	int size = WideCharToMultiByte(CP_UTF8, 0, text.c_str(), -1, NULL, 0, 0, NULL);
	std::vector<char> buf(size);
	size = WideCharToMultiByte(CP_UTF8, 0, text.c_str(), -1, &buf[0], (int)buf.size(), 0, NULL);
	return std::string(buf.begin(), buf.begin() + size).data(); // このdata()必要
}

std::wstring cp932_to_utf16(const std::string& text)
{
	int size = MultiByteToWideChar(CP_OEMCP, 0, text.c_str(), -1, NULL, 0);
	std::vector<wchar_t> buf(size);
	size = MultiByteToWideChar(CP_OEMCP, 0, text.c_str(), -1, &buf[0], (int)buf.size());
	return std::wstring(buf.begin(), buf.begin() + size).data(); // このdata()必要
}

std::string utf16_to_cp932(const std::wstring& text)
{
	int size = WideCharToMultiByte(CP_OEMCP, 0, text.c_str(), -1, NULL, 0, 0, NULL);
	std::vector<char> buf(size);
	size = WideCharToMultiByte(CP_OEMCP, 0, text.c_str(), -1, &buf[0], (int)buf.size(), 0, NULL);
	return std::string(buf.begin(), buf.begin() + size).data(); // このdata()必要
}

extern bool encode_zen_han_map[65536];

static int nZenkaku(wchar_t wch) {
	int ix = wch;
	if (encode_zen_han_map[ix]) {
		return 0;
	}
	return 8;
}



static vector<BYTE> ToOriginalHmStarUnicode(wchar_t wch) {
	/*
	DWORD star_uni = 0;
	char *p = (char *) &star_uni;

	*p = '\x1A';
	WORD* pw = (WORD*)(p + 1);
	*pw = wch | 0x8080;
	UINT u = 0; // nZenkaku(wch);
	p[3] = ((wch & 0x80) >> 7) + ((wch & 0x8000) >> 14) + 4 + u;
	vector<BYTE> ret;
	for (int i = 0; i < 4; i++) {
		ret.push_back((BYTE)p[i]);
	}
	cout << star_uni << endl;
	return ret;
	*/
	vector<BYTE> ret;
	ret.push_back(0x1A);
	ret.push_back(0x80 | LOBYTE(wch));
	ret.push_back(0x80 | HIBYTE(wch));
	BYTE byte4ix = ((wch & 0x80) >> 7) + ((wch & 0x8000) >> 14) + 4 + nZenkaku(wch);
	ret.push_back(byte4ix);

	// DWORD* d = (DWORD *) & ret[0];
	// cout << *d << endl;

	return ret;
}


vector<BYTE> EncodeWStringToOriginalEncodeVector(wstring original_string) {
	vector<BYTE> r;
	for (wchar_t ch : original_string) {
		vector<BYTE> byte4 = ToOriginalHmStarUnicode(ch);
		for (BYTE b : byte4) {
			r.push_back(b);
		}
	}

	r.push_back(0);
	return r;
}

vector<BYTE> EncodeWStringToOriginalEncodeVector2(wstring original_string) {
	vector<BYTE> r;
	for (wchar_t wch : original_string) {

		wstring wstr = L"";
		wstr += wch;
		string str = utf16_to_cp932(wstr);
		wstring wstr2 = cp932_to_utf16(str);

		// 同じならCP932に存在するコード
		if (wstr == wstr2) {
			for (char ch : str) {
				r.push_back(ch);
			}
		}

		else {

			// 異なるならCP932には存在しないコード
			vector<BYTE> byte4 = ToOriginalHmStarUnicode(wch);
			for (BYTE b : byte4) {
				r.push_back(b);
			}
		}
	}

	// 最後にNULL相当ついイア
	r.push_back(0);
	return r;
}

int main()
{

	for (int i = 0; i <= 0xFFFF; i++) {
		wchar_t wch = (wchar_t)i;
		wstring wstr = L"";
		wstr += wch;
		auto ret = EncodeWStringToOriginalEncodeVector2(wstr);
		DWORD d = 0;
		BYTE* p = (BYTE*)&d;
		for ( int i=0; i< ret.size()-1; i++) {
			*p = ret[i];
			p++;
		}
		cout << d << endl;
		/*
		cout << encode_zen_han_map[i] << ",";
		if (i % 128 == 127) {
			cout << endl;
		}
		*/
		// printf("%x\n", (int)i);
	}
}

// プログラムの実行: Ctrl + F5 または [デバッグ] > [デバッグなしで開始] メニュー
// プログラムのデバッグ: F5 または [デバッグ] > [デバッグの開始] メニュー

// 作業を開始するためのヒント: 
//    1. ソリューション エクスプローラー ウィンドウを使用してファイルを追加/管理します 
//   2. チーム エクスプローラー ウィンドウを使用してソース管理に接続します
//   3. 出力ウィンドウを使用して、ビルド出力とその他のメッセージを表示します
//   4. エラー一覧ウィンドウを使用してエラーを表示します
//   5. [プロジェクト] > [新しい項目の追加] と移動して新しいコード ファイルを作成するか、[プロジェクト] > [既存の項目の追加] と移動して既存のコード ファイルをプロジェクトに追加します
//   6. 後ほどこのプロジェクトを再び開く場合、[ファイル] > [開く] > [プロジェクト] と移動して .sln ファイルを選択します
