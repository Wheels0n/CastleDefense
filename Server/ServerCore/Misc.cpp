#include "stdafx.h"
#include "Misc.h"

void MiscHelper::ConvertUE2Nav(Coordiante* pCoord, OUT float pDst[], const float offset[])
{
	//����� �ٲ۵� �������Ƿ�, ���� ���� ������
	pDst[ENUM_TO_INT(_NAV_COMP::forward)]	= -(pCoord->x() - offset[ENUM_TO_INT(_UE_COMP::forward)]);
	pDst[ENUM_TO_INT(_NAV_COMP::right)]		= -(pCoord->y() - offset[ENUM_TO_INT(_UE_COMP::right)]);
	pDst[ENUM_TO_INT(_NAV_COMP::up)]		=  pCoord->z() - offset[ENUM_TO_INT(_UE_COMP::up)];
	
}
void MiscHelper::ConvertNav2UE(OUT Coordiante* pCoord, float pSrc[], const float offset[])
{
	float ue[3]				= { 0, };
	ue[ENUM_TO_INT(_UE_COMP::right)]		=  -pSrc[ENUM_TO_INT(_NAV_COMP::right)]		+ offset[ENUM_TO_INT(_UE_COMP::right)];
	ue[ENUM_TO_INT(_UE_COMP::forward)]		=  -pSrc[ENUM_TO_INT(_NAV_COMP::forward)]	+ offset[ENUM_TO_INT(_UE_COMP::forward)];
	ue[ENUM_TO_INT(_UE_COMP::up)]			=	pSrc[ENUM_TO_INT(_NAV_COMP::up)]		+ offset[ENUM_TO_INT(_UE_COMP::up)];

	pCoord->set_x(ue[ENUM_TO_INT(_UE_COMP::forward)]);
	pCoord->set_y(ue[ENUM_TO_INT(_UE_COMP::right)]);
	pCoord->set_z(ue[ENUM_TO_INT(_UE_COMP::up)]);
}
