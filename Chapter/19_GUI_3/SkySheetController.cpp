#include "SkySheetController.h"

SkySheetController::SkySheetController()
{

}


SkySheetController::~SkySheetController()
{
}

SkySheet *SkySheetController::Alloc()
{
	SkySheet* sheet;
	int i;
	for (i = 0; i < MAX_SHEETS; i++)
	{
		if (sheets0[i].m_flags == 0)
		{
			sheet = &sheets0[i];
			sheet->m_flags = SHEET_USE; /* ����� ��ũ */
			sheet->m_height = -1; /* ��ǥ���� */
			sheet->SetOwner(this);
			return sheet;
		}
	}
	return nullptr;	//����� �� �ִ� ��Ʈ�� ����
}

bool SkySheetController::Initialize(unsigned char *vram, int xsize, int ysize)
{
	m_map = (unsigned char *) new unsigned char[xsize * ysize];
	
	if (m_map == nullptr)
	{
		return false;
	}
	m_vram = vram;
	m_xsize = xsize;
	m_ysize = ysize;
	m_top = -1; /* ��Ʈ�� �� �嵵 ���� */
	
	for (int i = 0; i < MAX_SHEETS; i++) 
	{
		sheets0[i].m_flags = 0; /* �̻�� ��ũ */
	}

	return true;
}

void SkySheetController::UpdateSheets(int old, int height, SkySheet* sheet)
{
	/* ���ϴ� �ַ� sheets[]�� �þ���� ��ü */
	if (old > height)  /* �������� �������� */
	{
		if (height >= 0) {
			/* ������ ���� ����ø��� */
			for (int h = old; h > height; h--)
			{
				sheets[h] = sheets[h - 1];
				sheets[h]->m_height = h;
			}

			sheets[height] = sheet;
			RefreshMap(sheet->m_vx0, sheet->m_vy0, sheet->m_vx0 + sheet->m_bxsize, sheet->m_vy0 + sheet->m_bysize, height + 1);
			RefreshSub(sheet->m_vx0, sheet->m_vy0, sheet->m_vx0 + sheet->m_bxsize, sheet->m_vy0 + sheet->m_bysize, height + 1, old);
		}
		else
		{	/* ��ǥ��ȭ */
			if (m_top > old) {
				/* ���� �Ǿ� �ִ� ���� ������ */
				for (int h = old; h < m_top; h++) {
					sheets[h] = sheets[h + 1];
					sheets[h]->m_height = h;
				}
			}
			m_top--;
			
			RefreshMap(sheet->m_vx0, sheet->m_vy0, sheet->m_vx0 + sheet->m_bxsize, sheet->m_vy0 + sheet->m_bysize, 0);
			RefreshSub(sheet->m_vx0, sheet->m_vy0, sheet->m_vx0 + sheet->m_bxsize, sheet->m_vy0 + sheet->m_bysize, 0, old - 1);
		}
	}
	else if (old < height) /* �������� �������� */
	{
		if (old >= 0) {
			/* ������ ���� ���� ������ */
			for (int h = old; h < height; h++) {
				sheets[h] = sheets[h + 1];
				sheets[h]->m_height = h;
			}
			sheets[height] = sheet;
		}
		else {	/* ��ǥ�� ���¿��� ǥ�� ���·� */
				/* ���� �Ǿ� �ִ� ���� ��� �ø��� */
			for (int h = m_top; h >= height; h--)
			{
				sheets[h + 1] = sheets[h];
				sheets[h + 1]->m_height = h + 1;
			}
			
			sheets[height] = sheet;

			m_top++;/* ǥ������ ���̾ 1�� �����ϹǷ�, �� ���� ���̰� �����Ѵ� */			
		}
		RefreshMap(sheet->m_vx0, sheet->m_vy0, sheet->m_vx0 + sheet->m_bxsize, sheet->m_vy0 + sheet->m_bysize, height);
		RefreshSub(sheet->m_vx0, sheet->m_vy0, sheet->m_vx0 + sheet->m_bxsize, sheet->m_vy0 + sheet->m_bysize, height, height);
	}
}

void SkySheetController::RefreshMap(int vx0, int vy0, int vx1, int vy1, int h0)
{
	int h, bx, by, vx, vy, bx0, by0, bx1, by1;
	unsigned char *buf, sid;
	SkySheet *sht;

	if (vx0 < 0) 
	{ 
		vx0 = 0; 
	}

	if (vy0 < 0) 
	{ 
		vy0 = 0;
	}

	if (vx1 > m_xsize) 
	{ 
		vx1 = m_xsize; 
	}

	if (vy1 > m_ysize) 
	{ 
		vy1 = m_ysize; 
	}

	for (h = h0; h <= m_top; h++) 
	{
		sht = sheets[h];
		sid = sht - sheets0; /* ������ ���� �װ��� ���̾� ��ȣ�� �̿� */
		buf = sht->m_buf;
		bx0 = vx0 - sht->m_vx0;
		by0 = vy0 - sht->m_vy0;
		bx1 = vx1 - sht->m_vx0;
		by1 = vy1 - sht->m_vy0;

		if (bx0 < 0) 
		{
			bx0 = 0; 
		}

		if (by0 < 0) 
		{ 
			by0 = 0; 
		}

		if (bx1 > sht->m_bxsize) 
		{
			bx1 = sht->m_bxsize; 
		}

		if (by1 > sht->m_bysize) 
		{ 
			by1 = sht->m_bysize; 
		}

		for (by = by0; by < by1; by++) 
		{
			vy = sht->m_vy0 + by;
			for (bx = bx0; bx < bx1; bx++) 
			{
				vx = sht->m_vx0 + bx;
				if (buf[by * sht->m_bxsize + bx] != sht->m_col_inv) 
				{
					m_map[vy * m_xsize + vx] = sid;
				}
			}
		}
	}
	return;
}

void SkySheetController::RefreshSub(int vx0, int vy0, int vx1, int vy1, int h0, int h1)
{
	int h, bx, by, vx, vy, bx0, by0, bx1, by1;
	unsigned char *buf, sid;
	SkySheet *sht;

	/* refresh ������ ȭ��ܿ� ������ ���� */
	if (vx0 < 0) { vx0 = 0; }
	if (vy0 < 0) { vy0 = 0; }
	if (vx1 > m_xsize) 
	{ 
		vx1 = m_xsize; 
	}

	if (vy1 > m_ysize) 
	{ 
		vy1 = m_ysize; 
	}

	for (h = h0; h <= h1; h++) 
	{
		sht = sheets[h];
		buf = sht->m_buf;
		sid = sht - sheets0;

		/* vx0~vy1�� �����, bx0~by1�� �����Ѵ� */
		bx0 = vx0 - sht->m_vx0;
		by0 = vy0 - sht->m_vy0;
		bx1 = vx1 - sht->m_vx0;
		by1 = vy1 - sht->m_vy0;

		if (bx0 < 0)
		{ 
			bx0 = 0;
		}

		if (by0 < 0) 
		{ 
			by0 = 0; 
		}

		if (bx1 > sht->m_bxsize) 
		{ 
			bx1 = sht->m_bxsize; 
		}

		if (by1 > sht->m_bysize) 
		{ 
			by1 = sht->m_bysize; 
		}

		for (by = by0; by < by1; by++) 
		{
			vy = sht->m_vy0 + by;
			for (bx = bx0; bx < bx1; bx++) 
			{
				vx = sht->m_vx0 + bx;
				if (m_map[vy * m_xsize + vx] == sid) 
				{
					m_vram[vy * m_xsize + vx] = buf[by * sht->m_bxsize + bx];
				}
			}
		}
	}
	return;
}
