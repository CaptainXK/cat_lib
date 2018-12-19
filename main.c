#include <stdio.h>
#include <unistd.h>//getopt()
#include <inttypes.h>
#include <stdlib.h>//atoi()
#include <string.h>//strtok_r()

#include <cosc.h>
#include <association_app.h>
#include <allocation_app_l3cat.h>

Cosc gl_cosc[MAX_COSC];

int cosc_nb = 0;

void show_cosc_conf()
{
	int i, j;
	int _core_nb;
	unsigned int _cos_id;
	uint64_t _mask;
	unsigned int * _core_list;

	printf("[Global COSC config]\n");

	if(cosc_nb <= 0){
		printf("No cosc detected!\n");
		return;
	}

	for(i = 0; i < cosc_nb; ++i)
	{
		_cos_id = gl_cosc[i].cos_id;
		_mask = gl_cosc[i].mask;
		_core_nb = gl_cosc[i].core_nb;
		_core_list = gl_cosc[i].core_list;

		printf("cos#%u, mask=0x%.5lx, %d cores:", _cos_id, _mask, _core_nb);
		for(j = 0; j < _core_nb; ++j){
			printf(" %u", _core_list[j]);
		}
		
		printf("\n");
	}
}

int config_one_cosc(char * cosc_conf_str, int cosc_id)
{
	int ret = 1;

	printf("cosc:%s\n", cosc_conf_str);

	int idx = 0;
	int core_id = 0;	
	char *str, *token_tmp, *token[3], *saveptr;

	for(str = cosc_conf_str; ; str = NULL){
		token_tmp = strtok_r(str, ";",  &saveptr);
		if(token_tmp == NULL)
			break;
		else
			token[idx++] = token_tmp;
	}

	//cos id
	gl_cosc[cosc_id].cos_id = atoi(token[0]);

	//cos mask
	gl_cosc[cosc_id].mask = strtoul(token[1], NULL, 16);

	//core_list
	gl_cosc[cosc_id].core_nb = 0;
	for(str = token[2]; ; str = NULL){
		token_tmp = strtok_r(str, ",", &saveptr);
		if(token_tmp == NULL)
			break;
		else{
			gl_cosc[cosc_id].core_list[core_id++] = atoi(token_tmp);
			gl_cosc[cosc_id].core_nb += 1;
		}
	}

	//cosc nb +1
	cosc_id += 1;

	return ret;
}

void show_usage()
{
	printf("Usage:\n");
	printf("\t-h:show usage.\n");
	printf("\t-n:number of cosc.\n");
	printf("\t-C:COSC, cos and core_list.\n\t\tExample:\"0;0x0000f;1,2,3\", bind core#1~3 to cos#0 whose mask is 0x0000f\n");
}

int parse_cosc(int argc, char** argv)
{
	int flags, opt;
	int ret = 1;
	int idx=1;
	int cosc_id = 0;

	if(argc <= 1){
		show_usage();
		return ret;
	}
	
	while ( (opt = getopt(argc, argv, "n:C:h")) != -1  )
	{
		switch(opt){
			case 'n':
					cosc_nb=atoi(optarg);
					break;

			case 'C':
					if(cosc_nb <= 0){
						printf("Set number of cosc (-n) first!\n");
						return 1;
					}

					config_one_cosc(optarg, cosc_id++);

					for(idx = 0; idx < cosc_nb-1; ++idx){
						config_one_cosc(argv[optind+idx], cosc_id++);	
					}
				   	
					ret = 0;	
					
					break;

			case 'h':
			default:
					show_usage();
					break;
		}	
	}

	return ret;
}

int main(int argc, char** argv)
{
	//parse cos and core config
	int ret = parse_cosc(argc, argv);

	if(ret == 1){
		return 0;
	}
	
	//show current global cos and core configuration
	show_cosc_conf();

	//init all cos
	l3_cos_init(gl_cosc, cosc_nb);

	//bind core to cos
	core_bind_cos_init(gl_cosc, cosc_nb);	

	return 0;
}




