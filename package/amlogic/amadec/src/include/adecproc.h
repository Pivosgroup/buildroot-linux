#ifndef ADECPROC_H
#define ADECPROC_H


struct amadec_opt
{
	char * name;
	int socket_fd;
	unsigned int out_type;
	char * vol_cmd[16];
	
	unsigned int old_is_run:1;
	unsigned int server_mode:1;
	unsigned int front_run:1;
	unsigned int cmd_valid:1;
	unsigned int cmd_start:1;
	unsigned int cmd_stop:1;
	unsigned int cmd_quit:1;
	unsigned int cmd_pause:1;
	unsigned int cmd_resume:1;
	unsigned int cmd_mute:1;
	unsigned int cmd_unmute:1;
	unsigned int cmd_volset:1;
	unsigned int cmd_volget:1;
	unsigned int cmd_leftmono:1;
	unsigned int cmd_rightmono:1;
	unsigned int cmd_stereo:1;
	unsigned int cmd_swap:1;
	unsigned int cmd_automute:1;
       unsigned int cmd_spectrum_on:1;
       unsigned int cmd_spectrum_off:1;
	
};

extern int adecd_process(struct amadec_opt *);

#endif /* ADECPROC_H */
