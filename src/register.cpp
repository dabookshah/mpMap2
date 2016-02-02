#include <Rcpp.h>
#include <R_ext/Rdynload.h>
#include "checkHets.h"
#include "generateGenotypes.h"
#include "alleleDataErrors.h"
#include "estimateRF.h"
#ifdef CUSTOM_STATIC_RCPP
#include "internal.h"
#endif
#include "fourParentPedigreeRandomFunnels.h"
#include "eightParentPedigreeRandomFunnels.h"
#include "matrixChunks.h"
#include "rawSymmetricMatrix.h"
#include "dspMatrix.h"
#include "preClusterStep.h"
#include "hclustMatrices.h"
#include "mpMap2_openmp.h"
#include "order.h"
#include "arsa.h"
#include "impute.h"
extern "C"
{
	char* package_name = "mpMap2";
	R_CallMethodDef callMethods[] = 
	{
		{"checkHets", (DL_FUNC)&checkHets, 1},
		{"generateGenotypes", (DL_FUNC)&generateGenotypes, 3},
		{"alleleDataErrors", (DL_FUNC)&alleleDataErrors, 2},
		{"listCodingErrors", (DL_FUNC)&listCodingErrors, 3},
		{"estimateRF", (DL_FUNC)&estimateRF, 9},
		{"fourParentPedigreeRandomFunnels", (DL_FUNC)&fourParentPedigreeRandomFunnels, 4},
		{"eightParentPedigreeRandomFunnels", (DL_FUNC)&eightParentPedigreeRandomFunnels, 4},
		{"countValuesToEstimate", (DL_FUNC)&countValuesToEstimateExported, 2},
		{"singleIndexToPair", (DL_FUNC)&singleIndexToPairExported, 3},
		{"rawSymmetricMatrixSubsetIndices", (DL_FUNC)&rawSymmetricMatrixSubsetIndices, 4},
		{"rawSymmetricMatrixSubsetObject", (DL_FUNC)&rawSymmetricMatrixSubsetObject, 2},
		{"assignRawSymmetricMatrixFromEstimateRF", (DL_FUNC)&assignRawSymmetricMatrixFromEstimateRF, 4},
		{"assignRawSymmetricMatrixDiagonal", (DL_FUNC)&assignRawSymmetricMatrixDiagonal, 3},
		{"assignDspMatrixFromEstimateRF", (DL_FUNC)&assignDspMatrixFromEstimateRF, 4},
		{"preClusterStep", (DL_FUNC)&preClusterStep, 1},
		{"hclustThetaMatrix", (DL_FUNC)&hclustThetaMatrix, 2},
		{"hclustCombinedMatrix", (DL_FUNC)&hclustCombinedMatrix, 2},
		{"hclustLodMatrix", (DL_FUNC)&hclustLodMatrix, 2},
		{"omp_set_num_threads", (DL_FUNC)&mpMap2_omp_set_num_threads, 1},
		{"order", (DL_FUNC)&order, 6},
		{"checkRawSymmetricMatrix", (DL_FUNC)&checkRawSymmetricMatrix, 1},
		{"arsa", (DL_FUNC)&arsa, 5},
		{NULL, NULL, 0}
	};
	RcppExport void R_init_mpMap2(DllInfo *info)
	{
		std::vector<R_CallMethodDef> callMethodsVector;
		R_CallMethodDef* mpMap2CallMethods = callMethods;
		while(mpMap2CallMethods->name != NULL) mpMap2CallMethods++;
		callMethodsVector.insert(callMethodsVector.begin(), callMethods, mpMap2CallMethods);

#ifdef CUSTOM_STATIC_RCPP
		R_CallMethodDef* RcppStartCallMethods = Rcpp_get_call();
		R_CallMethodDef* RcppEndCallMethods = RcppStartCallMethods;
		while(RcppEndCallMethods->name != NULL) RcppEndCallMethods++;
		callMethodsVector.insert(callMethodsVector.end(), RcppStartCallMethods, RcppEndCallMethods);
#endif
		R_CallMethodDef blank = {NULL, NULL, 0};
		callMethodsVector.push_back(blank);

		R_registerRoutines(info, NULL, &(callMethodsVector[0]), NULL, NULL);
#ifdef CUSTOM_STATIC_RCPP
		init_Rcpp_cache();
#endif
		R_RegisterCCallable(package_name, "impute", (DL_FUNC)&impute);
	}
}
