#ifndef CPICTRANSITION_H
#define CPICTRANSITION_H
#include <QString>
#include "./effect/screen_area.h"
#define MAX_EFFECT 10

typedef int (*effect_fun_t)(screen_area_t* src,screen_area_t* dst, unsigned extra);

struct CEffectItem {
	QString name;
	effect_fun_t fun;
};

class CPicTransition
{
public:
	CPicTransition();
    CPicTransition(screen_area_t *src,screen_area_t *dst);
	~CPicTransition();
	void set_src_dst(screen_area_t *src,screen_area_t *dst);
    bool run_effect(char* effect_name,unsigned extra);
	bool register_effect(const char* effect_name,unsigned effect_fun);
private:
	screen_area_t src_area;		/* source area. */
	screen_area_t dst_area;		/* dest area. */
	
	CEffectItem effect_list[MAX_EFFECT];
	int cur_effect_len;
};

#endif // CPICTRANSITION_H
