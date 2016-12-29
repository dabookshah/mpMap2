	void applyFunnel(int startPosition, int endPosition, int finalCounter, int funnelID)
	{
		//Compute forward probabilities
		funnelEncoding enc = (*lineFunnelEncodings)[(*lineFunnelIDs)[finalCounter]];
		int funnel[16];
		for(int founderCounter = 0; founderCounter < nFounders; founderCounter++)
		{
			funnel[founderCounter] = ((enc & (15 << (4*founderCounter))) >> (4*founderCounter));
		}
		int startMarkerIndex = allPositions.markerIndices[startPosition];
		if(startMarkerIndex == -1)
		{
			for(int founderCounter = 0; founderCounter < nFounders; founderCounter++)
			{
				forwardProbabilities(funnel[founderCounter], 0) = 1.0/nFounders;
			}
		}
		else
		{
			int markerValue = recodedFinals(finalCounter, startMarkerIndex);
			int validInitial = 0;
			for(int founderCounter = 0; founderCounter < nFounders; founderCounter++)
			{
				if(recodedFounders(funnel[founderCounter], startMarkerIndex) == markerValue || markerValue == NA_INTEGER)
				{
					forwardProbabilities(funnel[founderCounter], 0) = 1;
					validInitial++;
				}
				else forwardProbabilities(founderCounter, 0) = 0;
			}
			for(int founderCounter = 0; founderCounter < nFounders; founderCounter++)
			{
				forwardProbabilities(founderCounter, 0) /= (double)validInitial;
			}
		}
		for(int positionCounter = startPosition; positionCounter < endPosition - 1; positionCounter++)
		{
			int markerIndex = allPositions.markerIndices[positionCounter+1];
			double sum = 0;
			if(markerIndex == -1)
			{
				for(int founderCounter = 0; founderCounter < nFounders; founderCounter++)
				{
					forwardProbabilities(founderCounter, positionCounter - startPosition + 1) = 0;
					//The founder at the previous marker
					for(int founderCounter2 = 0; founderCounter2 < nFounders; founderCounter2++)
					{
						forwardProbabilities(funnel[founderCounter], positionCounter - startPosition + 1) += forwardProbabilities(funnel[founderCounter2], positionCounter - startPosition) * funnelHaplotypeProbabilities(positionCounter - startPosition, 0).values[founderCounter2][founderCounter];
					}
					sum += forwardProbabilities(funnel[founderCounter], positionCounter - startPosition + 1);
				}
			}
			else
			{
				int markerValue = recodedFinals(finalCounter, markerIndex);
				//The founder at the new marker
				for(int founderCounter = 0; founderCounter < nFounders; founderCounter++)
				{
					forwardProbabilities(founderCounter, positionCounter - startPosition + 1) = 0;
					if(recodedFounders(funnel[founderCounter], markerIndex) == markerValue || markerValue == NA_INTEGER)
					{
						//The founder at the previous marker
						for(int founderCounter2 = 0; founderCounter2 < nFounders; founderCounter2++)
						{
							forwardProbabilities(funnel[founderCounter], positionCounter - startPosition + 1) += forwardProbabilities(funnel[founderCounter2], positionCounter - startPosition) * funnelHaplotypeProbabilities(positionCounter - startPosition, 0).values[founderCounter2][founderCounter];
						}
					}
					sum += forwardProbabilities(funnel[founderCounter], positionCounter - startPosition + 1);
				}
			}
			for(int founderCounter = 0; founderCounter < nFounders; founderCounter++)
			{
				forwardProbabilities(funnel[founderCounter], positionCounter - startPosition + 1) /= sum;
			}
		}
		//Now the backwards probabilities
		for(int founderCounter = 0; founderCounter < nFounders; founderCounter++)
		{
			backwardProbabilities(founderCounter, endPosition - startPosition - 1) = 1/(double)nFounders;
		}
		for(int positionCounter = endPosition - 2; positionCounter >= startPosition; positionCounter--)
		{
			int markerIndex = allPositions.markerIndices[positionCounter+1];
			double sum = 0;
			if(markerIndex == -1)
			{
				//The founder at the current marker
				for(int founderCounter = 0; founderCounter < nFounders; founderCounter++)
				{
					backwardProbabilities(funnel[founderCounter], positionCounter - startPosition) = 0;
					//The founder at the previous marker
					for(int founderCounter2 = 0; founderCounter2 < nFounders; founderCounter2++)
					{
						backwardProbabilities(funnel[founderCounter], positionCounter - startPosition) += backwardProbabilities(funnel[founderCounter2], positionCounter - startPosition + 1) * funnelHaplotypeProbabilities(positionCounter - startPosition, 0).values[founderCounter2][founderCounter];
					}
					sum += backwardProbabilities(funnel[founderCounter], positionCounter - startPosition);
				}
			}
			else
			{
				int markerValue = recodedFinals(finalCounter, markerIndex);
				//The founder at the current marker
				for(int founderCounter = 0; founderCounter < nFounders; founderCounter++)
				{
					backwardProbabilities(funnel[founderCounter], positionCounter - startPosition) = 0;
					//The founder at the previous marker
					for(int founderCounter2 = 0; founderCounter2 < nFounders; founderCounter2++)
					{
						if(recodedFounders(funnel[founderCounter2], markerIndex) == markerValue || markerValue == NA_INTEGER)
						{
							backwardProbabilities(funnel[founderCounter], positionCounter - startPosition) += backwardProbabilities(funnel[founderCounter2], positionCounter - startPosition + 1) * funnelHaplotypeProbabilities(positionCounter - startPosition, 0).values[founderCounter2][founderCounter];
						}
					}
					sum += backwardProbabilities(funnel[founderCounter], positionCounter - startPosition);
				}
			}
			for(int founderCounter = 0; founderCounter < nFounders; founderCounter++)
			{
				backwardProbabilities(funnel[founderCounter], positionCounter - startPosition) /= sum;
			}
		}
		//Now we can compute the marginal probabilities
		for(int positionCounter = startPosition; positionCounter < endPosition; positionCounter++)
		{
			double sum = 0;
			for(int founderCounter = 0; founderCounter < nFounders; founderCounter++)
			{
				results(nFounders*finalCounter + founderCounter, positionCounter) = backwardProbabilities(founderCounter, positionCounter - startPosition) * forwardProbabilities(founderCounter, positionCounter - startPosition);
				sum += results(nFounders*finalCounter + founderCounter, positionCounter);
			}
			for(int founderCounter = 0; founderCounter < nFounders; founderCounter++)
			{
				results(nFounders*finalCounter + founderCounter, positionCounter) /= sum;
			}
		}
	}

