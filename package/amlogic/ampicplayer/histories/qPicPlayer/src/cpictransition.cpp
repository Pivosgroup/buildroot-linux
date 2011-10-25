#include "cpictransition.h"
#include "./effect/effect_utils.h"

CPicTransition::CPicTransition()
{
	cur_effect_len=-1;
}
CPicTransition::~CPicTransition()
{
	cur_effect_len=-1;

}
CPicTransition::CPicTransition(screen_area_t *src,screen_area_t *dst)
{
	src_area=*src;
	dst_area=*dst;
}
void CPicTransition::set_src_dst(screen_area_t *src,screen_area_t *dst)
{
	src_area=*src;
	dst_area=*dst;
}

bool CPicTransition::run_effect(char *effect_name,unsigned extra)
{
	QString new_name(effect_name);
	for(int i=0;i<=cur_effect_len;i++) {
		if(effect_list[i].name.compare(new_name,Qt::CaseInsensitive)==0) {
			if(effect_list[i].fun!=NULL)
				effect_list[i].fun(&src_area,&dst_area,extra);
				return true;
		}
	}
	return false;
	//copy_area(&src_area,&dst_area,extra);
	
}

bool CPicTransition::register_effect(const char* effect_name,unsigned effect_fun)
{
	if(cur_effect_len>=(MAX_EFFECT-1)) return false;
	
	QString new_name(effect_name);
	if(cur_effect_len<0) {
		 effect_list[++cur_effect_len].name  += new_name;
		 effect_list[cur_effect_len].fun   = (effect_fun_t)effect_fun;
	} else {
		for(int i=0;i<=cur_effect_len;i++) {
			if(effect_list[i].name.compare(new_name,Qt::CaseInsensitive)==0) return false;
		}
		effect_list[++cur_effect_len].name  += new_name;
		effect_list[cur_effect_len].fun  = (effect_fun_t)effect_fun;
	}
	return true;
}
