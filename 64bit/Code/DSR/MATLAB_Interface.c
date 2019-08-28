/*
 *
 *	This is a simple program that illustrates how to call the MATLAB
 *	Engine functions from NetSim C Code.
 *
 */
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "engine.h"
#include "mat.h"
#include "mex.h"
#include "main.h"
#include "802_15_4.h"
#include "direct.h"
#include "DSR.h"


char buf[100];
Engine *ep;
int status;
mxArray *Xc = NULL, *Yc = NULL, *Pwr = NULL;
double **x, **y, **p;
mxArray *out;
double *result[3];
int dim = 0;
double fn_netsim_matlab_init()
{
	/*
	 * Start the MATLAB engine Process
	 */
	fprintf(stderr, "\nPress any key to start MATLAB...\n");
	_getch();
	if (!(ep = engOpen(NULL))) {
		MessageBox((HWND)NULL, (LPCWSTR)"Can't start MATLAB engine",
			(LPCWSTR) "MATLAB_Interface.c", MB_OK);
		exit(-1);
	}

	engEvalString(ep, "desktop");
	//specify the path of the DSR folder where the clustering.m file is present
	
	sprintf(buf, "fid = fopen('log.txt','w+'); fclose(fid);");
	status = engEvalString(ep, buf);
	return 0;
}

void fn_netsim_matlab_run(int s_count, int n_cls, POWER **remainingEnergy)
{
	int i = 0;
	dim = NETWORK->nDeviceCount - 1;

	Xc = mxCreateNumericMatrix((mwSize)dim, 1, mxDOUBLE_CLASS, mxREAL);
	x = (double**)mxMalloc(dim * sizeof(*x));
	x[0] = (double*)mxGetPr(Xc);

	Yc = mxCreateNumericMatrix((mwSize)dim, 1, mxDOUBLE_CLASS, mxREAL);
	y = (double**)mxMalloc(dim * sizeof(*y));
	y[0] = (double*)mxGetPr(Yc);

	Pwr = mxCreateNumericMatrix((mwSize)dim, 1, mxDOUBLE_CLASS, mxREAL);
	p = (double**)mxMalloc(dim * sizeof(*p));
	p[0] = (double*)mxGetPr(Pwr);

	for (i = 1; i < dim; i++)
	{
		x[i] = x[i - 1] + 1;
		y[i] = y[i - 1] + 1;
		p[i] = p[i - 1] + 1;
	}


	for (i = 0; i < NETWORK->nDeviceCount - 1; i++)
	{
		x[i][0] = DEVICE_POSITION(i + 1)->X;
		y[i][0] = DEVICE_POSITION(i + 1)->Y;
		p[i][0] = remainingEnergy[i]->dRemainingPower;
	}

	engPutVariable(ep, "Xc", Xc);
	engPutVariable(ep, "Yc", Yc);
	engPutVariable(ep, "Pwr", Pwr);

	sprintf(buf, "[A,B,C]=clustering([Xc,Yc],%d,%d,Pwr)", s_count, n_cls);
	status = engEvalString(ep, buf);
	double *cluster_head, *cluster_id, *cluster_size;

	out = engGetVariable(ep, "A");//contains the cluster head id's of each cluster
	cluster_head = mxGetPr(out);

	out = engGetVariable(ep, "B");//contains the cluster id's of each sensor
	cluster_id = mxGetPr(out);

	out = engGetVariable(ep, "C");//contains the size of each cluster
	cluster_size = mxGetPr(out);

	fn_netsim_dynamic_form_clusters(cluster_id, cluster_size);//cluster id's and size of each cluster are passed as parameters
	fn_netsim_assign_cluster_heads(cluster_head);//cluster head id's are passed as parameters

}

double fn_netsim_matlab_finish()
{
	//Close the MATLAB Engine Process
	fprintf(stderr, "\nPress any key to close MATLAB...\n");
	_getch();
	status = engEvalString(ep, "exit");
	return 0;
}

