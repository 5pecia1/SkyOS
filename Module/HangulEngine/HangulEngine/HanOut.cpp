#include "windef.h"
#include "HanOut.h"

#include "hanlib.h"      // �ѱ� ���̺귯�� 0.17
#include "hancode\hancode.h"    // �Ѷ����� 3.0 �ѱ��ڵ� ��ȯ ���̺귯�� 0.1
#include "hancode\hanin.h"      // "


TEngFont *pDefEngFont = NULL;
THanFont *pDefHanFont = NULL;
TSpcFont *pDefSpcFont = NULL;
THanjaFont *pDefHanjaFont = NULL;

static void CompleteHanChar(byte *ABuffer32, byte *AHanByte, THanFont *AHanFont)
{
    THangul _Hangul;
	bool flag = true;

    _Hangul.HanByte.Byte0 = AHanByte[0];
    _Hangul.HanByte.Byte1 = AHanByte[1];

    int F1 = _CodeTable[0][_Hangul.HanCode.F1];
    int F2 = _CodeTable[1][_Hangul.HanCode.F2];
    int F3 = _CodeTable[2][_Hangul.HanCode.F3];

    int F3B = AHanFont->pF3B[F2];
    int F2B = AHanFont->pF2B[F1 * 2 + (F3 != 0)];
    int F1B = AHanFont->pF1B[F2 * 2 + (F3 != 0)];

    if (F1) HanComplete(true, ABuffer32, AHanFont->F1[F1B][F1], 32), flag = false;
    if (F2) HanComplete(flag, ABuffer32, AHanFont->F2[F2B][F2], 32), flag = false;
    if (F3)	HanComplete(flag, ABuffer32, AHanFont->F3[F3B][F3], 32), flag = false;
}
//---------------------------------------------------------------------------
static void PutBitmap8x16_OVERWRITE(int x, int y, byte *ABitmap16)
{
    for (int i = 0; i < 16; i++) {
       // byte *p = (byte *)EngBitmap->ScanLine[i];
        //p[0] = ~ABitmap16[i];
    }
    //ACanvas->Draw(x, y, EngBitmap);
}
//---------------------------------------------------------------------------
static void PutBitmap16x16_OVERWRITE(int x, int y, byte *ABitmap32)
{
    for (int i = 0; i < 16; i++) {
        //byte *p = (byte *)HanBitmap->ScanLine[i];
       // p[0] = ~ABitmap32[2*i];
        //p[1] = ~ABitmap32[2*i+1];
    }
    //ACanvas->Draw(x, y, HanBitmap);
}
//---------------------------------------------------------------------------
static void PutBitmap8x16_OVERLAP(int x, int y, byte *ABitmap16)
{   // �����ϰ� �׸���
    for (int i = 0; i < 16; i++) {
        //byte *p = (byte *)EngBitmap->ScanLine[i];
        //p[0] = ~ABitmap16[i];
    }
    //ĵ������ �귯�ÿ� ������ ������ 8x16 ũ�⸸ŭ �����
    //ACanvas->FillRect(TRect(x, y, x + 8, y + 16));

    //EngBitmap->Transparent = true;
    //ACanvas->Draw(x, y, EngBitmap);
    //EngBitmap->Transparent = false; // �̰� �� ����� ��
}
//---------------------------------------------------------------------------
static void PutBitmap16x16_OVERLAP(int x, int y, byte *ABitmap32)
{   // �����ϰ� �׸���
    for (int i = 0; i < 16; i++) {
        //byte *p = (byte *)HanBitmap->ScanLine[i];
       // p[0] = ~ABitmap32[2*i];
       // p[1] = ~ABitmap32[2*i+1];
    }
    // ĵ������ �귯�ÿ� ������ ������ 16x16 ũ�⸸ŭ �����
    //ACanvas->FillRect(TRect(x, y, x + 16, y + 16));

    //HanBitmap->Transparent = true;
    //ACanvas->Draw(x, y, HanBitmap);
    //HanBitmap->Transparent = false;  // �̰� �� ����� ��
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
void (*PutBitmap8x16)(int x, int y, byte *ABitmap32) = PutBitmap8x16_OVERWRITE;
void (*PutBitmap16x16)(int x, int y, byte *ABitmap32) = PutBitmap16x16_OVERWRITE;
//---------------------------------------------------------------------------
void SetOutputMode(int mode)
{
    switch (mode) {
    case OVERWRITE:
        PutBitmap8x16 = PutBitmap8x16_OVERWRITE;
        PutBitmap16x16 = PutBitmap16x16_OVERWRITE;
        break;
    case OVERLAP:
        PutBitmap8x16 = PutBitmap8x16_OVERLAP;
        PutBitmap16x16 = PutBitmap16x16_OVERLAP;
        break;
    }
}
//---------------------------------------------------------------------------
void HanTextOut(int left, int top, byte *s)
{
    byte bitmap32[32], johab[2];

    int x = 0;
    for (unsigned i = 0; i < strlen((char*)s); ) {
        if (ishangul1st(s, i)) {
            // CP949 Ȯ��ϼ��� �� ��� ���������� ��ȯ
            code_cnvtcs(CODE_KSSM, CODE_CP949, johab, s[i], s[i+1]);

            codeidx_t codeidx;
            _code->getidx(&codeidx, johab[0], johab[1]);
            switch (codeidx.kind) {
            case CODEKIND_SPECIAL:
                PutBitmap16x16(left + x, top, pDefSpcFont->Spc[s[i] - 0xA1][s[i+1] - 0xA1]);
                break;
            case CODEKIND_HANGUL:
                CompleteHanChar(bitmap32, johab, pDefHanFont);
                PutBitmap16x16(left + x, top, bitmap32);
                break;
            case CODEKIND_HANJA:
                PutBitmap16x16(left + x, top, pDefHanjaFont->Hanja[s[i] - 0xCA][s[i+1] - 0xA1]);
                break;
            }
            x += 16, i += 2;
        } else {
            PutBitmap8x16(left + x, top, pDefEngFont->Eng[s[i]]);
            x += 8, i++;
        }
    }
}
//---------------------------------------------------------------------------

