#include <string.h>
#include <stdio.h>
#include "PLG.h"
#include "PLGmain.h"

/*******************************************************************************
	Main
*******************************************************************************/
int main(int argc, char **argv)
{
PLG 	*state = 0;
	if ( argc > 1 )
		{
		state = new PLG();
		state->process(argv[1]);
		}
	else {
		state = new PLG("12345 hello");
		state->run();
		}
}

void PLGmain::run()
{
	return;
}
