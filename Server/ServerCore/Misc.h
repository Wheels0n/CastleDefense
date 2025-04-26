#pragma once
#include "stdafx.h"
#include "test.pb.h"

//RecastNav : y+, right Hand
//UE5		: z+, left Hand
//protobuf 메시지에 들어있는 건 UE5로 
//그외 코드 상에서는 RecastNav로 가정
// 
//recastNav에서는 우상단이 원점으로 계산되는 모양이다. y+ 오른손좌표계임에도 말이다.
//ue5에서는 정반대이다. 플레이어의 경우 접근을 안해서 잘 몰랐던 모양이다.  
//ue에서 가져온 obj파일이 recast&detour에서 회전이 되는 모양이다. 언리얼에서 마치 yaw-90한 것과 같다.  
//원래 recast->ue 변환은 x(우향)->y, y->z, z(전방)->x가 맞다. 
//그런데 회전떄문인지 ue의 x(전방)값이 recastNav의 x(우향)값으로, y(우향)이 z(전방)으로 계산하는 게 맞다.
//계산의 용의를 위해 묶인 해 주었다..
//yaw는 atan2() 값을 이용한다. 이 함수는 수학에서처럼 반시계방향인데 ue에서는 시계방향이라 90-atan()을  
//해줘야한다.  

#define	ENUM_TO_INT(x)  static_cast<int> (x)

enum class _UE_COMP
{
	forward,//x
	right,	//y
	up		//z
};
enum class _NAV_COMP
{
	forward,//x
	up,		//y
	right	//z
};

struct AABB
{
	//중심으로 부터 상대적인 값만 기록, center는 동적 개체의 위치로
	float  center[3];
	float  bmin[3];
	float  bmax[3];
	
	void SetExtent(const float e[3])
	{
		bmax[ENUM_TO_INT(_NAV_COMP::forward)] = center[ENUM_TO_INT(_NAV_COMP::forward)] - e[ENUM_TO_INT(_NAV_COMP::forward)];
		bmin[ENUM_TO_INT(_NAV_COMP::forward)] = min(center[ENUM_TO_INT(_NAV_COMP::forward)] + e[ENUM_TO_INT(_NAV_COMP::forward)], 0);
		bmax[ENUM_TO_INT(_NAV_COMP::right)] = center[ENUM_TO_INT(_NAV_COMP::right)] - e[ENUM_TO_INT(_NAV_COMP::right)];
		bmin[ENUM_TO_INT(_NAV_COMP::right)] = min(center[ENUM_TO_INT(_NAV_COMP::right)] + e[ENUM_TO_INT(_NAV_COMP::right)], 0);
		bmax[ENUM_TO_INT(_NAV_COMP::up)] = center[ENUM_TO_INT(_NAV_COMP::up)] - e[ENUM_TO_INT(_NAV_COMP::up)];
		bmin[ENUM_TO_INT(_NAV_COMP::up)] = min(center[ENUM_TO_INT(_NAV_COMP::up)] + e[ENUM_TO_INT(_NAV_COMP::up)], 0);
	}
};



class MiscHelper
{
public:
	static void ConvertUE2Nav(Coordiante* pCoord, float pDst[], const float offset[]);
	static void ConvertNav2UE(Coordiante* pCoord, float pSrc[], const float offset[]);

private:

};
