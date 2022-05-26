// 秀丸スターユニコードからの復元 DecodeOriginalEncodeVector はおそらくこれで正しい
// EncodeWStringToOriginalEncodeVector2 の方はこれでは完全な再現ができない。
// おそらく 斎藤まるお 氏はなにか１つ言っていないことがある。


#include <windows.h>
#include <string>
#include <vector>
#include "convert_string.h"

static int nZenkaku(wchar_t wch) {
	wstring wstr = L"";
	wstr += wch;
	string u8 = utf16_to_utf8(wstr);

	WORD* pu8 = (WORD*)u8.c_str();
	WORD vu8 = *pu8;
	if (0x00 <= vu8 && vu8 <= 0x80) {
		return 0;
	}
	if (vu8 == 0xf8f0) {
		return 0;
	}
	if (0xff61 <= vu8 == vu8 <= 0xff9f) {
		return 0;
	}
	if (0xf8f1 <= vu8 == vu8 <= 0xf8f3) {
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