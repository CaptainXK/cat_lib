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

int mask_is_contiguous(uint32_t _mask){
	
	if(_mask == 0){
		printf("llc mask can not be 0\n");
		return 0;
	}
	
	//find first 1 bit
	while( (_mask & 1U) == 0U){
		_mask >>= 1;
	}

	//find first 0 bit after contiguous 1 bits
	while( (_mask & 1U) == 1U){
		_mask >>= 1;
	}

	//if _mask is 0, means all 1 bits is contiguous
	if(_mask == 0){
		return 1;
	}
	//else, some 1 bits is still on high end, means _mask is not contiguous
	else{
		printf("llc mask is not contiguous\n");
		return 0;
	}

}

void get_cos_mask_slice(uint32_t cos_mask_list[], uint32_t llc_mask, uint32_t cos_nb)
{
	uint32_t tot_cache_lines_available = 0;
	uint32_t llc_mask_temp = llc_mask;
	uint32_t llc_mask_base = 0;
	uint32_t llc_cache_lines_per_cos = 0;
	int i;

	//count 1 bit on llc_mask
	while(llc_mask_temp){
		if(llc_mask_temp & 0x1 == 1){
			tot_cache_lines_available += 1;
		}

		llc_mask_temp >>= 1;
	}

	//llc cache lines per cos
	llc_cache_lines_per_cos = tot_cache_lines_available / cos_nb;
	printf("llc lines per cos = %u\n", llc_cache_lines_per_cos);

	//prepare llc_cache_line_base
	for(i = 0; i < llc_cache_lines_per_cos; ++i){
		llc_mask_base <<= 1;
		llc_mask_base += 1;
	}

	while( (llc_mask_base & llc_mask) == 0){
		printf("Base <<\n");
		llc_mask_base <<= llc_cache_lines_per_cos;
	}	
	printf("llc mask base = 0x%.5x\n", llc_mask_base);

	//create cos mask for every cos
	for(i=0; i < cos_nb; ++i){
		cos_mask_list[i] = llc_mask_base;
		llc_mask_base <<= llc_cache_lines_per_cos;
	}

}

int parse_core_mask(uint32_t core_mask, uint32_t llc_mask)
{
	int core_list[MAX_CORE_NB]={0};
	uint32_t cos_mask_list[MAX_CORE_NB]={0};
	int core_idx = 0;
	int core_list_idx = 0;
	int llc_cache_lines_per_cos = 0;
	unsigned int i;

	//init cosc_nb
	cosc_nb = 0;
	
	//check validity of llc_mask
	if(mask_is_contiguous(llc_mask) == 0)
		return 1;
	
	//retrive whole mask to find core enabled
	while(core_mask != 0){
		if(core_mask & 0x1 == 1){
			core_list[core_list_idx] = core_idx;
			core_list_idx += 1;
			core_idx += 1;
			cosc_nb += 1;
		}
		
		core_mask >>= 1;
	}

	if(cosc_nb <= 0)
		return 1;

	//get cos mask for every cos	
	get_cos_mask_slice(cos_mask_list, llc_mask, cosc_nb);

	//config cosc list by cos_mask_list and core_list
	for(i=0; i<cosc_nb; ++i){
		gl_cosc[i].cos_id = i;
		gl_cosc[i].mask = cos_mask_list[i];
		gl_cosc[i].core_nb = 1;
		gl_cosc[i].core_list[0] = core_list[i];
	}
	
	return 0;	
}

void show_usage()
{
	printf("Usage:\n");
	printf("\t-h:show usage.\n");
	printf("\t-n:number of cosc.\n");
	printf("\t-C:COSC option, cos and core_list.\n\t\tExample:\"0;0x0000f;1,2,3\", bind core#1~3 to cos#0 whose mask is 0x0000f\n");
	printf("\t-m: -m [core_mask llc_tot_mask]. Each bit in core mask represents one core, and they will be assigned to one cos with same LLC cache line. llc_tot_mask represent llc cache line available and it must be contiguous\n");
}

int parse_cosc(int argc, char** argv)
{
	int flags, opt;
	int ret = 1;
	int idx=1;
	int cosc_id = 0;
	uint32_t core_mask;
	uint32_t llc_mask;

	if(argc <= 1){
		show_usage();
		return ret;
	}
	
	while ( (opt = getopt(argc, argv, "n:C:m:h")) != -1  )
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

			case 'm':
					core_mask = (uint32_t)strtoul(optarg, NULL, 16);
					llc_mask = (uint32_t)strtoul(argv[optind], NULL, 16);
					if (parse_core_mask(core_mask, llc_mask) == 0){
						ret = 0;
					}
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




