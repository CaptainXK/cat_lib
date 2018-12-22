#include <cat_main.h>

int main(int argc, char** argv)
{
	//parse cos and core config
	int ret = parse_cosc(argc, argv);

	if(ret == 1){
		return 0;
	}
	
	//show current global cos and core configuration
	show_cosc_conf();

	//init cos config and core bind
	init_cosc();

	return 0;
}
