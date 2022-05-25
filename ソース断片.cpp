/*
static int nZenkaku(wchar_t wch) {
	wstring wstr = L"";
	wstr += wch;

	WORD* pw = (WORD*)wstr.c_str();
	WORD wv = *pw;
	if (0x00 <= wv && wv <= 0x80) {
		return 0;
	}
	if (wv == 0xf8f0) {
		return 0;
	}
	if (0xff61 <= wv == wv <= 0xff9f) {
		return 0;
	}
	if (0xf8f1 <= wv == wv <= 0xf8f3) {
		return 0;
	}
	return 8;
}



static vector<BYTE> ToOriginalHmStarUnicode(wchar_t wch) {

	vector<BYTE> ret;
	ret.push_back(0x1A);
	ret.push_back(0x80 + LOBYTE(wch));
	ret.push_back(0x80 + HIBYTE(wch));
	BYTE byte4ix = ((wch & 0x80) >> 7) + ((wch & 0x8000) >> 14) + 4 + nZenkaku(wch);
	ret.push_back(byte4ix);

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
*/








#include <windows.h>
#include <string>
#include <vector>
#include "convert_string.h"

using namespace std;

bool IsSTARTUNI_inline(DWORD byte4) {
	return (byte4 & 0xF4808000) == 0x04808000;
}

WCHAR inline GetUnicodeInText(char* pchSrc) {
	return MAKEWORD(
		(pchSrc[1] & 0x7F | ((pchSrc[3] & 0x01) << 7)),
		(pchSrc[2] & 0x7F | ((pchSrc[3] & 0x02) << 6))
	);
}


std::wstring DecodeOriginalEncodeVector(BYTE *original_encode_string) {
	try {
		// 独自エンコード
		string str_original_encode = (char *)original_encode_string;

		const char* pstr = str_original_encode.c_str();

		// 最終的な返り値
		wstring result = L"";

		// 一時バッファー用
		string tmp_buffer = "";
		int len = str_original_encode.size();

		int lastcheckindex = len - sizeof(DWORD); // IsSTARTUNI_inline には 4バイト必要
		if (lastcheckindex < 0) {
			lastcheckindex = 0;
		}
		for (int i = 0; i < (int)strlen(pstr); i++) {
			if (i <= lastcheckindex) {
				DWORD* pStarUni = (DWORD*)(&pstr[i]);
				if (IsSTARTUNI_inline(*pStarUni)) {
					if (tmp_buffer.length() > 0) {
						result += cp932_to_utf16(tmp_buffer);
						tmp_buffer.clear();
					}

					char* src = (char *)(&pstr[i]);
					wchar_t wch = GetUnicodeInText(src);
					i = i + 3; // 1バイトではなく4バイト消化したので、計算する
					result += wch;
					continue;
				}
			}
			tmp_buffer += pstr[i];
		}

		if (tmp_buffer.length() > 0) {
			result += cp932_to_utf16(tmp_buffer);
			tmp_buffer.clear();
		}

		return result;
	}
	catch (...) {
		OutputDebugString(L"エラー:\nHmOrignalUnicodeDecodeError");
	}

	return L"";
}