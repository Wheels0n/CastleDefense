#pragma once
#include "stdafx.h"
#include "test.pb.h"

//RecastNav : y+, right Hand
//UE5		: z+, left Hand
//protobuf �޽����� ����ִ� �� UE5�� 
//�׿� �ڵ� �󿡼��� RecastNav�� ����
// 
//recastNav������ ������ �������� ���Ǵ� ����̴�. y+ ��������ǥ���ӿ��� ���̴�.
//ue5������ ���ݴ��̴�. �÷��̾��� ��� ������ ���ؼ� �� ������ ����̴�.  
//ue���� ������ obj������ recast&detour���� ȸ���� �Ǵ� ����̴�. �𸮾󿡼� ��ġ yaw-90�� �Ͱ� ����.  
//���� recast->ue ��ȯ�� x(����)->y, y->z, z(����)->x�� �´�. 
//�׷��� ȸ���������� ue�� x(����)���� recastNav�� x(����)������, y(����)�� z(����)���� ����ϴ� �� �´�.
//����� ���Ǹ� ���� ���� �� �־���..
//yaw�� atan2() ���� �̿��Ѵ�. �� �Լ��� ���п���ó�� �ݽð�����ε� ue������ �ð�����̶� 90-atan()��  
//������Ѵ�.  

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
	//�߽����� ���� ������� ���� ���, center�� ���� ��ü�� ��ġ��
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
