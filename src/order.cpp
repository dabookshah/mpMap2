#include "order.h"
#include "impute.h"
#include "arsaRaw.h"
SEXP order(SEXP mpcrossLG_sexp, SEXP groupsToOrder_sexp, SEXP cool_, SEXP temperatureMin_, SEXP nReps_, SEXP verbose_)
{
	Rcpp::S4 mpcrossLG;
	try
	{
		mpcrossLG = mpcrossLG_sexp;
	}
	catch(...)
	{
		throw std::runtime_error("Input mpcrossLG must be an S4 object");
	}

	Rcpp::S4 lg;
	try
	{
		lg = Rcpp::as<Rcpp::S4>(mpcrossLG.slot("lg"));
	}
	catch(...)
	{
		throw std::runtime_error("Slot mpcrossLG@lg must be an S4 object");
	}

	Rcpp::S4 rf;
	try
	{
		rf = Rcpp::as<Rcpp::S4>(mpcrossLG.slot("rf"));
	}
	catch(...)
	{
		throw std::runtime_error("Slot mpcrossLG@rf must be an S4 object");
	}

	Rcpp::S4 theta;
	try
	{
		theta = Rcpp::as<Rcpp::S4>(rf.slot("theta"));
	}
	catch(...)
	{
		throw std::runtime_error("Slot mpcrossLG@rf@theta must be an S4 object");
	}

	Rcpp::RawVector originalRawData;
	try
	{
		originalRawData = theta.slot("data");
	}
	catch(...)
	{
		throw std::runtime_error("Slot mpcrossLG@rf@theta@data must be a raw vector");
	}

	std::vector<double> levels;
	try
	{
		levels = Rcpp::as<std::vector<double> >(Rcpp::as<Rcpp::NumericVector>(theta.slot("levels")));
	}
	catch(...)
	{
		throw std::runtime_error("Slot mpcrossLG@rf@theta@levels must be a numeric vector");
	}

	std::vector<int> groups;
	try
	{
		groups = Rcpp::as<std::vector<int> >(lg.slot("groups"));
	}
	catch(...)
	{
		throw std::runtime_error("Slot mpcrossLG@lg@groups must be an integer vector");
	}

	std::vector<int> allGroups;
	try
	{
		allGroups = Rcpp::as<std::vector<int> >(lg.slot("allGroups"));
	}
	catch(...)
	{
		throw std::runtime_error("Slot mpcrossLG@lg@allGroups must be an integer vector");
	}

	std::vector<int> groupsToOrder;
	try
	{
		groupsToOrder = Rcpp::as<std::vector<int> >(groupsToOrder_sexp);
	}
	catch(...)
	{
		throw std::runtime_error("Input groupsToOrder must be an integerVector");
	}

	double cool;
	try
	{
		cool = Rcpp::as<double>(cool_);
	}
	catch(...)
	{
		throw std::runtime_error("Input cool must be a number");
	}

	double temperatureMin;
	try
	{
		temperatureMin = Rcpp::as<double>(temperatureMin_);
	}
	catch(...)
	{
		throw std::runtime_error("Input temperatureMin must be a number");
	}

	long nReps;
	try
	{
		nReps = Rcpp::as<int>(nReps_);
	}
	catch(...)
	{
		throw std::runtime_error("Input nReps must be an integer");
	}

	bool verbose;
	try
	{
		verbose = Rcpp::as<bool>(verbose_);
	}
	catch(...)
	{
		throw std::runtime_error("Input verbose must be a boolean");
	}


	//Check that groupsToOrder contains unique values, which are contained in allGroups
	std::sort(groupsToOrder.begin(), groupsToOrder.end());
	std::sort(allGroups.begin(), allGroups.end());
	if(std::unique(groupsToOrder.begin(), groupsToOrder.end()) != groupsToOrder.end())
	{
		throw std::runtime_error("Input groupsToOrder must contain unique values");
	}
	for(std::vector<int>::iterator groupToOrder = groupsToOrder.begin(); groupToOrder != groupsToOrder.end(); groupToOrder++)
	{
		std::vector<int>::iterator bound = std::lower_bound(allGroups.begin(), allGroups.end(), *groupToOrder);
		if(bound == allGroups.end() || *bound != *groupToOrder)
		{
			throw std::runtime_error("Input groupsToOrder contained a group that was not present in slot mpcrossLG@lg@allGroups");
		}
	}


	Rcpp::Function asDist("as.dist"), seriate("seriate"), getOrder("get_order"), criterion("criterion");
	R_xlen_t nMarkers = groups.size();

	std::vector<int> permutation, currentGroupPermutation;
	permutation.reserve(nMarkers);
	std::vector<int> markersThisGroup;
	//This holds a copy of the raw data, for the purposes of doing imputation. We don't want to touch the original, obviously. 
	std::vector<unsigned char> imputedRaw;
	//Stuff for the verbose output case
	Rcpp::RObject barHandle;
	Rcpp::Function txtProgressBar("txtProgressBar"), setTxtProgressBar("setTxtProgressBar"), close("close");
	for(std::vector<int>::iterator currentGroup = allGroups.begin(); currentGroup != allGroups.end(); currentGroup++)
	{
		markersThisGroup.clear();
		//linkage groups are not required to be contiguous in this case, so we have to scan through to find the number of markers in this group
		for(std::vector<int>::iterator currentMarkerGroup = groups.begin(); currentMarkerGroup != groups.end(); currentMarkerGroup++)
		{
			if(*currentMarkerGroup == *currentGroup)
			{
				markersThisGroup.push_back((int)std::distance(groups.begin(), currentMarkerGroup));
			}
		}
		int nMarkersCurrentGroup = (int)markersThisGroup.size();
		if(nMarkersCurrentGroup == 0) continue;
		
		//Do imputation
		//So first make a copy of the raw data subset
		imputedRaw.resize((nMarkersCurrentGroup * (nMarkersCurrentGroup + 1)) / 2);
		//column
		for(int i = 0; i < nMarkersCurrentGroup; i++)
		{
			//row
			for(int j = 0; j <= i; j++)
			{
				int row = markersThisGroup[j], column = markersThisGroup[i];
				if(row > column) std::swap(row, column);
				imputedRaw[(i * (i + 1))/2 + j] = originalRawData[(column * (column + 1))/2 + row];
			}
		}
		std::vector<int> contiguousIndices(nMarkersCurrentGroup);
		for(int i = 0; i < nMarkersCurrentGroup; i++) contiguousIndices[i] = i;

		
		std::string error;
		std::function<void(unsigned long,unsigned long)> imputationProgressFunction = [](unsigned long,unsigned long){};
		if(verbose)
		{
			Rcpp::Rcout << "Starting imputation for group " << *currentGroup << std::endl;
			barHandle = txtProgressBar(Rcpp::Named("style") = 3, Rcpp::Named("min") = 0, Rcpp::Named("max") = 1000, Rcpp::Named("initial") = 0);
			imputationProgressFunction = [barHandle, setTxtProgressBar](unsigned long done, unsigned long totalSteps)
			{
				setTxtProgressBar.topLevelExec(barHandle, (int)((double)(1000*done) / (double)totalSteps));
			};
		}
		bool imputationResult = impute(&(imputedRaw[0]), levels, NULL, NULL, contiguousIndices, error, imputationProgressFunction);
		if(verbose)
		{
			close(barHandle);
		}

		
		if(!imputationResult)
		{
			throw std::runtime_error(error.c_str());
		}
		//Unpack the data into a symmetric matrix
		std::vector<Rbyte> distMatrix(nMarkersCurrentGroup*nMarkersCurrentGroup);
		for(std::size_t i = 0; i < nMarkersCurrentGroup; i++)
		{
			for(std::size_t j = 0; j <= i; j++)
			{
				distMatrix[i * nMarkersCurrentGroup + j] = distMatrix[j * nMarkersCurrentGroup + i] = imputedRaw[(i*(i+1))/2 + j];
			}
		}
	
		
		std::function<void(unsigned long, unsigned long)> orderingProgressFunction = [](unsigned long,unsigned long){};
		if(verbose)
		{
			Rcpp::Rcout << "Starting to order group " << *currentGroup << std::endl;
			barHandle = txtProgressBar(Rcpp::Named("style") = 3, Rcpp::Named("min") = 0, Rcpp::Named("max") = 1000, Rcpp::Named("initial") = 0);
			orderingProgressFunction = [barHandle, setTxtProgressBar](unsigned long done, unsigned long totalSteps)
			{
				setTxtProgressBar.topLevelExec(barHandle, (int)((double)(1000*done) / (double)totalSteps));
			};
		}
		arsaRaw(nMarkersCurrentGroup, &(distMatrix[0]), levels, cool, temperatureMin, nReps, currentGroupPermutation, orderingProgressFunction);
		if(verbose)
		{
			close(barHandle);
		}
		for(int i = 0; i < nMarkersCurrentGroup; i++) permutation.push_back(markersThisGroup[currentGroupPermutation[i]]+1);
	}
	return Rcpp::wrap(permutation);
}
