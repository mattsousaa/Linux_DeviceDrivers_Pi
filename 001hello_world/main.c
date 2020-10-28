#include <linux/module.h>

static int __init helloworld_init(void){
	pr_info("Hello World\n");
	return 0;    
}

static void __exit helloworld_cleanup(void){
	pr_info("Good Bye World\n");
}

module_init(helloworld_init);
module_exit(helloworld_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mateus Sousa");
MODULE_DESCRIPTION("A simple Linux LKM for the RPi");
MODULE_INFO(board, "Raspberry pi 4 Model B");
